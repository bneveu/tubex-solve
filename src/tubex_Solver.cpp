/* ============================================================================
 *  tubex-lib - Solver class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou, Bertrand Neveu
 *  Bug fixes : 
 *  Created   : 2018
 * ---------------------------------------------------------------------------- */


#include <time.h>
#include "tubex_Solver.h"
#include "tubex_Exception.h"
#define GRAPHICS 0



using namespace std;
using namespace ibex;

namespace tubex
{
  Solver::Solver(const Vector& max_thickness)
  {
    m_max_thickness = max_thickness;
    
    #if GRAPHICS // embedded graphics
      vibes::beginDrawing();
      m_fig = new VIBesFigTubeVector("Solver");
      m_fig->set_properties(100, 100, 700, 500);
    #endif
  }

  Solver::~Solver()
  {
    #if GRAPHICS
      delete m_fig;
      vibes::endDrawing();
    #endif
  }

  void Solver::set_refining_fxpt_ratio(float refining_fxpt_ratio)
  {
    m_refining_fxpt_ratio = refining_fxpt_ratio;
  }

  void Solver::set_propa_fxpt_ratio(float propa_fxpt_ratio)
  {
    m_propa_fxpt_ratio = propa_fxpt_ratio;
  }

  void Solver::set_var3b_fxpt_ratio(float var3b_fxpt_ratio)
  {
    m_var3b_fxpt_ratio = var3b_fxpt_ratio;
  }

  
   void Solver::set_var3b_propa_fxpt_ratio(float var3b_propa_fxpt_ratio)
  {
    m_var3b_propa_fxpt_ratio = var3b_propa_fxpt_ratio;
  }
  

  void Solver::set_var3b_timept(int var3b_timept)
  {
    m_var3b_timept = var3b_timept;
  }

  void Solver::set_bisection_timept(int bisection_timept)
  {
    m_bisection_timept = bisection_timept;
  }

  void Solver::set_trace(int trace)
  {
    m_trace = trace;
  }

  void Solver::set_max_slices(int max_slices)
  {
    m_max_slices=max_slices;
  }
  
  void Solver::set_refining_mode(int refining_mode)
  {
    m_refining_mode=refining_mode;
  }
    
  void Solver::set_contraction_mode(int contraction_mode)
  {
    m_contraction_mode=contraction_mode;
  }

  void Solver::set_stopping_mode(int stopping_mode)
  {
    m_stopping_mode=stopping_mode;
  }

  void Solver::set_var3b_external_contraction (bool external_contraction)
  {
    m_var3b_external_contraction=external_contraction;
  }


  double Solver::one_finite_gate(const TubeVector &x){
    bool finite=true;
    for (int i=0; i< x.size() ; i++)
      if (x[i].first_slice()->input_gate().diam() >= DBL_MAX)
	{finite=false;break;}
    if (finite==true)
      return x[0].first_slice()->tdomain().lb();
    else{
      finite=true; 
      for (int i=0; i< x.size() ; i++)
	if (x[i].last_slice()->output_gate().diam() >= DBL_MAX)
	{finite=false;break;}
    }
    if (finite==true)
      return x[0].last_slice()->tdomain().ub();
    else{
      for (int i=0 ;i< x.size(); i++){
	for ( const Slice*s= x[i].first_slice(); s!=NULL; s=s->next_slice())
	  if (s->input_gate().diam()<DBL_MAX && s->input_gate().diam()>0  )
	    {return s->tdomain().lb();}
      }
    }
    throw tubex::Exception (" solver " , "solver unable to bisect ");
  }
	       
    

  bool Solver::refining(TubeVector& x)
  {
    int nb_slices=x[0].nb_slices();
    if (nb_slices >= m_max_slices)  // no refining if max_slices is already reached
      return false;

    //   cout << " volume before refining " << x.volume() << endl;

    if  (m_refining_mode == 1)
      {  // one slice is refined
      
      // double t_refining = x[0].wider_slice()->tdomain().mid() // the widest slice            
      double t_refining= x.steepest_slice()->tdomain().mid();    
      x.sample(t_refining);
      // cout << "refining point " << t_refining << endl;
      return true;
    }
    else if 
      (m_refining_mode==0 || x.volume()>= DBL_MAX)
      { // all slices are refined 
	refining_all_slices(x);
	return true;
      }
    else if (m_refining_mode== 2 || m_refining_mode== 3){ // first, 10% of the slices (the widest) are refined
      int nb_refining = nb_slices/10 +1;
      for (int k=0; k< nb_refining; k++){

	double t_refining= x[0].wider_slice()->tdomain().mid();    
	x.sample(t_refining);
	if (k+nb_slices+1 >= m_max_slices) return true;
      }
      refining_with_threshold(x);
      return true;
    }
    else return false;
  }

  void Solver::refining_all_slices(TubeVector & x) {
    int nb_slices=x[0].nb_slices();
    for (int i=0; i<x.size();i++)
	  {
	    int k=0;
	    for ( Slice*s= x[i].first_slice(); s!=NULL; s=s->next_slice()){
	      if (k+nb_slices >= m_max_slices) break;
	      double wid=s->tdomain().diam();
	      x[i].sample(s->tdomain().mid(),s);
	      if (s->tdomain().diam() < wid){ // refining is actually done
		k++;
		s=s->next_slice();
	      }
	    }
	  }
  }


  // the refining is focused on slices  with a larger than average (or median) max difference (in all dimensions)  between input and output gates
  void Solver::refining_with_threshold (TubeVector & x){
    int nb_slices_before = x[0].nb_slices();
    vector<double>  slice_step;

    double step_threshold=0;
    if (m_refining_mode==2) step_threshold= average_refining_threshold(x, slice_step);
    else if (m_refining_mode==3) step_threshold= median_refining_threshold(x, slice_step);
    double min_diam=(x.tdomain().diam() / (100 * nb_slices_before));      
    for (int i=0; i<x.size();i++)
      {
	int k=0; 
	int new_slices=0; 

	for ( Slice*s= x[i].first_slice(); s!=NULL; s=s->next_slice()){
	  if (new_slices +nb_slices_before >= m_max_slices) break;

	  if (slice_step[k] >= step_threshold  && (s->tdomain().diam() > min_diam))
	    {
	      x[i].sample(s->tdomain().mid(),s);
	      //     cout << " sample " << s->tdomain().mid() << endl;
	      s=s->next_slice();
	      new_slices++;
	    }
	  k++;
	}
      }
  }

  



  double Solver::median_refining_threshold (const TubeVector &x, vector<double> & slice_step){

	double step_threshold;
	vector<double> stepmed;
	const Slice* s[x.size()];
	for(int k=0; k< x.size(); k++)
	  s[k]=x[k].first_slice();

	for (const Slice*slice=s[0]; slice!=NULL; slice=slice->next_slice()){
	  //          cout << " t_refining " << slice->tdomain().mid() << endl;
	  double step_max= fabs(slice->output_gate().mid() - slice->input_gate().mid());

	  for (int k=1; k< x.size(); k++){

	    step_max=std::max(step_max,fabs(s[k]->output_gate().mid() - s[k]->input_gate().mid()));

	    s[k]=s[k]->next_slice();
	  }
	  //   cout << " step max " << step_max << endl;
	  slice_step.push_back(step_max);
	  if (step_max < DBL_MAX)  // to not take into account infinite gates in average computation
	    {
	      stepmed.push_back(step_max); // storage for computing the median value
	    }
	}
	
	sort(stepmed.begin(),stepmed.end());
	step_threshold =stepmed[stepmed.size()/2];
	//	cout << " threshold " << step_threshold << endl;
	return step_threshold;
  }

 

  double Solver::average_refining_threshold(const TubeVector &x, vector<double> & slice_step)
    
  {
        int nbsteps=0;
	double step_threshold;

	const Slice* s[x.size()];
	for(int k=0; k< x.size(); k++)
	  s[k]=x[k].first_slice();

	for (const Slice*slice=s[0]; slice!=NULL; slice=slice->next_slice()){

	  double step_max= fabs(slice->output_gate().mid() - slice->input_gate().mid());

	  for (int k=1; k< x.size(); k++){

	    step_max=std::max(step_max,fabs(s[k]->output_gate().mid() - s[k]->input_gate().mid()));

	    s[k]=s[k]->next_slice();
	  }

	  slice_step.push_back(step_max);
	  if (step_max < DBL_MAX)  // to not take into account infinite gates in average computation
	    {
		nbsteps++;
		step_threshold=(step_threshold*(nbsteps-1)+step_max)/nbsteps;
	    }

	}
	return step_threshold;
  }

  void Solver::bisection(const TubeVector& x, list<pair<pair<int,double>,TubeVector> > &s, int level) {
    if (m_trace) cout << "Bisection... (level " << level << ")" << endl;
	    //	    if (f) bisection_guess (x,*f);  //TODO use bisection_guess
	    double t_bisection;
	      if (m_bisection_timept==0){
		if (x.volume() < DBL_MAX)
		  x.max_gate_diam(t_bisection);
		else
		  t_bisection=one_finite_gate(x);
	      }
		  
	      else if (m_bisection_timept==1)
		t_bisection=x[0].tdomain().ub();
	      else if  (m_bisection_timept==-1)
		t_bisection=x[0].tdomain().lb();
	      else if  (m_bisection_timept==2){
		if (rand()%2)
		  t_bisection=x[0].tdomain().lb();
		else
		  t_bisection=x[0].tdomain().ub();
	      }
	      else if  (m_bisection_timept==3){
	      if (level%2)
		t_bisection=x[0].tdomain().lb();
	      else
		t_bisection=x[0].tdomain().ub();
	      }

	   
	      
	    bisections++;
	    level++;
            try{
	      pair<TubeVector,TubeVector> p_x = x.bisect(t_bisection);

	      s.push_front(make_pair(make_pair(level,t_bisection), p_x.second));       
	      s.push_front(make_pair(make_pair(level,t_bisection), p_x.first));
              if (m_trace)	    
		cout << " t_bisection " << t_bisection << " x volume " << x.volume() << " nb_slices " << x.nb_slices()  << endl;
	   
	    }
	    catch (Exception &)   // when the bisection time was not bisectable, change to largest gate
	      {	 
		// cout << " bisection exception " << endl;
		x.max_gate_diam(t_bisection);
	        pair<TubeVector,TubeVector> p_x = x.bisect(t_bisection);

             if (m_trace)	      cout << " t_bisection " << t_bisection << " x volume " << x.volume() << " nb_slices " << x.nb_slices()  << endl;
	    
		s.push_front(make_pair(make_pair(level,t_bisection), p_x.second));     
		s.push_front(make_pair(make_pair(level,t_bisection), p_x.first));
	      }
  }

  
  const list<TubeVector> Solver::solve(const TubeVector& x0,  TFnc& f, void (*ctc_func)(TubeVector&, double t0, bool incremental)) { return (solve(x0,&f,ctc_func));}
  

  const list<TubeVector> Solver::solve(const TubeVector& x0, void (*ctc_func)(TubeVector&,double t0, bool incremental)) {return (solve(x0,NULL, ctc_func));}

 
  const list<TubeVector> Solver::solve(const TubeVector& x0, TFnc* f, void (*ctc_func)(TubeVector&, double t0, bool incremental))

  {
    bisections=0;
    solving_time=0.0;
    assert(x0.size() == m_max_thickness.size());

    int sol_i = 0;
    clock_t t_start = clock();

    #if GRAPHICS
    m_fig->show(true);
    #endif
    
    double t_init=x0[0].tdomain().lb();
    list<pair<pair<int,double>,TubeVector> > s;
    s.push_back(make_pair(make_pair(0,t_init), x0));
    list<TubeVector> l_solutions;

    while(!s.empty())
    {
      int level = s.front().first.first;
      double t_bisect= s.front().first.second;
     
      TubeVector x = s.front().second;
      s.pop_front();

      bool emptiness;
      double volume_before_refining;
      
      bool incremental =0;
      if (level >0) incremental=1;

      contraction_step(x, f, ctc_func,incremental,t_bisect);
     
      emptiness = x.is_empty();
      if (trace && !emptiness)    cout <<  " volume after contraction " << x.volume()  << endl;      
      if (! emptiness)
	do // loop refining; contraction; var3b
      {
        volume_before_refining = x.volume();
        // 1. Refining
	if(m_refining_fxpt_ratio >= 0.0)
	  if (! refining(x))
	    {break;}
	if (m_trace) {
	  cout << " nb_slices after refining step " << x[0].nb_slices() << endl;
	}
	// 2. Contraction after refining
	contraction_step(x, f, ctc_func,false,x[0].tdomain().lb() );
	emptiness = x.is_empty();
	if (m_trace && !emptiness) cout << " volume after contraction " <<  x.volume() << endl;

      }
      
      while(!emptiness
	    && !(stopping_condition_met(x))
	    && !(fixed_point_reached(volume_before_refining, x.volume(), m_refining_fxpt_ratio)));
      // 3. Bisection
      emptiness=x.is_empty();
      if(!emptiness)
        {
          if(stopping_condition_met(x) || m_bisection_timept==-2 )
          {
            l_solutions.push_back(x);
	    /*
            #if GRAPHICS // displaying solution
	      ostringstream o; o << "solution_" << i;
	      m_fig->add_tubevector(&l_solutions.back(), o.str());
              m_fig->show(true);
            #endif
	    */
              sol_i++;
	      if (m_trace) cout << "solution_" << sol_i <<  " vol  " << x.volume() << " max diam " << x.max_diam() << endl;
          }

          else
          {
            bisection(x,s,level);
	  }

    	}
    }
    
    
    if (m_trace){
      cout << endl;
      cout << "Solving time " << (double)(clock() - t_start)/CLOCKS_PER_SEC << endl;
    }

    if (m_trace)
      print_solutions(l_solutions);
   
    while (l_solutions.size()>1)
      {
      int k = l_solutions.size();
      clustering(l_solutions);
      if (k==l_solutions.size())
	{ if (m_trace) cout << " end of clustering " << endl;
	  break;}
      if (m_trace) 
	print_solutions(l_solutions);
      }
    
    double total_time =  (double)(clock() - t_start)/CLOCKS_PER_SEC;
    solving_time=total_time;
    if (m_trace)  cout << "Total time with clustering: " << solving_time << endl;
    if (m_trace) cout << "Number of bisections " << bisections << endl;
    return l_solutions;
    }
  
  // clustering during the search :  not used 
  void Solver::clustering(list<pair<int,TubeVector> >& l_tubes)
  {
    assert(!l_tubes.empty());
    list<pair<int,TubeVector> > l_clustered;
    list<pair<int,TubeVector> >::iterator it1, it2;
    int size= l_tubes.size();
    for(it1 = l_tubes.begin(); it1 != l_tubes.end(); ++it1)     
      {
	bool clustering = false;
	for(it2 = l_clustered.begin(); it2 != l_clustered.end(); ++it2)
	  {
	    if(
	       !((it1->second & it2->second).is_empty())
	       )
	      {
		it2->second = it2->second | it1->second;
		clustering = true;
	      }
	  }
	if(!clustering)
	  l_clustered.push_back(*it1);
      }
    
    l_tubes = l_clustered;
  }


 /* Clustering algorithm for a list of tubes : modify the list by merging tubes that 
    have a non empty intersection 
    fixed point algorithm until no tube can be merged */

  void Solver::clustering(list<TubeVector>& l_tubes)  {
    assert(!l_tubes.empty());
    list<TubeVector> l_clustered;
    list<TubeVector>::iterator it1, it2;
    int nn=0;
    for(it1 = l_tubes.begin(); it1 != l_tubes.end(); ++it1) {
      bool clustering = false;
      int nn1=0;
      for(it2 = l_clustered.begin(); it2 != l_clustered.end(); ++it2) {
		if(!((*it1 & *it2).is_empty()))
	//	if (! empty_intersection(*it1,*it2)) // TO DO : test this algo 
	  {
	    *it2 = (*it1 | *it2);
	    clustering = true;
	    //	cout << ", ti↦" << (*it2)(it2->domain().lb()) << endl;
	    if (m_trace) cout << " tube " << nn+1 << " in cluster " << nn1+1 << endl;
	    break;
	  }
	  nn1++;
      }
      if(!clustering){
	if (m_trace) {cout << "new cluster tube " << nn+1 << endl;
	  cout << ", ti↦" << (*it1)(it1->tdomain().lb()) << endl;}
	l_clustered.push_back(*it1);
      }
      nn++;
    }
    l_tubes = l_clustered;
  }

  /*   more efficient algorithm in case of tubes with different slicings : no need to compute  the tube intersection  TODO test this algorithm */

  bool Solver::empty_intersection( TubeVector & tubevector1, TubeVector & tubevector2)
  {

      for (int k=0; k< tubevector1.size() ; k++)
         
	 {Slice* s2=tubevector2[k].first_slice();
	   Slice* s1=tubevector1[k].first_slice();
	   if ((s2->input_gate()&s1->codomain()).is_empty()
	       ||
	       (s1->input_gate()&s2->codomain()).is_empty())
	     {cout << "first slice " << endl; 
	       return true;}
	     
	   for (const Slice* s= tubevector1[k].first_slice(); s!=NULL; s=s->next_slice()){

	     while (s2->tdomain().ub() <= s->tdomain().lb()){
          
	       s2=s2->next_slice();}
             
	     if ((s2->input_gate()&s->codomain()).is_empty())
	       {cout << " middle s2 " << endl; 
				 cout << *s << endl;
		                 cout << *s2 << endl; 
		 return true;}
	     if (s->tdomain().ub() <= s2->tdomain().lb())
	      if ((s->output_gate() & s2->codomain()).is_empty())
		{cout << " middle s "<< endl;  return true;}
	   
	     s2=tubevector2[k].last_slice();
	     s1=tubevector1[k].last_slice();
	     if ((s2->output_gate()&s1->codomain()).is_empty()
		 ||
		 (s1->output_gate()&s2->codomain()).is_empty())
	       {cout << " last slice " << endl; 
		 return true;}
	   
	   }
	 }
  return false;
  }


  void Solver::print_solutions (const list<TubeVector>& l_solutions){
    int j = 0;
      list<TubeVector>::const_iterator it;
      for(it = l_solutions.begin(); it != l_solutions.end(); ++it)
	{
	  j++;

	  cout << "  " << j << ": "
	       << *it <<  ", ti↦" << (*it)(it->tdomain().lb()) <<  ", tf↦" << (*it)(it->tdomain().ub())
	       << " (max diam: " << it->max_diam() << ")"
	       << " volume : " << it->volume() 
	       << endl;
	}
  }
  
    
  void Solver::display_solutions(const list<TubeVector> & l_solutions) {
      int i=0;
      list<TubeVector>::const_iterator it;
      for(it = l_solutions.begin(); it != l_solutions.end(); ++it){
            #if GRAPHICS // displaying solution
              i++;
	      ostringstream o; o << "solution_" << i;
	      const TubeVector* tv= &(*it);
	      m_fig->add_tube(tv, o.str());
              m_fig->show(true);
            #endif
    
      }
  }

  VIBesFigTubeVector* Solver::figure()
  {
    return m_fig;
  }

  bool Solver::stopping_condition_met(const TubeVector& x)
  {
    if (m_stopping_mode==1)
      return gate_stopping_condition (x);
    else if (m_stopping_mode==2)
      return boundarygate_stopping_condition (x);
    else
      return diam_stopping_condition (x);
  }
 

  bool Solver::diam_stopping_condition(const TubeVector& x)
  {
  
    for(int i = 0 ; i < x.size() ; i++){
      for(const Slice *s = x[i].first_slice() ; s != NULL ; s= s->next_slice())
	{if (s->codomain().diam()> m_max_thickness[i])
	    return false;
	}
    }
    return true;
  }
  

  bool Solver::gate_stopping_condition(const TubeVector& x)
  {
    for(int i = 0 ; i < x.size() ; i++)
      {
	double tmaxgate;
	if(x[i].max_gate_diam(tmaxgate) > m_max_thickness[i])
	  return false;
      }
    return true;
  }

 bool Solver::boundarygate_stopping_condition(const TubeVector& x)
  {
    for(int i = 0 ; i < x.size() ; i++)
      {
	if(
	   x[i].first_slice()->input_gate().diam()  > m_max_thickness[i]
	   ||
	   x[i].last_slice()->output_gate().diam()  > m_max_thickness[i]
	   )
	  return false;
      }
    return true;
  }

  double Solver::extreme_gates_sumofdiams(const TubeVector& x)
  { double sum=0.0;	
    for(int i = 0 ; i < x.size() ; i++)
      {
	//       cout << i << x[i].first_slice()->input_gate() << "  " << x[i].last_slice()->output_gate() << endl;					
       sum+=x[i].first_slice()->input_gate().diam();
       sum+=x[i].last_slice()->output_gate().diam();
      }
    return sum;
  }
      


  bool Solver::fixed_point_reached(double volume_before, double volume_after, float fxpt_ratio)
  {
    if (fxpt_ratio > 1) return false;
    if(fxpt_ratio == 0. || volume_after == volume_before)
      return true;
    return (volume_after / volume_before >= fxpt_ratio);
  }

  //  ------------------------------------------------------CONTRACTION----------------------------------------------
  
  void Solver::picard_contraction (TubeVector &x, const TFnc& f){
    if (x.volume()>= DBL_MAX){
      //      cout << " volume before picard " << x.volume() << endl;
      CtcPicard ctc_picard;
      ctc_picard.preserve_slicing(true);
      ctc_picard.contract(f, x, TimePropag::FORWARD | TimePropag::BACKWARD);
      //   cout << " volume after picard " << x.volume() << endl;
    }
  }

  void Solver::deriv_contraction (TubeVector &x, const TFnc& f, double t0, bool incremental){
    CtcDeriv ctc;
    //    cout << " x before ctc deriv " << x << " volume " << x.volume() << " empty : " << x.is_empty() << endl;
    ctc.set_fast_mode(true);
    //    ctc.contract(x, f.eval_vector(x), TimePropag::FORWARD);
    //    ctc.contract(x, f.eval_vector(x), TimePropag::BACKWARD);
    if (incremental && t0==x.tdomain().lb())
      ctc.contract(x, f.eval_vector(x), TimePropag::FORWARD);
    else if (incremental && t0==x.tdomain().ub())
      ctc.contract(x, f.eval_vector(x), TimePropag::BACKWARD);
    else
      ctc.contract(x, f.eval_vector(x), TimePropag::FORWARD | TimePropag::BACKWARD);

    //    cout << " x  after ctc deriv " << x << " volume " << x.volume() << " empty : " << x.is_empty() <<endl;
  }

  void Solver::integration_contraction(TubeVector &x, const TFnc& f, double t0, bool incremental){
    
    DynCtc* ctc_dyncid;
    CtcIntegration* ctc_integration;
    if (m_contraction_mode==0)
      ctc_dyncid =  new CtcDynBasic(f);
    else if (m_contraction_mode==1) 
      ctc_dyncid =  new CtcDynCid(f);
    else if (m_contraction_mode==2){
      // ctc_dyncid =  new CtcDynCidGuess(f,0.);
      ctc_dyncid =  new CtcDynCidGuess(f);
      //      (dynamic_cast <CtcDynCidGuess*> (ctc_dyncid))->set_variant(1);
      //      (dynamic_cast <CtcDynCidGuess*> (ctc_dyncid))->set_dpolicy(2);
          }
  
    ctc_dyncid->set_fast_mode(true);

    ctc_integration = new CtcIntegration (f,ctc_dyncid);
    //    if(x.volume() >= DBL_MAX ||  x.nb_slices() == 1 ) ctc_integration->set_picard_mode(true);
    if(x.volume() >= DBL_MAX) ctc_integration->set_picard_mode(true);

    TubeVector v = f.eval_vector(x);
    incremental=false ; // stronger contraction without incrementality ; comment this line for incrementality

    if (incremental) // incrementality if incremental and no other function : not used (cf previous line)
      {
	ctc_integration->set_incremental_mode(true);

	ctc_integration->contract(x,v,t0,TimePropag::FORWARD) ;
	//	cout << " after t0 forward " <<  x << endl;
	ctc_integration->contract(x,v,t0,TimePropag::BACKWARD) ;
	//	cout << " after t0 backward " << x << endl;

      }
    else
      {
      ctc_integration->set_incremental_mode(false);
      //      cout << " x before " << x << endl;
      ctc_integration->contract(x,v,x[0].tdomain().lb(),TimePropag::FORWARD);
      //      cout << " x after forward " << x << endl;
      ctc_integration->contract(x,v,x[0].tdomain().ub(),TimePropag::BACKWARD);
      //      cout << " x after backward " << x << endl;
      }
    delete ctc_dyncid; delete ctc_integration;
  }


  void Solver::contraction_step(TubeVector &x, TFnc* f, void (*ctc_func)(TubeVector&, double t0, bool incremental),  bool incremental, double t0) {
    //  Fixed_Point_Contractions up to the fixed point
    fixed_point_contraction(x, f, ctc_func, m_propa_fxpt_ratio, incremental, t0);
    
    bool emptiness = x.is_empty();
    //  Var3b
	
    if (!emptiness && m_var3b_fxpt_ratio >= 0.0)
      fixed_point_var3b(x, f, ctc_func);
    
  }


  /* v3b=true  indicates that  fixed_point_contraction is called from var3b */
  void Solver::fixed_point_contraction(TubeVector &x, TFnc* f, void (*ctc_func)(TubeVector&, double t0, bool incremental), float propa_fxpt_ratio, bool incremental, double t0 , bool v3b)
  {
    if  (propa_fxpt_ratio <0.0) return;
    double volume_before_ctc;
    double volume_after_ctc;
    
    do
      {
	/*	if (m_stopping_mode==2)
	  volume_before_ctc = extreme_gates_sumofdiams(x);
	else
	*/
	  volume_before_ctc = x.volume();
	//	cout << " before contraction 3b " << v3b << endl;
	contraction (x, f, ctc_func, incremental, t0, v3b);
	//	cout << " after contraction 3b " << endl;
	/*
	if (m_stopping_mode==2)
	  volume_after_ctc = extreme_gates_sumofdiams(x);
	else
	*/
	  volume_after_ctc = x.volume();
	
	incremental=false;
      }

    while(!(x.is_empty()) 
	  && propa_fxpt_ratio >0
	  && !fixed_point_reached(volume_before_ctc, volume_after_ctc, propa_fxpt_ratio));
    
  }
  
  
  void Solver::contraction (TubeVector &x, TFnc * f,
			    void (*ctc_func) (TubeVector&,double t0,bool incremental),
			    bool incremental, double t0 , bool v3b)
  {
    if (ctc_func && (!v3b || m_var3b_external_contraction))
      { 
	ctc_func(x, t0, incremental);  // Other constraints contraction
	incremental=false;
      }
    if (f){                     // ODE contraction
	  
      if (m_contraction_mode==4){   // CtcPicard + CtcDeriv
	if (!v3b) picard_contraction(x,*f);
	deriv_contraction(x,*f, t0, incremental);
      }
      else if (m_contraction_mode <=2 ){                   // CtcIntegration 
	  integration_contraction(x,*f,t0,incremental);
      }
	  //	  cout << " tube after contraction " << x << " volume after " << x.volume() << " nb_slices  " << x[0].nb_slices() <<endl;
	  
    }
  }
  
  //----------------- -------------------- VAR3B ----------------------------------


  void Solver::fixed_point_var3b(TubeVector &x, TFnc * f,void (*ctc_func) (TubeVector&,double t0,bool incremental)){
      double volume_before_var3b;
      double emptiness;
      if (m_var3b_fxpt_ratio>=0.0)
      do
	  { 
	    volume_before_var3b=x.volume();
	    if  (x.volume() < DBL_MAX)
	      cout << " volume before var3b "  << x.volume() << " x " << x << endl;
	    for (int i=0; i< x.size() ; i++)
	      cout << i << " " << x[i].first_slice()->input_gate() << endl;
	    var3b(x, f, ctc_func);
	    cout << " volume after var3b "  << x.volume() << " x " << x <<  endl;
	     for (int i=0; i< x.size() ; i++)
	      cout << i << " " << x[i].first_slice()->input_gate() << endl;
	    emptiness = x.is_empty();
	  }
      while (!emptiness  
	     && !(stopping_condition_met(x))
	     && !fixed_point_reached(volume_before_var3b, x.volume(), m_var3b_fxpt_ratio));
    }

  void Solver::var3b(TubeVector &x, TFnc * f,void (*ctc_func) (TubeVector&,double t0,bool incremental))
  {
    //    cout << " volume before var3b " << x.volume() << endl;

   
    int contraction_mode = m_contraction_mode;
    m_contraction_mode=4;  // var3B calls CtcDeriv as internal ODE contractor 
    // incremental contractors using CtcIntegration are too weak and non incremental contractors as
    // CtcIntegration with CtcDyncid or CtcDyncidGuess are too costly
    //    cout << " var3b " << x << endl;
    double t_bisection;
    if (m_var3b_timept==1)
      t_bisection=x[0].tdomain().ub();
    else if (m_var3b_timept==-1)
      t_bisection=x[0].tdomain().lb();
    else  if (m_var3b_timept==2){
      if (rand()%2)
	t_bisection=x[0].tdomain().lb();
      else
	t_bisection=x[0].tdomain().ub();
    }
    else
      x.max_gate_diam(t_bisection);  
    cout << " t_bisection var3b " << t_bisection << endl;
    for(int k=0; k<x.size() ; k++)
      {

	double rate=m_var3b_bisection_minrate;

	while (rate < m_var3b_bisection_maxrate){
	  try
	    {pair<TubeVector,TubeVector> p_x = x.bisect(t_bisection,k,rate);
             

	      fixed_point_contraction(p_x.first, f, ctc_func, m_var3b_propa_fxpt_ratio, true, t_bisection, true);

              if (p_x.first.is_empty())
		x=p_x.second;
	     //	     else {x = p_x.second | p_x.first ; break;}
	     else {p_x.second|= p_x.first; x = p_x.second  ; break;} // no slicing

	     rate= m_var3b_bisection_ratefactor*rate;

	    }
	  catch (Exception& )
	    {break;}
	}


	fixed_point_contraction(x,f, ctc_func, m_var3b_propa_fxpt_ratio, true, t_bisection, true);

	rate = 1 - m_var3b_bisection_minrate;
       
	while (rate > 1-m_var3b_bisection_maxrate ){
	  try{
	    pair<TubeVector,TubeVector> p_x = x.bisect(t_bisection,k,rate);


	    fixed_point_contraction(p_x.second, f, ctc_func, m_var3b_propa_fxpt_ratio, true, t_bisection, true);

	    if (p_x.second.is_empty())
	       x=p_x.first;
	     //	     else {x = p_x.first |  p_x.second ; break;}
	     else {p_x.first |= p_x.second ; x= p_x.first ; break;}   // no slicing
	     rate=1-m_var3b_bisection_ratefactor*(1-rate);

	  }
	  catch (Exception& )
	    {break;}
	}
	fixed_point_contraction(x,f , ctc_func, m_var3b_propa_fxpt_ratio, true, t_bisection,true);

	
      }
    m_contraction_mode=contraction_mode;
    //    cout << " volume after var3b " << x.volume() << endl;
  }



 const BoolInterval Solver::solutions_contain(const list<TubeVector>& l_solutions, const TrajectoryVector& truth)
  {
    assert(!l_solutions.empty());

    BoolInterval result = NO;
    list<TubeVector>::const_iterator it;
    for(it = l_solutions.begin() ; it != l_solutions.end() ; ++it)
    {
      assert(truth.size() == it->size());
      BoolInterval b = it->contains(truth);
      if(b == YES) return YES;
      else if(b == MAYBE) result = MAYBE;
    }
    
    return result;
  }
}
