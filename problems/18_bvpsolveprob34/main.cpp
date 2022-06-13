/** 
 *  tubex-lib - Examples
 *  Solver testcase 18
 * ----------------------------------------------------------------------------
 *
 *  \date       2019
 *  \author     Bertrand Neveu
 *  \copyright  Copyright 2019 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */


#include "math.h"
#include "codac.h"
#include "tubex-solve.h"


using namespace std;
using namespace ibex;
using namespace codac;

void contract(TubeVector& x)

/* example 34  bvpsolve  with ksi=1
   2 tubes solutions 
 */
{
  TFunction f("x1", "x2" ,"(x2;-exp(x1))");

  CtcPicard ctc_picard;
  ctc_picard.preserve_slicing(false);
  if (x.volume() > 50000.)
    ctc_picard.contract(f, x, TimePropag::FORWARD | TimePropag::BACKWARD);

  TubeVector v = f.eval_vector(x);
  
  CtcDeriv ctc_deriv;
  ctc_deriv.set_fast_mode(true);
  ctc_deriv.contract(x, v, TimePropag::FORWARD | TimePropag::BACKWARD);

  /*
  v=f.eval_vector(x);
  
   
  CtcDynCid* ctc_dyncid = new CtcDynCid(f);     
  
  //CtcDynCidGuess*  ctc_dyncid = new CtcDynCidGuess(f);
  ctc_dyncid->set_fast_mode(true);
  CtcIntegration ctc_integration(f,ctc_dyncid);
  ctc_integration.contract(x,v,x[0].domain().lb(),FORWARD) ;
  ctc_integration.contract(x,v,x[0].domain().ub(),BACKWARD) ;
  delete ctc_dyncid;
  */
}

int main()
{
  /* =========== PARAMETERS =========== */
  TFunction f("x1", "x2" ,"(x2;-exp(x1))");
    Tube::enable_syntheses(false);

    Interval domain(0.,1.);
    TubeVector x(domain, 0.01, 2);
    IntervalVector v(2);
    v[0]=Interval(0.,0.);
    //    v[1]=Interval(-1.e8,1.e8);
    v[1]=Interval(-20.,20.);
    //v[1]=Interval(-23.,23.);
    x.set(v, 0.); // ini
    v[0]=Interval(0.,0.);
    v[1]=Interval(-20.,20.);
    //    v[1]=Interval(-23.,23.);
    x.set(v,1.);

    double eps0=0.05;
    double eps1=0.05;
  /* =========== SOLVER =========== */
    Vector epsilon(2);
    epsilon[0]=eps0;
    epsilon[1]=eps1;


    tubex::Solver solver(epsilon);

    //   solver.set_refining_fxpt_ratio(0.99999);
    solver.set_refining_fxpt_ratio(2.0);
    //solver.set_refining_fxpt_ratio(0.9);

    solver.set_propa_fxpt_ratio(0.9999);
    //solver.set_propa_fxpt_ratio(0.999);
    solver.set_propa_fxpt_ratio(0.);
    solver.set_var3b_propa_fxpt_ratio(0.999);

    solver.set_var3b_fxpt_ratio(0.999);
    // solver.set_var3b_fxpt_ratio(-1);
    solver.set_var3b_timept(0);
    solver.set_bisection_timept(3);
    solver.set_trace(1);
    solver.set_max_slices(20000);
    //    solver.set_max_slices(5000);
    solver.set_refining_mode(0);
    solver.set_contraction_mode(2);
    list<TubeVector> l_solutions = solver.solve(x, f);
    cout << "nb sol " << l_solutions.size() << endl;
    return 0;
}
