#include "scheme.hxx"
#include "parameters.hxx"
#include "version.hxx"
#include "metal/User.h"
#include <cmath>
#include "iteration.hxx"
#include "variation.hxx"

#include <sstream>
#include <iomanip>

Scheme::Scheme(Parameters &P) :
    codeName(version), m_P(P), m_u(P), m_v(P)  {

  // m_t = 0.0;
  // m_duv = 0.0;

  // int i;
  // for (i = 0; i < 3; i++)
  // {
  //   m_n[i] = m_P.n(i);
  //   m_dx[i] = m_P.dx(i);
  //   m_xmin[i] = m_P.xmin(i);
  // }

  // m_dt = m_P.dt();

  // Initialisation des pointeurs pour la variation
    d_diff = nullptr;
    d_diffPartial = nullptr;

    // On récupère les dimensions locales pour Scheme (utile pour le calcul de taille)
    for (int i = 0; i < 3; i++) {
      m_n[i] = m_P.n(i);
      // m_dx et m_xmin ne sont plus utiles ici si gérés par Values/GPU,
      // mais tu peux les garder si tu t'en sers ailleurs.
    }

    m_dt = m_P.dt(); // Important pour le pas de temps

    m_u.init();
    m_u.boundaries();
    m_v.init();
    m_v.boundaries();
}

Scheme::~Scheme()
{
  freeVariationData(d_diff, d_diffPartial);
}

float Scheme::present()
{
  return m_t;
}

bool Scheme::iteration()
{
  int n_int[3];
    n_int[0] = (int)m_n[0];
    n_int[1] = (int)m_n[1];
    n_int[2] = (int)m_n[2];

    // 1. Calcul de la physique (GPU)
    ::iteration(m_v, m_u, m_dt, n_int,
                m_P.imin(0), m_P.imax(0),
                m_P.imin(1), m_P.imax(1),
                m_P.imin(2), m_P.imax(2));

    // 2. Calcul de la variation (GPU + Récupération CPU)
    // On a besoin du nombre total de points pour la réduction
    int total_points = n_int[0] * n_int[1] * n_int[2];

    // Appel de la fonction définie dans variation_metal.cxx
    // Elle va : calculer la diff, réduire la somme, et retourner le float sur CPU
    m_duv = ::variation(m_u, m_v, d_diff, d_diffPartial, total_points);

    // 3. Gestion du temps et échange
    m_t += m_dt;
    m_u.swap(m_v);

    return true;
}

// float Scheme::iteration_domaine(int imin, int imax,
//                                  int jmin, int jmax,
//                                  int kmin, int kmax)
// {
//   float lam_x = 1 / (m_dx[0] * m_dx[0]);
//   float lam_y = 1 / (m_dx[1] * m_dx[1]);
//   float lam_z = 1 / (m_dx[2] * m_dx[2]);
//   float xmin = m_xmin[0];
//   float ymin = m_xmin[1];
//   float zmin = m_xmin[2];
//   int i, j, k;
//   float du, du1, du2, du_sum = 0.0;

//   float x, y, z;

//   for (i = imin; i <= imax; i++)
//     for (j = jmin; j <= jmax; j++)
//       for (k = kmin; k <= kmax; k++) {

//         du1 = (-2 * m_u(i, j, k) + m_u(i + 1, j, k) + m_u(i - 1, j, k)) * lam_x
//             + (-2 * m_u(i, j, k) + m_u(i, j + 1, k) + m_u(i, j - 1, k)) * lam_y
//             + (-2 * m_u(i, j, k) + m_u(i, j, k + 1) + m_u(i, j, k - 1)) * lam_z;

//         x = xmin + i * m_dx[0];
//         y = ymin + j * m_dx[1];
//         z = zmin + k * m_dx[2];
//         du2 = force(x, y, z, m_t);

//         du = m_dt * (du1 + du2);
//         m_v(i, j, k) = m_u(i, j, k) + du;

//         du_sum += du > 0 ? du : -du;
//       }

//   return du_sum;
// }

const Values & Scheme::getOutput()
{
  return m_u;
}

void Scheme::setInput(const Values & u)
{
  m_u = u;
  m_v = u;
}
