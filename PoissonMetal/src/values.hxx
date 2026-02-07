#ifndef __VALUES__
#define __VALUES__

#include "parameters.hxx"
#include <vector>
#include <iostream>

class Values {

public:

  Values(Parameters & p);
  virtual ~Values();
  void operator= (const Values &);

  void init();
  void zero();
  void boundaries();

  float & operator() (int i,int j,int k) {
    return h_u[n2*k + n1*j + i];
  }
  float operator() (int i,int j,int k) const {
    return h_u[n2*k + n1*j + i];
  }

  void plot(int order) const;
  void swap(Values & other);
  int size(int i) const { return m_n[i]; }
  void print(std::ostream &f) const;

  float * dataCPU() { return h_u; }
  float * dataGPU() { return d_u; }
  const float * dataCPU() const { return h_u; }
  const float * dataGPU() const { return d_u; }
  void synchronized(bool b) { h_synchronized = b; }

  float* get_d_u() { return d_u;}
  const float* get_d_u() const { return d_u;}

private:

  Values(const Values &);
  int n1, n2, nn;
  float * d_u, * h_u;
  mutable bool h_synchronized;

  Parameters & m_p;
  int m_imin[3];
  int m_imax[3];
  int m_n[3];

  float m_dx[3];
  float m_xmin[3];
  float m_xmax[3];

};

std::ostream & operator<< (std::ostream & f, const Values & v);

void zero(float *d, int n);
void init(float *d, int n[3]);
void boundaries(float *d, int n[3], int imin[3], int imax[3]);

#endif
