#include "scheme.hxx"
#include "parameters.hxx"
#include "version.hxx"
#include "user.hxx"
#include <cmath>

#include <sstream>
#include <iomanip>

Scheme::Scheme(Parameters &P) :
    codeName(version), m_P(P), m_u(P), m_v(P)  {

  m_t = 0.0;
  m_duv = 0.0;

  int i;
  for (i = 0; i < 3; i++)
  {
    m_n[i] = m_P.n(i);
    m_dx[i] = m_P.dx(i);
    m_xmin[i] = m_P.xmin(i);
  }

  m_dt = m_P.dt();
}

Scheme::~Scheme()
{
}

REAL_TYPE Scheme::present()
{
  return m_t;
}

REAL_TYPE Scheme::iteration_domaine(int imin, int imax,
                                 int jmin, int jmax,
                                 int kmin, int kmax)
{
  REAL_TYPE lam_x = 1 / (m_dx[0] * m_dx[0]);
  REAL_TYPE lam_y = 1 / (m_dx[1] * m_dx[1]);
  REAL_TYPE lam_z = 1 / (m_dx[2] * m_dx[2]);
  REAL_TYPE xmin = m_xmin[0], dx = m_dx[0];
  REAL_TYPE ymin = m_xmin[1], dy = m_dx[1];
  REAL_TYPE zmin = m_xmin[2], dz = m_dx[2];
  REAL_TYPE dt = m_dt;
  REAL_TYPE t = m_t;

  DeviceArray u = m_u.deviceArray();
  DeviceArray v = m_v.deviceArray();
  
  REAL_TYPE du_sum = 0.0;

  Kokkos::parallel_reduce("iteration_domaine",
    Kokkos::MDRangePolicy<Kokkos::Rank<3>>({imin, jmin, kmin}, {imax+1, jmax+1, kmax+1}),
    KOKKOS_LAMBDA(const int i, const int j, const int k, REAL_TYPE & local_sum) {
      
      REAL_TYPE du1 = (-2 * u(i, j, k) + u(i + 1, j, k) + u(i - 1, j, k)) * lam_x
          + (-2 * u(i, j, k) + u(i, j + 1, k) + u(i, j - 1, k)) * lam_y
          + (-2 * u(i, j, k) + u(i, j, k + 1) + u(i, j, k - 1)) * lam_z;

      REAL_TYPE x = xmin + i * dx;
      REAL_TYPE y = ymin + j * dy;
      REAL_TYPE z = zmin + k * dz;
      REAL_TYPE du2 = force(x, y, z, t);

      REAL_TYPE du = dt * (du1 + du2);
      v(i, j, k) = u(i, j, k) + du;

      local_sum += du > 0 ? du : -du;
    }, du_sum);

  return du_sum;
}

bool Scheme::iteration()
{

  m_duv = iteration_domaine(
      m_P.imin(0), m_P.imax(0),
      m_P.imin(1), m_P.imax(1),
      m_P.imin(2), m_P.imax(2));

  m_t += m_dt;
  m_u.swap(m_v);

  return true;
}

const Values & Scheme::getOutput()
{
  return m_u;
}

void Scheme::setInput(const Values & u)
{
  Kokkos::deep_copy(m_u.deviceArray(), u.deviceArray());
}

