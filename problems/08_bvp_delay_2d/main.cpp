/** 
 *  tubex-solve - Problems
 *  Solver testcase
 * ----------------------------------------------------------------------------
 *
 *  \date       2018
 *  \author     Simon Rohou
 *  \copyright  Copyright 2019 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#include "codac.h"
#include "tubex-solve.h"

using namespace std;
using namespace ibex;
using namespace codac;

class FncDelayCustom : public TFnc
{
  public: 

    FncDelayCustom() : TFnc(2, 2, true) { };
    const Interval eval(const IntervalVector& x) const { /* scalar case not defined */ }
    const Interval eval(int slice_id, const TubeVector& x) const { /* scalar case not defined */ }
    const Interval eval(const Interval& t, const TubeVector& x) const { /* scalar case not defined */ }
    const TubeVector eval_vector(const TubeVector& x) const {  /* scalar case not defined */ }
    const IntervalVector eval_vector(const IntervalVector& x) const { /* scalar case not defined */ }

    const IntervalVector eval_vector(int slice_id, const TubeVector& x) const
    {
      Interval t = x[0].slice(slice_id)->tdomain();
      return eval_vector(t, x);
    }

    const IntervalVector eval_vector(const Interval& t, const TubeVector& x) const
    {
      IntervalVector eval_result(x.size());
      /*PeriodicFunction pf; // has been removed

      IntervalVector intv_1 = pf.eval_vector(t - 1., x);
      IntervalVector intv_075 = pf.eval_vector(t - 0.75, x);
      IntervalVector intv_05 = pf.eval_vector(t - 0.5, x);
      IntervalVector intv_025 = pf.eval_vector(t - 0.25, x);

      eval_result[0] = x[0](t)
        * (3. - sin(t) - (3. - cos(t)) * x[0](t)
            - (((2. + sin(t)) / 24.) * (intv_1[1] + 6.*intv_075[1] + 4.*intv_05[1] + 10.*intv_025[1] + 3*x[1](t))));
      eval_result[1] = x[1](t)
        * (6. - cos(t) - (10. - sin(t)) * x[1](t)
            - (((2. + sin(t)) / 24.) * (intv_1[0] + 6.*intv_075[0] + 4.*intv_05[0] + 10.*intv_025[0] + 3*x[0](t))));
      */
      return eval_result;
    }
};

void contract(TubeVector& x, double t0, bool incremental)
{
  FncDelayCustom f;

  CtcPicard ctc_picard(1.1);
  ctc_picard.preserve_slicing(true);
  ctc_picard.contract(f, x, TimePropag::FORWARD | TimePropag::BACKWARD);

  CtcDeriv ctc_deriv;
  ctc_deriv.preserve_slicing(true);
  ctc_deriv.contract(x, f.eval_vector(x), TimePropag::FORWARD | TimePropag::BACKWARD);
}

int main()
{
  /* =========== PARAMETERS =========== */

    Tube::enable_syntheses(false);
    int n = 2;
    Vector epsilon(n, 0.4);
    Interval domain(0., 2.*M_PI);
    IntervalVector codomain(2);
    codomain[0] = Interval(0.3, 1.4);
    codomain[1] = Interval(0.2, 0.625);
    TubeVector x(domain, codomain);

    IntervalVector init(2);
    init[0] = Interval(1.25, 1.3);
    init[1] = Interval(0.25, 0.3);
    x.set(init, x.tdomain().lb());
    init[0] = Interval(1.25, 1.3);
    init[1] = Interval(0.25, 0.3);
    x.set(init, x.tdomain().ub());

  /* =========== SOLVER =========== */

    tubex::Solver solver(epsilon);
    solver.set_refining_fxpt_ratio(0.9);
    solver.set_propa_fxpt_ratio(0.9);
    solver.set_var3b_fxpt_ratio(0.);
    list<TubeVector> l_solutions = solver.solve(x, &contract);


  return EXIT_SUCCESS;
}
