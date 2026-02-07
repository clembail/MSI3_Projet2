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

  m_t = 0.0;

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
    ::iteration(m_v, m_u, m_dt, n_int, m_t);

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

const Values & Scheme::getOutput()
{
  return m_u;
}

void Scheme::setInput(const Values & u)
{
  m_u = u;
  m_v = u;
}
