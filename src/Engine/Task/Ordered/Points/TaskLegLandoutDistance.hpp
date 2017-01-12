/* Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
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

#ifndef TASKLEG_LANDOUT_DISTANCE_H
#define TASKLEG_LANDOUT_DISTANCE_H

#include "Geo/GeoVector.hpp"
#include "Geo/Memento/DistanceMemento.hpp"
#include "Geo/Memento/GeoVectorMemento.hpp"
#include "Compiler.h"

struct AircraftState;

class OrderedTaskPoint;

/**
 * A class to calculate the best task distance for the current leg
 * prior to entering the destination oz
 */

class TaskLegLandoutDistance {
  /**
   * The two points on the edge of the destination oz farthest left and right
   * where the tangent lines touch from the prior point's location scored.
   */
  GeoPoint oz_point_farthest_left;
  GeoPoint oz_point_farthest_right;
  GeoVector vector_to_oz_farthest_left;
  GeoVector vector_to_oz_farthest_right;
  fixed last_distance_nominal;
  Angle last_bearing_nominal;

  OrderedTaskPoint& destination;

public:

  TaskLegLandoutDistance(OrderedTaskPoint &_destination)
    :destination(_destination) {
    Clear();
  }

  /**
   * returns distance for scorable landout not including
   * and scoring adjustements
   * @param ref: location of glider
   */
  fixed GetDistance(const GeoPoint &ref) const;

  /**
   * Find the location on the oz's boundary of the destination point
   * that are the farthest left and right with respect to the origin point's
   * location scored.
   * @param force.  If true, will redo the geometry despite little movement
   * @return true
   */
  bool SetLandoutDistanceGeometry(bool force);

protected:
  /**
   * what is the point on the OZ boundary closest to the ref?
   * @return Geopoint
   */
  GeoPoint GetClosestOZPoint(const GeoPoint &ref) const;

  void Clear();

};

#endif
