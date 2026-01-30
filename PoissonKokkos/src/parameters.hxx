#ifndef __PARAMETERS__
#define __PARAMETERS__

#include <iostream>
#include <string>
#include <memory>
#include <functional>

#include "type.hxx"
#include "arguments.hxx"

class Parameters : public Arguments {
public:

  Parameters(int argc, char **argv);
  void info();

  int n(int i) const { return m_n[i]; }
  REAL_TYPE dx(int i) const { return m_dx[i]; }
  REAL_TYPE xmin(int i) const { return m_xmin[i]; }
  REAL_TYPE xmax(int i) const { return m_xmax[i]; }

  int imin(int i) const { return m_imin[i]; }
  int imax(int i) const { return m_imax[i]; }
  
  int itmax() const { return m_itmax; }
  REAL_TYPE dt() const { return m_dt; }

  int freq() const { return m_freq; }
  std::string resultPath() const { return m_path; }
  bool help();

private:

  std::string m_command;

  int m_n[3];
  REAL_TYPE m_xmin[3], m_xmax[3], m_dx[3];
  int m_imin[3], m_imax[3];
  
  int m_itmax;
  REAL_TYPE m_dt;
  
  int m_freq;

  std::string m_path;
  bool m_help;

};

std::ostream & operator<<(std::ostream &f, const Parameters & p);


#endif
