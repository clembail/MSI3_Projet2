/*
 * scheme.hxx
 *
 *  Created on: 5 janv. 2016
 *      Author: tajchman
 */

#ifndef SCHEME_HXX_
#define SCHEME_HXX_

#include "values.hxx"
#include "parameters.hxx"

class Scheme {

public:
  Scheme(Parameters &P);
  ~Scheme();

  float present();

  bool iteration();
  float variation() { return m_duv; }

  const Values & getOutput();
  void setInput(const Values & u);

  std::string codeName;

protected:
  float m_t, m_dt;
  size_t m_n[3];
  float m_dx[3];
  float m_xmin[3];

  float iteration_domaine(int imin, int imax,
                           int jmin, int jmax,
                           int kmin, int kmax);

  Parameters &m_P;
  Values m_u, m_v;
  float m_duv;

  float *d_diff;        // Pointeur pour le tableau de différences (GPU)
  float *d_diffPartial;
};

#endif /* SCHEME_HXX_ */
