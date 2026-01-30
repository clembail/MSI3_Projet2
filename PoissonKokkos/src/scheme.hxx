
#ifndef SCHEME_HXX_
#define SCHEME_HXX_

#include "values.hxx"
#include "parameters.hxx"

class Scheme {

public:
  Scheme(Parameters &P);
  ~Scheme();

  REAL_TYPE present();

  bool iteration();
  REAL_TYPE variation() { return m_duv; }

  const Values & getOutput();
  void setInput(const Values & u);

  std::string codeName;
  
  REAL_TYPE iteration_domaine(int imin, int imax, 
                              int jmin, int jmax,
                              int kmin, int kmax);

protected:

  REAL_TYPE m_t, m_dt;
  size_t m_n[3];
  REAL_TYPE m_dx[3];
  REAL_TYPE m_xmin[3];

  Parameters &m_P;
  Values m_u, m_v;
  REAL_TYPE m_duv;
};

#endif /* SCHEME_HXX_ */
