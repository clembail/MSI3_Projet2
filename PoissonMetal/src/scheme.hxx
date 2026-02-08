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

  Parameters &m_P;
  Values m_u, m_v;
  float m_duv;

  float *d_diff;
  float *d_diffPartial;
};

#endif
