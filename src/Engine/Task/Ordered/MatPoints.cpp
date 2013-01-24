/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "MatPoints.hpp"
#include "Waypoint/WaypointVisitor.hpp"
#include "Waypoint/Waypoints.hpp"
#include "Points/OrderedTaskPoint.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"

/**
 * Class to build vector from visited waypoints.
 * Intended to be used temporarily.
 */
class MatWaypointVisitorVector: public WaypointVisitor
{
public:
  enum MaxMattPoints {
    MAX_MAT_POINTS=256,
  };

private:
  MatPoints::MatVector &vector;

  /**
   * only load MAX_MAT_POINTS items for sanity
   */
  unsigned count;

  const AbstractTaskFactory& factory;

public:
/**
   * Constructor
   * @param wpv Vector to add to
   * @return Initialised object
   */
  MatWaypointVisitorVector(MatPoints::MatVector &wpv,
                           const AbstractTaskFactory &_factory)
  :vector(wpv), count(0u), factory(_factory) {}

  /**
   * Visit method, adds result to vector
   * @param wp Waypoint that is visited
   */
  void Visit(const Waypoint& wp) {
    if (wp.IsTurnpoint() && count <= MAX_MAT_POINTS) {
      count++;

      OrderedTaskPoint* tp = (OrderedTaskPoint*)factory.CreateIntermediate(wp);
      vector.push_back(tp);
    }
  }
};

void
MatPoints::FillMatPoints(const Waypoints &wps,
                         const AbstractTaskFactory &factory)
{
  mat_point_vector.reserve(MatWaypointVisitorVector::MAX_MAT_POINTS);
  MatWaypointVisitorVector wvv(mat_point_vector, factory);
  wps.VisitNamePrefix(_T(""), wvv);
}

void
MatPoints::ClearMatPoints()
{
  if (mat_point_vector.empty())
    return;

  for (auto v = mat_point_vector.begin(); v != mat_point_vector.end(); ) {
    delete *v;
    mat_point_vector.erase(v);
  }
  assert(mat_point_vector.empty());
}
