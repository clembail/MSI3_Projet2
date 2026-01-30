#ifndef __VALUES__
#define __VALUES__

#include <vector>
#include <iostream>
#include "parameters.hxx"
#include "paramKokkos.hxx"

typedef Kokkos::View<REAL_TYPE***, Kokkos::LayoutLeft, MemSpace> DeviceArray;
typedef DeviceArray::host_mirror_type HostArray;

class Values {

public:

  Values(Parameters & p);
  virtual ~Values();
  void operator= (const Values &);
  
  void init();
  void boundaries();

  void plot(int order) const;
  void swap(Values & other);
  int size(int i) const { return m_n[i]; }
  void print() const;
  
  DeviceArray & deviceArray() { return d; }
  
  const DeviceArray & deviceArray() const { return d; }
  
private:
  
  Values(const Values &);
  int n1, n2, nn;
  DeviceArray d;
  Parameters & m_p;
  int m_imin[3];
  int m_imax[3];
  int m_n[3];

  REAL_TYPE m_dx[3];
  REAL_TYPE m_xmin[3];
  REAL_TYPE m_xmax[3];

};

std::ostream & operator<< (std::ostream & f, const Values & v);

#endif
