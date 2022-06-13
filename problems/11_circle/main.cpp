#include "math.h"
#include "codac.h"
#include "tubex-solve.h"
#include <iomanip>

using namespace std;
 using namespace ibex;
using namespace codac;

void contract(TubeVector& x)
{
TFunction f("x1", "x2" ,"(-x2;x1)");
  

  CtcPicard ctc_picard;
  ctc_picard.preserve_slicing(false);
  if (x.volume() > 5000)
    ctc_picard.contract(f, x, TimePropag::FORWARD);
  /*
  CtcDeriv ctc_deriv;
  ctc_deriv.set_fast_mode(true);
  ctc_deriv.contract(x, f.eval_vector(x), FORWARD | BACKWARD);
  */
  
  TubeVector v = f.eval_vector(x);
  CtcDynCid* ctc_dyncid = new CtcDynCid(f);     
  //  CtcDynCidGuess* ctc_dyncid = new CtcDynCidGuess(f);     
  ctc_dyncid->set_fast_mode(true);
  CtcIntegration ctc_integration(f,ctc_dyncid);

  ctc_integration.contract(x,v,x[0].tdomain().lb(),TimePropag::FORWARD) ;

  ctc_integration.contract(x,v,x[0].tdomain().ub(),TimePropag::BACKWARD) ;

  delete ctc_dyncid;
  
}

int main()
{
TFunction f("x1", "x2" ,"(-x2;x1)");
  /* =========== PARAMETERS =========== */
  double pi=M_PI;
    Tube::enable_syntheses(false);


    double volume=0.0;
    double totaltime=0.0;


    Interval domain(0.,pi);
    TubeVector x(domain, 0.1, 2);
    IntervalVector v(2);
    vector<IntervalVector*> gates;
    v[0]=Interval(0.,0.);
    v[1]=Interval(1.,1.);
    x.set(v, 0.); // ini
    cout << x[0](0.) << " "  << x[1](0.) << endl;

    double eps=0.002;
    double step=pi;
    int nbsteps=1;
  /* =========== SOLVER =========== */
    for (int i=0; i< nbsteps; i++){
      //   Vector epsilon(2, eps*(i+1));
      Vector epsilon(2, eps);
      double t0=i*step;
      double t1=(i+1)*step;
      Interval domain(t0,t1);
      TubeVector x(domain, step, 2);
      //      TubeVector x(domain, 0.1, 2);
      x.set(v,t0 ); // initial condition
      tubex::Solver solver(epsilon);
      solver.set_refining_fxpt_ratio(2.);
      //solver.set_refining_fxpt_ratio(0.9999);
      //      solver.set_propa_fxpt_ratio(0.99);
      solver.set_propa_fxpt_ratio(0);
      //solver.set_var3b_fxpt_ratio(0.99);
      //      solver.set_var3b_propa_fxpt_ratio(0.99);
      solver.set_var3b_fxpt_ratio(-1.);

      solver.set_max_slices(40000);
      solver.set_var3b_timept(1);
      solver.set_refining_mode(0);
      solver.set_contraction_mode(2);
      solver.set_trace(1);


    list<TubeVector> l_solutions = solver.solve(x, f);
    cout << "nb sol " << l_solutions.size() << endl;
    if (l_solutions.size()==1) { cout << " volume " << l_solutions.front().volume() << endl;
      volume+=l_solutions.front().volume();
      totaltime+=solver.solving_time;
      v[0]=l_solutions.front()[0].last_slice()->output_gate();
      v[1]=l_solutions.front()[1].last_slice()->output_gate();
      IntervalVector* v1 = new IntervalVector(v);
      gates.push_back(v1);

    }
    else break;

    }
    cout << " total volume " << volume << " total time " << totaltime << endl;;
    /*    for (int k=0; k< gates.size(); k++)
      cout << k << "  " << *(gates[k]) << endl;
    */
    cout << " last gate " << *(gates[gates.size()-1]) << endl;
    return 0;
}
