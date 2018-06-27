/* ============================================================================
 *  tubex-lib - DimensionException class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#include <string>
#include <sstream>
#include "tubex_DimensionException.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  DimensionException::DimensionException(int dim, const string& comment)
  {
    ostringstream os;
    os << "invalid dimension: " << dim;
    if(comment != "")
      os << " (" << comment << ")";
    os << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const Vector& x1, const Vector& x2)
  {
    ostringstream os;
    os << "objects of different dimension: ";
    os << "Vector(" << x1.size() << "), "
       << "Vector(" << x2.size() << ")" << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const DynamicalItem& x, int i)
  {
    ostringstream os;
    os << "wrong dimension access: ";
    os << x.className() << "(" << x.dim() << "), i=" << i << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const DynamicalItem& x, const IntervalVector& box)
  {
    ostringstream os;
    os << "objects of different dimension: ";
    os << x.className() << "(" << x.dim() << "), "
       << "IntervalVector(" << box.size() << ")" << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const DynamicalItem& x, const Function& f)
  {
    ostringstream os;
    os << "objects of different dimension: ";
    os << x.className() << "(" << x.dim() << "), "
       << "Function(" << f.image_dim() << ")" << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const DynamicalItem& x1, const DynamicalItem& x2)
  {
    ostringstream os;
    os << "objects of different dimension: ";
    os << x1.className() << "(" << x1.dim() << "), "
       << x2.className() << "(" << x2.dim() << ")" << endl;
    m_what_msg = os.str();
  }

  DimensionException::DimensionException(const VibesFigure_Tube& fig, const DynamicalItem& x)
  {
    ostringstream os;
    os << "objects of different dimension: ";
    os << "VibesFigure_Tube(" << fig.dim() << "), "
       << x.className() << "(" << x.dim() << ")" << endl;
    m_what_msg = os.str();
  }

  void DimensionException::check(int dim)
  {
    if(dim < 1)
      throw DimensionException(dim);
  }

  void DimensionException::checkScalar(int dim)
  {
    if(dim != 1)
      throw DimensionException(dim, "not scalar");
  }

  void DimensionException::check(const Vector& x1, const Vector& x2)
  {
    if(x1.size() != x2.size())
      throw DimensionException(x1, x2);
  }

  void DimensionException::check(const DynamicalItem& x, int i)
  {
    if(i >= x.dim() || i < 0)
      throw DimensionException(x, i);
  }

  void DimensionException::check(const DynamicalItem& x, const IntervalVector& box)
  {
    if(x.dim() != box.size())
      throw DimensionException(x, box);
  }

  void DimensionException::check(const DynamicalItem& x, const ibex::Function& f)
  {
    if(x.dim() != f.image_dim())
      throw DimensionException(x, f);
  }

  void DimensionException::check(const DynamicalItem& x1, const DynamicalItem& x2)
  {
    if(x1.dim() != x2.dim())
      throw DimensionException(x1, x2);
  }

  void DimensionException::check(const VibesFigure_Tube& fig, const DynamicalItem& x)
  {
    int dim = fig.dim();
    if(dim != 0 && x.dim() != dim)
      throw DimensionException(fig, x);
  }
}