//---------------------------------------------------------------- -*- C++ -*-
// Class  : complex
// Author : nur (Nur Arad),  1-Jun-97
//----------------------------------------------------------------------------

#ifndef MAKE_DEPEND
#endif // ndef MAKE_DEPEND

#include "complex.h"
#include "support.h"



double  complex::real() const { return re; }
double  complex::imag() const { return im; }

complex::complex() {}
complex::complex(const complex& y) :re(y.real()), im(y.imag()) {}
complex::complex(double r, double i) :re(r), im(i) {}

complex::~complex() {}

complex&  complex::operator =  (const complex& y) 
{ 
  re = y.real(); im = y.imag(); return *this; 
} 

complex&  complex::operator += (const complex& y)
{ 
  re += y.real();  im += y.imag(); return *this; 
}

complex&  complex::operator += (double y)
{ 
  re += y; return *this; 
}

complex&  complex::operator -= (const complex& y)
{ 
  re -= y.real();  im -= y.imag(); return *this; 
}

complex&  complex::operator -= (double y)
{ 
  re -= y; return *this; 
}

complex&  complex::operator *= (const complex& y)
{  
  double r = re * y.real() - im * y.imag();
  im = re * y.imag() + im * y.real(); 
  re = r; 
  return *this; 
}

complex&  complex::operator *= (double y)
{  
  re *=  y; im *=  y; return *this; 
}


//  functions

int  operator == (const complex& x, const complex& y)
{
  return x.real() == y.real() && x.imag() == y.imag();
}

int  operator == (const complex& x, double y)
{
  return x.imag() == 0.0 && x.real() == y;
}

int  operator != (const complex& x, const complex& y)
{
  return x.real() != y.real() || x.imag() != y.imag();
}

int  operator != (const complex& x, double y)
{
  return x.imag() != 0.0 || x.real() != y;
}

complex  operator - (const complex& x)
{
  return complex(-x.real(), -x.imag());
}

complex  conj(const complex& x)
{
  return complex(x.real(), -x.imag());
}

complex  operator + (const complex& x, const complex& y)
{
  return complex(x.real() + y.real(), x.imag() + y.imag());
}

complex  operator + (const complex& x, double y)
{
  return complex(x.real() + y, x.imag());
}

complex  operator + (double x, const complex& y)
{
  return complex(x + y.real(), y.imag());
}

complex  operator - (const complex& x, const complex& y)
{
  return complex(x.real() - y.real(), x.imag() - y.imag());
}

complex  operator - (const complex& x, double y)
{
  return complex(x.real() - y, x.imag());
}

complex  operator - (double x, const complex& y)
{
  return complex(x - y.real(), -y.imag());
}

complex  operator * (const complex& x, const complex& y)
{
  return complex(x.real() * y.real() - x.imag() * y.imag(), 
                 x.real() * y.imag() + x.imag() * y.real());
}

complex  operator * (const complex& x, double y)
{
  return complex(x.real() * y, x.imag() * y);
}

complex  operator * (double x, const complex& y)
{
  return complex(x * y.real(), x * y.imag());
}

complex operator / (const complex& z, double x)
{
  return complex(z.real()/x, z.imag()/x);
}

complex operator / (double x, const complex& z)
{
  return x*complex(z.real(), -z.imag())/norm(z);
}

complex operator / (const complex& x, const complex& y)
{
  return x*conj(y)/norm(y);
}

double  real(const complex& x)
{
  return x.real();
}

double  imag(const complex& x)
{
  return x.imag();
}

double  abs(const complex& x)
{
  return hypot(x.real(), x.imag());
}

double  norm(const complex& x)
{
  return (x.real() * x.real() + x.imag() * x.imag());
}

double  arg(const complex& x)
{
  return atan2(x.imag(), x.real());
}

complex  polar(double r, double t)
{
  return complex(r * cos(t), r * sin(t));
}

complex exp(const complex& x)
{
  double im = x.imag();
  return exp(x.real()) *(cos(im) + I*sin(im));
}

complex cos(const complex& x)
{
  complex ex(exp(I*x));
  return (ex + 1.0/ex)/2.0;
}

complex sin(const complex& x)
{
  complex ex(exp(I*x));
  return (ex - 1.0/ex)/(2.0*I);
}

complex cosh(const complex& x)
{
  complex ex(exp(x));
  return (ex + 1.0/ex)/2.0;
}

complex sinh(const complex& x)
{
  complex ex(exp(x));
  return (ex - 1.0/ex)/(2.0);
}

complex pow(const complex& x, int n)
{
  if(n == 0) return 1;
  
  double norm = 1;
  double xn = abs(x);
  if(n > 0) {
    while (n--) norm *= xn;
    double phi = arg(x)*n;
    return polar(norm, phi);
  }
  else return 1/pow(x, -n);
}

complex pow(const complex& x, double y)
{
  double norm = abs(x);
  norm = pow(norm, y);
  double phi = arg(x)*y;
  return polar(norm, phi);
}

ostream&  operator << (ostream& s, const complex& x)
{
  s << "(" << x.real() <<x.imag() <<")";
  return s;
}

ostream&  operator >> (ostream& s, const complex& x)
{
  s >> x.real() >>x.imag();
  return s;
}

