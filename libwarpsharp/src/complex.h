// This may look like C code, but it is really -*- C++ -*-
/*
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _complex_h
#ifdef __GNUG__
#pragma interface
#endif
#define _complex_h 1


#include <iostream>
#include <math.h>

using std::ostream;
using std::istream;


class complex
{
#ifdef __ATT_complex__
public:
#else
protected:
#endif

  double           re;
  double           im;

public:

  double           real() const;
  double           imag() const;

                   complex();
                   complex(const complex& y);
                   complex(double r, double i=0);

  ~complex();

  complex&         operator =  (const complex& y);

  complex&         operator += (const complex& y);
  complex&         operator += (double y);
  complex&         operator -= (const complex& y);
  complex&         operator -= (double y);
  complex&         operator *= (const complex& y);
  complex&         operator *= (double y);

  complex&         operator /= (const complex& y);
  complex&         operator /= (double y);

  void             error(const char* msg) const;
};


// non-inline functions

complex   operator /  (const complex& x, const complex& y);
complex   operator /  (const complex& x, double y);
complex   operator /  (double   x, const complex& y);

complex   cos(const complex& x);
complex   sin(const complex& x);

complex   cosh(const complex& x);
complex   sinh(const complex& x);

complex   exp(const complex& x);
complex   log(const complex& x); // not implemented yet

complex   pow(const complex& x, int p);
complex   pow(const complex& x, const complex& p); // not implemented yet
complex   pow(const complex& x, double y); // not implemented yet
complex   sqrt(const complex& x); // not yet

istream&  operator >> (istream& s, complex& x);
ostream&  operator << (ostream& s, const complex& x);

// other functions defined as inlines

int  operator == (const complex& x, const complex& y);
int  operator == (const complex& x, double y);
int  operator != (const complex& x, const complex& y);
int  operator != (const complex& x, double y);

complex  operator - (const complex& x);
complex  conj(const complex& x);
complex  operator + (const complex& x, const complex& y);
complex  operator + (const complex& x, double y);
complex  operator + (double x, const complex& y);
complex  operator - (const complex& x, const complex& y);
complex  operator - (const complex& x, double y);
complex  operator - (double x, const complex& y);
complex  operator * (const complex& x, const complex& y);
complex  operator * (const complex& x, double y);
complex  operator * (double x, const complex& y);

double  real(const complex& x);
double  imag(const complex& x);
double  abs(const complex& x);
double  norm(const complex& x);
double  arg(const complex& x);

complex  polar(double r, double t = 0.0);



#endif
