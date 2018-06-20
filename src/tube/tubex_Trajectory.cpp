/* ============================================================================
 *  tubex-lib - Trajectory class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2018
 * ---------------------------------------------------------------------------- */

#include <sstream>
#include "tubex_Trajectory.h"
#include "tubex_DomainException.h"
#include "tubex_DimensionException.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  // Definition

  Trajectory::Trajectory()
  {
    
  }

  Trajectory::Trajectory(const Interval& domain, const Function& f)
    : m_domain(domain), m_function(new Function(f))
  {
    DomainException::check(domain);
    IntervalVector box(1, domain);
    m_codomain = m_function->eval_vector(box);
  }

  Trajectory::Trajectory(const map<double,Vector>& map_values)
    : m_map_values(map_values)
  {
    typename map<double,Vector>::const_iterator it_map;
    for(it_map = m_map_values.begin() ; it_map != m_map_values.end() ; it_map++)
    {
      set(it_map->first, it_map->second);
      DimensionException::check(m_map_values.begin()->second, it_map->second);
    }
  }

  Trajectory::~Trajectory()
  {
    if(m_function != NULL)
      delete m_function;
  }

  int Trajectory::dim() const
  {
    if(m_function != NULL)
      return m_function->image_dim();

    else if(m_map_values.size() == 0)
      return 0;
    
    else
      return m_map_values.begin()->second.size();
  }

  // Access values

  const map<double,Vector>& Trajectory::getMap() const
  {
    return m_map_values;
  }

  const Function* Trajectory::getFunction() const
  {
    return m_function;
  }

  const Interval Trajectory::domain() const
  {
    return m_domain;
  }

  const IntervalVector Trajectory::codomain() const
  {
    return m_codomain;
  }

  const Vector Trajectory::operator[](double t) const
  {
    DomainException::check(*this, t);

    if(m_function != NULL)
    {
      IntervalVector box(1, Interval(t));
      return m_function->eval(box).mid();
    }

    else if(m_map_values.find(t) != m_map_values.end())
      return m_map_values.at(t); // key exists

    else
    {
      typename map<double,Vector>::const_iterator it_lower, it_upper;
      it_lower = m_map_values.lower_bound(t);
      it_upper = it_lower;
      it_lower--;

      Vector p(dim());

      // Linear interpolation
      for(int i = 0 ; i < dim() ; i++)
        p[i] = it_lower->second[i] +
               (t - it_lower->first) * (it_upper->second[i] - it_lower->second[i]) /
               (it_upper->first - it_lower->first);

      return p;
    }
  }
  
  const IntervalVector Trajectory::operator[](const Interval& t) const
  {
    DomainException::check(*this, t);

    if(m_domain == t)
      return m_codomain;

    else if(m_function != NULL)
    {
      IntervalVector box(1, Interval(t));
      return m_function->eval_vector(box);
    }

    else
    {
      IntervalVector eval(dim(), Interval::EMPTY_SET);
      eval |= (*this)[t.lb()];
      eval |= (*this)[t.ub()];

      for(map<double,Vector>::const_iterator it = m_map_values.lower_bound(t.lb()) ;
          it != m_map_values.upper_bound(t.ub()) ; it++)
        eval |= it->second;

      return eval;
    }
  }
  
  // Tests

  bool Trajectory::operator==(const Trajectory& x) const
  {
    DimensionException::check(*this, x);

    if(m_function == NULL && x.getFunction() == NULL)
    {
      typename map<double,Vector>::const_iterator it_map;
      for(it_map = m_map_values.begin() ; it_map != m_map_values.end() ; it_map++)
      {
        if(x.getMap().find(it_map->first) == x.getMap().end())
          return false;

        if(it_map->second != x.getMap().at(it_map->first))
          return false;
      }

      return true;
    }

    else if(m_function != NULL && x.getFunction() != NULL)
    {
      throw Exception("Trajectory::operator==",
                      "operator== not implemented in case of Trajectory defined by a Function");
    }

    else
      return false;
  }
  
  bool Trajectory::operator!=(const Trajectory& x) const
  {
    DimensionException::check(*this, x);
    return domain() != x.domain() | codomain() != x.codomain() | !(*this == x);
  }

  // Setting values

  Vector& Trajectory::set(double t, const Vector& y)
  {
    DimensionException::check(*this, y);
    m_map_values.emplace(t, y);
    m_domain |= t;
    m_codomain |= y;
    return m_map_values.at(t);
  }

  void Trajectory::truncateDomain(const Interval& domain)
  {
    DomainException::check(domain);

    map<double,Vector>::iterator it = m_map_values.begin();
    while(it != m_map_values.end())
    {
      if(!domain.contains(it->first)) it = m_map_values.erase(it);
      else ++it;
    }

    m_codomain.set_empty();
    for(map<double,Vector>::iterator it = m_map_values.begin() ; it != m_map_values.end() ; it++)
      m_codomain |= it->second;

    m_domain &= domain;
  }

  void Trajectory::shiftDomain(double shift_ref)
  {
    map<double,Vector> map_temp = m_map_values;
    m_map_values.clear();

    for(map<double,Vector>::iterator it = map_temp.begin() ; it != map_temp.end() ; it++)
      m_map_values.emplace(it->first - shift_ref, it->second);

    m_domain -= shift_ref;
  }
}