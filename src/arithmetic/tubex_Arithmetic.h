/* ============================================================================
 *  tubex-lib - Tube arithmetic
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#ifndef TUBEX_ARITHMETIC
#define TUBEX_ARITHMETIC

#include "tubex_Tube.h"
#include "ibex_Interval.h"

namespace tubex
{
  Tube operator+(const Tube& x);
  Tube operator+(const Tube& x1, const Tube& x2);
  Tube operator+(const Tube& x1, double x2);
  Tube operator+(double x1, const Tube& x2);
  Tube operator+(const Tube& x1, const ibex::Interval& x2);
  Tube operator+(const ibex::Interval& x1, const Tube& x2);

  Tube operator-(const Tube& x);
  Tube operator-(const Tube& x1, const Tube& x2);
  Tube operator-(const Tube& x1, double x2);
  Tube operator-(double x1, const Tube& x2);
  Tube operator-(const Tube& x1,  const ibex::Interval& x2);
  Tube operator-(const ibex::Interval& x1, const Tube& x2);

  Tube operator*(const Tube& x1, const Tube& x2);
  Tube operator*(const Tube& x1, double x2);
  Tube operator*(double x1, const Tube& x2);
  Tube operator*(const ibex::Interval& x1, const Tube& x2);
  Tube operator*(const Tube& x1, const ibex::Interval& x2);

  Tube operator/(const Tube& x1, const Tube& x2);
  Tube operator/(const Tube& x1, double x2);
  Tube operator/(double x1, const Tube& x2);
  Tube operator/(const ibex::Interval& x1, const Tube& x2);
  Tube operator/(const Tube& x1, const ibex::Interval& x2);

  Tube operator|(const Tube& x1, const Tube& x2);
  Tube operator|(const Tube& x1, double x2);
  Tube operator|(double x1, const Tube& x2);
  Tube operator|(const ibex::Interval& x1, const Tube& x2);
  Tube operator|(const Tube& x1, const ibex::Interval& x2);

  Tube operator&(const Tube& x1, const Tube& x2);
  Tube operator&(const Tube& x1, double x2);
  Tube operator&(double x1, const Tube& x2);
  Tube operator&(const ibex::Interval& x1, const Tube& x2);
  Tube operator&(const Tube& x1, const ibex::Interval& x2);

  Tube abs(const Tube& x);
  Tube sqr(const Tube& x);
  Tube sqrt(const Tube& x);
  Tube pow(const Tube& x, int p);
  Tube pow(const Tube& x, double p);
  Tube pow(const Tube &x, const ibex::Interval& p);
  Tube root(const Tube& x, int p);
  Tube exp(const Tube& x);
  Tube log(const Tube& x);
  Tube cos(const Tube& x);
  Tube sin(const Tube& x);
  Tube tan(const Tube& x);
  Tube acos(const Tube& x);
  Tube asin(const Tube& x);
  Tube atan(const Tube& x);
  Tube cosh(const Tube& x);
  Tube sinh(const Tube& x);
  Tube tanh(const Tube& x);
  Tube acosh(const Tube& x);
  Tube asinh(const Tube& x);
  Tube atanh(const Tube& x);
  Tube atan2(const Tube& y, const Tube& x);
}

#endif