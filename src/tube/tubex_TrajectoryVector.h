/* ============================================================================
 *  tubex-lib - TrajectoryVector class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2018
 * ---------------------------------------------------------------------------- */

#ifndef TrajectoryVector_HEADER
#define TrajectoryVector_HEADER

#include <map>
#include "ibex_Vector.h"
#include "ibex_Interval.h"
#include "ibex_Function.h"
#include "tubex_DynamicalItem.h"

namespace tubex
{
  class TrajectoryVector : DynamicalItem
  {
    public:

      // Definition
      TrajectoryVector();
      TrajectoryVector(const ibex::Interval& domain, const ibex::Function& f);
      TrajectoryVector(const std::map<double,ibex::Vector>& m_map_values);
      TrajectoryVector(const TrajectoryVector& traj);
      ~TrajectoryVector();
      int dim() const;

      // Access values
      const std::map<double,ibex::Vector>& getMap() const;
      const ibex::Function* getFunction() const;
      const ibex::Interval domain() const; // todo: output const Interval& (reference)
      const ibex::IntervalVector codomain() const; // todo: output const Interval& (reference)
      const ibex::IntervalVector codomainBox() const; // todo: output const Interval& (reference)
      const ibex::Vector operator[](double t) const;
      const ibex::IntervalVector operator[](const ibex::Interval& t) const;

      // Tests
      bool notDefined() const;
      bool operator==(const TrajectoryVector& x) const;
      bool operator!=(const TrajectoryVector& x) const;

      // Setting values
      ibex::Vector& set(double t, const ibex::Vector& y);
      void truncateDomain(const ibex::Interval& domain);
      void shiftDomain(double shift_ref);

      // String
      friend std::ostream& operator<<(std::ostream& str, const TrajectoryVector& x);


    protected:

      /** Class variables **/

        ibex::Interval m_domain = ibex::Interval::EMPTY_SET;
        ibex::IntervalVector m_codomain = ibex::IntervalVector(1, ibex::Interval::EMPTY_SET);
        // A trajectory is defined either by a Function or a map of values
        ibex::Function *m_function = NULL;
        std::map<double,ibex::Vector> m_map_values;
  };
}

#endif