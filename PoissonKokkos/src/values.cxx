#include "values.hxx"
#include "os.hxx"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "user.hxx"
#include <Kokkos_Core.hpp>

/* A modifier */
Values::Values(Parameters & prm) : m_p(prm)
{
  for (int i=0; i<3; i++) {
    m_imin[i] = m_p.imin(i);
    m_imax[i] = m_p.imax(i);
    m_n[i]    = m_p.n(i);
    m_dx[i]   = m_p.dx(i);
    m_xmin[i] = m_p.xmin(i);
    m_xmax[i] = m_p.xmax(i);
  }
  
  d = DeviceArray("u", m_n[0]+2, m_n[1]+2, m_n[2]+2);
}

Values::~Values()
{
}

/* Introduire une lamda ou un objet fonction */
void Values::init()
{
  REAL_TYPE dx = m_dx[0];
  REAL_TYPE dy = m_dx[1];
  REAL_TYPE dz = m_dx[2];
  REAL_TYPE xmin = m_xmin[0];
  REAL_TYPE ymin = m_xmin[1];
  REAL_TYPE zmin = m_xmin[2];

  DeviceArray u = d; // capture by value

  Kokkos::parallel_for("init", 
    Kokkos::MDRangePolicy<Kokkos::Rank<3>>({m_imin[0], m_imin[1], m_imin[2]}, {m_imax[0]+1, m_imax[1]+1, m_imax[2]+1}),
    KOKKOS_LAMBDA(const int i, const int j, const int k) {
      REAL_TYPE x = xmin + i*dx;
      REAL_TYPE y = ymin + j*dy; 
      REAL_TYPE z = zmin + k*dz;
      u(i,j,k) = cond_ini(x, y, z);
    });
}

void Values::boundaries()
{
  DeviceArray u = d;
  REAL_TYPE *xmin = m_xmin; // Capture pointer to array? No, need to capture values or array. 
  // Arrays are members, cannot capture 'this' on device.
  // Better to copy scalar values to local variables for capture.
  
  REAL_TYPE xmin_0 = m_xmin[0], xmax_0 = m_xmax[0], dx_0 = m_dx[0];
  REAL_TYPE ymin_0 = m_xmin[1], ymax_0 = m_xmax[1], dy_0 = m_dx[1];
  REAL_TYPE zmin_0 = m_xmin[2], zmax_0 = m_xmax[2], dz_0 = m_dx[2];

  int imin = m_imin[0], imax = m_imax[0];
  int jmin = m_imin[1], jmax = m_imax[1];
  int kmin = m_imin[2], kmax = m_imax[2];

  // Lim X
  Kokkos::parallel_for("Lim X", 
    Kokkos::MDRangePolicy<Kokkos::Rank<2>>({jmin, kmin}, {jmax+1, kmax+1}),
    KOKKOS_LAMBDA(const int j, const int k) {
      REAL_TYPE y = ymin_0 + j*dy_0;
      REAL_TYPE z = zmin_0 + k*dz_0;
      u(imin, j, k) = cond_lim(xmin_0, y, z);
      u(imax, j, k) = cond_lim(xmax_0, y, z);
    });

  // Lim Y
  Kokkos::parallel_for("Lim Y", 
    Kokkos::MDRangePolicy<Kokkos::Rank<2>>({imin, kmin}, {imax+1, kmax+1}),
    KOKKOS_LAMBDA(const int i, const int k) {
      REAL_TYPE x = xmin_0 + i*dx_0;
      REAL_TYPE z = zmin_0 + k*dz_0;
      u(i, jmin, k) = cond_lim(x, ymin_0, z);
      u(i, jmax, k) = cond_lim(x, ymax_0, z);
    });

  // Lim Z
  Kokkos::parallel_for("Lim Z", 
    Kokkos::MDRangePolicy<Kokkos::Rank<2>>({imin, jmin}, {imax+1, jmax+1}),
    KOKKOS_LAMBDA(const int i, const int j) {
      REAL_TYPE x = xmin_0 + i*dx_0;
      REAL_TYPE y = ymin_0 + j*dy_0;
      u(i, j, kmin) = cond_lim(x, y, zmin_0);
      u(i, j, kmax) = cond_lim(x, y, zmax_0);
    });
}

void Values::print() const
{
  if (((m_imax[0]-m_imin[0]) > 5) || 
      ((m_imax[1]-m_imin[1]) > 5) ||
      ((m_imax[2]-m_imin[2]) > 5))
    return;

  int imin = m_imin[0]-1, jmin = m_imin[1]-1, kmin = m_imin[2]-1;
  int imax = m_imax[0]+1, jmax = m_imax[1]+1, kmax = m_imax[2]+1;

  HostArray h = Kokkos::create_mirror_view(d);
  Kokkos::deep_copy(h, d);

  int i, j, k;
  for (i=imin; i<=imax; i++) {
    printf("\ni=%-4d", i);
    for(j=jmin; j<=jmax; j++) {
      printf("\n\tj=%-4d", j);
      for (k=kmin; k<=kmax; k++)
        printf(" %7.2g", h(i,j,k));
    }
    printf("\n");
  }
}

template<typename T>
void swap(T & a, T & b)
{
  T t = a;
  a = b;
  b = t;
}

void Values::swap(Values & other)
{
  DeviceArray tmp = d;
  d = other.d;
  other.d = tmp;
  
  int i;
  for (i=0; i<3; i++) {
    ::swap(m_n[i], other.m_n[i]);
    ::swap(m_imin[i], other.m_imin[i]);
    ::swap(m_imax[i], other.m_imax[i]);
    ::swap(m_dx[i], other.m_dx[i]);
    ::swap(m_xmin[i], other.m_xmin[i]);
    ::swap(m_xmax[i], other.m_xmax[i]);
  }
}

void Values::plot(int order) const
{
  HostArray h = Kokkos::create_mirror(d);
  Kokkos::deep_copy(h, d);

  int ghost = 0;
  std::ostringstream s;
  int i, j, k;
  int imin = m_imin[0] - ghost, jmin = m_imin[1] - ghost, kmin = m_imin[2] - ghost;
  int imax = m_imax[0] + ghost, jmax = m_imax[1] + ghost, kmax = m_imax[2] + ghost;

  s << m_p.resultPath();
  mkdir_p(s.str().c_str());
  
  s << "/plot_" << order << ".vtr";
  std::ofstream f(s.str().c_str());

  f << "<?xml version=\"1.0\"?>\n";
  f << "<VTKFile type=\"RectilinearGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"
    << "<RectilinearGrid WholeExtent=\""
    << imin << " " << imax  << " "
    << jmin << " " << jmax  << " "
    << kmin << " " << kmax
    << "\">\n"
    << "<Piece Extent=\""
    << imin << " " << imax  << " "
    << jmin << " " << jmax  << " "
    << kmin << " " << kmax
    << "\">\n";

  f << "<PointData Scalars=\"values\">\n";
  f << "  <DataArray type=\"Float64\" Name=\"values\" format=\"ascii\">\n";

  for (k = kmin; k <= kmax; k++)
  {
    for (j = jmin; j <= jmax; j++)
    {
      for (i = imin; i <= imax; i++)
        f << " " << std::setw(12) << std::setprecision(6) << h(i, j, k);
      f << "\n";
    }
    f << "\n";
  }
  f << " </DataArray>\n";
   
  f << "</PointData>\n";

  f << " <Coordinates>\n";

  for (k = 0; k < 3; k++)
  {
    f << "   <DataArray type=\"Float64\" Name=\"" << char('X' + k) << "\""
      << " format=\"ascii\">";

    int imin = m_imin[k] - ghost;
    int imax = m_imax[k] + ghost;
    REAL_TYPE x0 = m_xmin[k], dx = m_dx[k];
    for (i = imin; i <= imax; i++)
      f << " " << x0 + i * dx;
    f << " </DataArray>\n";
  }
  f << " </Coordinates>\n";

  f << "</Piece>\n"
    << "</RectilinearGrid>\n"
    << "</VTKFile>\n" <<std::endl;
}

void Values::operator=(const Values &other)
{
  int i;
  
  for (i=0; i<3; i++) {
    m_n[i] = other.m_n[i];
    m_imin[i] = other.m_imin[i];
    m_imax[i] = other.m_imax[i];
    m_dx[i] = other.m_dx[i];
    m_xmin[i] = other.m_xmin[i];
    m_xmax[i] = other.m_xmax[i];
  }
  Kokkos::deep_copy(d, other.d);
}
