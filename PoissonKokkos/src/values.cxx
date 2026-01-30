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
  nn = 1;
  for (int i=0; i<3; i++) {
    m_imin[i] = m_p.imin(i);
    m_imax[i] = m_p.imax(i);
    m_n[i]    = m_p.n(i);
    m_dx[i]   = m_p.dx(i);
    m_xmin[i] = m_p.xmin(i);
    m_xmax[i] = m_p.xmax(i);
    nn *= m_n[i];
  }
  
  n1 = m_n[2];      // nombre de points dans la premiere direction
  n2 = m_n[1] * n1; // nombre de points dans le plan des 2 premieres directions
  m_u = new REAL_TYPE[nn];
}

Values::~Values()
{
/* A modifier */
  delete [] m_u;
}

/* Introduire une lamda ou un objet fonction */
void Values::init()
{
  int i, j, k;
  REAL_TYPE x, y, z;

  for (i=m_imin[0]; i<=m_imax[0]; i++)
    for (j=m_imin[1]; j<=m_imax[1]; j++)
      for (k=m_imin[2]; k<=m_imax[2]; k++) {
        x = m_xmin[0] + i*m_dx[0];
        y = m_xmin[1] + j*m_dx[1]; 
        z = m_xmin[2] + k*m_dx[2];
        operator()(i,j,k) = cond_ini(x, y, z);
      }
}

struct sLim
{
  sLim(REAL_TYPE * vmin, REAL_TYPE * vmax, REAL_TYPE *dv, 
    DeviceArray u, 
    int *ivmin, int *ivmax)
  : _u(u)
  {
    xmin =  vmin[0]; xmax =  vmax[0];
    ymin =  vmin[1]; ymax =  vmax[1];
    zmin =  vmin[2]; zmax =  vmax[2];
    imin = ivmin[0]; imax = ivmax[0];
    jmin = ivmin[1]; jmax = ivmax[1];
    kmin = ivmin[2]; kmax = ivmax[2];
  }

  REAL_TYPE xmin, xmax, dx, ymin, ymax, dy, zmin, zmax, dz;
  int imin, imax, jmin, jmax, kmin, kmax;
  DeviceArray _u;
  Kokkos::MDRangePolicy< Kokkos::Rank<2>> indices;
};

struct sLimX : public sLim
{
  sLimX(REAL_TYPE * vmin, REAL_TYPE * vmax, REAL_TYPE *dv, 
    DeviceArray u, 
    int *ivmin, int *ivmax) : sLim(vmin, vmax, dv, u, ivmin, ivmax)
    {
      indices= {{ivmin[1],ivmin[2]},
                {ivmax[1],ivmax[2]}};
    }
  KOKKOS_FUNCTION
  void operator() (long unsigned j, long unsigned k) const
  {
    REAL_TYPE y = ymin + j*dy; 
    REAL_TYPE z = zmin + k*dz;
    _u(imin,j,k) = cond_lim(xmin, y, z);
    _u(imax,j,k) = cond_lim(xmax, y, z);
  }
};

struct sLimY : public sLim
{
  sLimY(REAL_TYPE * vmin, REAL_TYPE * vmax, REAL_TYPE *dv, 
    DeviceArray u, 
    int *ivmin, int *ivmax) : sLim(vmin, vmax, dv, u, ivmin, ivmax)
    {
      indices= {{ivmin[0],ivmin[2]},
                {ivmax[0],ivmax[2]}};
      }
  KOKKOS_FUNCTION
  void operator() (long unsigned i, long unsigned k) const
  {
    REAL_TYPE x = xmin + i*dx; 
    REAL_TYPE z = zmin + k*dz;
    _u(i,jmin,k) = cond_lim(x, ymin, z);
    _u(i,jmax,k) = cond_lim(x, ymax, z);
  }
};

struct sLimZ : public sLim
{
  sLimZ(REAL_TYPE * vmin, REAL_TYPE * vmax, REAL_TYPE *dv, 
    DeviceArray u, 
    int *ivmin, int *ivmax) : sLim(vmin, vmax, dv, u, ivmin, ivmax)
    {
      indices= {{ivmin[0],ivmin[1]},
                {ivmax[0],ivmax[1]}};
      }
  KOKKOS_FUNCTION
  void operator() (long unsigned i, long unsigned j) const
  {
    REAL_TYPE x = xmin + i*dx;
    REAL_TYPE y = ymin + j*dy; 
    _u(i,j,kmin) = cond_lim(x, y, zmax);
    _u(i,j,kmax) = cond_lim(x, y, zmax);
  }
};

void Values::boundaries()
{

  return;
  sLimX IX(m_xmin, m_xmax, m_dx, d, m_imin, m_imax);
  Kokkos::parallel_for("Lim X", IX.indices, IX);

  sLimZ IY(m_xmin, m_xmax, m_dx, d, m_imin, m_imax);
  Kokkos::parallel_for("Lim Y", IY.indices, IY);

  sLimZ IZ(m_xmin, m_xmax, m_dx, d, m_imin, m_imax);
  Kokkos::parallel_for("Lim Z", IZ.indices, IZ);
}

void Values::print() const
{
  if (((m_imax[0]-m_imin[0]) > 5) || 
      ((m_imax[1]-m_imin[1]) > 5) ||
      ((m_imax[2]-m_imin[2]) > 5))
    return;

  int imin = m_imin[0]-1, jmin = m_imin[1]-1, kmin = m_imin[2]-1;
  int imax = m_imax[0]+1, jmax = m_imax[1]+1, kmax = m_imax[2]+1;

  /* Introduire une lambda ou un objet fonction */  
  int i, j, k;
  for (i=imin; i<=imax; i++) {
    printf("\ni=%-4d", i);
    for(j=jmin; j<=jmax; j++) {
      printf("\n\tj=%-4d", j);
      for (k=kmin; k<=kmax; k++)
        printf(" %7.2g", d_copy(i,j,k));
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
/* A modifier */
  ::swap(m_u, other.m_u);
  int i;
  for (i=0; i<3; i++) {
    ::swap(m_n[i], other.m_n[i]);
    ::swap(m_imin[i], other.m_imin[i]);
    ::swap(m_imax[i], other.m_imax[i]);
    ::swap(m_dx[i], other.m_dx[i]);
    ::swap(m_xmin[i], other.m_xmin[i]);
    ::swap(m_xmax[i], other.m_xmax[i]);
  }
  ::swap(nn, other.nn);
  ::swap(n1, other.n1);
  ::swap(n2, other.n2);
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
  nn = other.nn;
  n1 = other.n1;
  n2 = other.n2;
  Kokkos::deep_copy(d, other.d);
}
