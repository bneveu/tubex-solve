/* ============================================================================
 *  tubex-lib - Function class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2018
 * ---------------------------------------------------------------------------- */

#include "tubex_Function.h"
#include "tubex_TubeVector.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  Function::Function(const char* y)
    : tubex::Fnc(0, 1)
  {
    m_ibex_f = new ibex::Function("t", y);
  }

  Function::Function(const char* x1, const char* y)
    : tubex::Fnc(1, 1)
  {
    m_ibex_f = new ibex::Function("t", x1, y);
    // todo: check x1!="t"
  }

  Function::Function(const tubex::Function& f)
    : tubex::Fnc(f.nbVars(), f.imageDim())
  {
    *this = f;
  }

  Function::~Function()
  {
    delete m_ibex_f;
  }

  const Function& Function::operator=(const Function& f)
  {
    if(m_ibex_f != NULL)
      delete m_ibex_f;
    m_ibex_f = new ibex::Function(*f.m_ibex_f);
    Fnc::operator=(f);
  }

  const IntervalVector Function::eval(const Interval& t) const
  {
    IntervalVector box(1, t);
    return m_ibex_f->eval_vector(box);
  }

  const IntervalVector Function::eval(const Interval& t, const TubeVector& x) const
  {
    if(nbVars() == 0)
      return eval(t);

    // todo: check dim x regarding f
    if(x[t].is_empty())
      return IntervalVector(imageDim(), Interval::EMPTY_SET);

    IntervalVector box(nbVars() + 1); // +1 for system variable (t)
    box[0] = t;
    if(nbVars() != 0)
      box.put(1, x[t]);
    return m_ibex_f->eval_vector(box);
  }
}