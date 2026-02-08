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

  d_diff = nullptr;
  d_diffPartial = nullptr;

  for (int i = 0; i < 3; i++) {
    m_n[i] = m_P.n(i);
  }

  m_dt = m_P.dt();

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

    ::iteration(m_v, m_u, m_dt, n_int, m_t);

    int total_points = n_int[0] * n_int[1] * n_int[2];
  
    m_duv = ::variation(m_u, m_v, d_diff, d_diffPartial, total_points);

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
