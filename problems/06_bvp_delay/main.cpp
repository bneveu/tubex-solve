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
#include "ibex_CtcHC4.h"
#include "ibex_SystemFactory.h"

using namespace std;
using namespace ibex;
using namespace codac;

class FncDelayCustom : public TFnc
{
  public: 

    FncDelayCustom(double delay) : TFnc(1, 1, true), m_delay(delay) { }
    const Interval eval(int slice_id, const TubeVector& x) const { cout << "not defined 1" << endl; }
    const Interval eval(const Interval& t, const TubeVector& x) const { cout << "not defined 2" << endl; }
    const Interval eval(const IntervalVector& x) const { cout << "not defined 3" << endl; }
    const IntervalVector eval_vector(const IntervalVector& x) const { cout << "not defined 4" << endl; }

    const IntervalVector eval_vector(int slice_id, const TubeVector& x) const
    {
      Interval t = x[0].slice(slice_id)->tdomain();
      return eval_vector(t, x);
    }

    const IntervalVector eval_vector(const Interval& t, const TubeVector& x) const
    {
      IntervalVector eval_result(x.size(), Interval::EMPTY_SET);

      if((t - m_delay).lb() <= x.tdomain().lb())
        eval_result |= x(t);

      if((t - m_delay).ub() >= x.tdomain().lb())
        eval_result |= exp(m_delay) * x((t - m_delay) & x.tdomain());

      return eval_result;
    }

  protected:

    double m_delay = 0.;
};

void contract(TubeVector& x, double t0, bool incremental)
{
  // Boundary constraints

    Variable vx0, vx1;
    SystemFactory fac;
    fac.add_var(vx0);
    fac.add_var(vx1);
    fac.add_ctr(sqr(vx0) + sqr(vx1) = 1);
    System sys(fac);
    ibex::CtcHC4 hc4(sys);
    IntervalVector bounds(2);
    bounds[0] = x[0](0.);
    bounds[1] = x[0](1.);
    hc4.contract(bounds);
    x.set(IntervalVector(bounds[0]), 0.);
    x.set(IntervalVector(bounds[1]), 1.);

  // Differential constraint

    double delay = 0.5;
    FncDelayCustom f(delay);
    //tubex::Function f("x", "x");

    CtcPicard ctc_picard;
    //ctc_picard.preserve_slicing(true);
    ctc_picard.contract(f, x);

    // todo: check if this is useful:
    CtcDelay ctc_delay;
    Interval idelay(delay);
    TubeVector v(x, IntervalVector(x.size()));
    ctc_delay.contract(idelay, x, v);
    v *= exp(delay);

    CtcDeriv ctc_deriv;
    ctc_deriv.contract(x, v);
}

int main()
{
  /* =========== PARAMETERS =========== */

    Tube::enable_syntheses(false);
    int n = 1;
    Vector epsilon(n, 0.05);
    Interval domain(0.,1.);
    TubeVector x(domain, n);
    TrajectoryVector truth1(domain, TFunction(" exp(t)/sqrt(1+exp(2))"));
    TrajectoryVector truth2(domain, TFunction("-exp(t)/sqrt(1+exp(2))"));

  /* =========== SOLVER =========== */

    tubex::Solver solver(epsilon);
    solver.set_refining_fxpt_ratio(0.999);
    solver.set_propa_fxpt_ratio(0.999);
    solver.set_var3b_fxpt_ratio(0.);
    //    solver.figure()->add_trajectoryvector(&truth1, "truth1");
    //    solver.figure()->add_trajectoryvector(&truth2, "truth2");
    list<TubeVector> l_solutions = solver.solve(x, &contract);


  // Checking if this example still works:
  return (solver.solutions_contain(l_solutions, truth1) == YES
       && solver.solutions_contain(l_solutions, truth2) == YES) ? EXIT_SUCCESS : EXIT_FAILURE;
}
