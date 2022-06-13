/** 
 *  tubex-lib - Examples
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

void contract(TubeVector& x)
{
TFunction f("x", "-x");

  
  CtcPicard ctc_picard;
  ctc_picard.preserve_slicing(false);
  if (x.volume() > 1.e100)
    ctc_picard.contract(f, x);

  if (x.volume() <  1.e100){

  TubeVector v = f.eval_vector(x);
  CtcDynCid* ctc_dyncid = new CtcDynCid(f);     
    //ctc_dyncid->set_fast_mode(true);
  CtcIntegration ctc_integration(f,ctc_dyncid);
  //  cout << "before contraction " << x << " volume " << x.volume();
  ctc_integration.contract(x,v,x[0].tdomain().lb(),TimePropag::FORWARD) ;
  //  cout << " after forward " <<  x << " volume " << x.volume() << endl;
  ctc_integration.contract(x,v,x[0].tdomain().ub(),TimePropag::BACKWARD) ;
  //  cout << x << " volume " << x.volume() << endl;
  delete ctc_dyncid;
  

  /*

  CtcDeriv ctc_deriv;
  ctc_deriv.preserve_slicing(false);
  ctc_deriv.set_fast_mode(true);
  ctc_deriv.contract(x, f.eval_vector(x));
  */
  }
}

int main()
{
  /* =========== PARAMETERS =========== */
  TFunction f("x", "-x");
    Tube::enable_syntheses(false);
    int n = 1;
    //    Vector epsilon(n, 0.1);
    Vector epsilon(n, 0.5);
    Interval domain(0.,1.);
    TubeVector x(domain,0.1, n);
    x.set(IntervalVector(n, Interval(0.5,1.)*exp(Interval(-0.))), 0.); // initial condition
    TrajectoryVector truth1(domain, TFunction("1.0*exp(-t)"));
    TrajectoryVector truth2(domain, TFunction("0.5*exp(-t)"));

  /* =========== SOLVER =========== */

    tubex::Solver solver(epsilon);
    //    solver.set_refining_fxpt_ratio(0.99999);
    solver.set_refining_fxpt_ratio(2.0);
   
    //    solver.set_propa_fxpt_ratio(0.9999);
    //    solver.set_propa_fxpt_ratio(0.99);
    solver.set_propa_fxpt_ratio(0.);
    //    solver.set_var3b_fxpt_ratio(0.99);
    solver.set_var3b_propa_fxpt_ratio(0.99);
    solver.set_var3b_fxpt_ratio(-1);


    solver.set_var3b_timept(1);
    solver.set_trace(1);
    solver.set_max_slices(10000);
    solver.set_refining_mode(0);
    solver.set_contraction_mode(2);
    solver.set_bisection_timept(-2);

    //    solver.figure()->add_trajectoryvector(&truth1, "truth1");
    //    solver.figure()->add_trajectoryvector(&truth2, "truth2");
    list<TubeVector> l_solutions = solver.solve(x, f);


  // Checking if this example still works:
  Tube hull = TubeVector::hull(l_solutions)[0];
  Tube f_hull = Tube(domain, 0.0001, TFunction("[0.5,1.0]*exp(-t)"));
  return f_hull.is_subset(hull) ? EXIT_SUCCESS : EXIT_FAILURE;
}
