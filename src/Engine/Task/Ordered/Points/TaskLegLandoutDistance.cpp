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

#include "TaskLegLandoutDistance.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/ObservationZones/ObservationZoneClient.hpp"
#include "Task/ObservationZones/Boundary.hpp"

#include <assert.h>
#include <algorithm>


void
TaskLegLandoutDistance::Clear()
{
  oz_point_farthest_left.SetInvalid();
  oz_point_farthest_right.SetInvalid();
  vector_to_oz_farthest_left.SetInvalid();
  vector_to_oz_farthest_right.SetInvalid();
}

fixed
TaskLegLandoutDistance::GetDistance(const GeoPoint &ref) const
{
  if (!ref.IsValid() ||
      !destination.GetLocation().IsValid() ||
      destination.GetPrevious() == nullptr ||
      !destination.GetPrevious()->GetLocationScored().IsValid() ||
      !vector_to_oz_farthest_left.IsValid() ||
      !vector_to_oz_farthest_right.IsValid() ||
      !destination.GetLocationMax().IsValid()) {
    return fixed(0);
  }

  const GeoPoint &start = destination.GetPrevious()->GetLocationScored();
  const GeoVector v = start.DistanceBearing(ref);

  if (v.distance <= vector_to_oz_farthest_left.distance) {

    if (v.bearing > vector_to_oz_farthest_left.bearing &&
        v.bearing < vector_to_oz_farthest_right.bearing) {
      // in the cone that will intersect the oz
      return v.distance;
    } else {
      // not in the cone, but not passed the midpoint of the cone
      if (v.bearing - start.Bearing(destination.GetLocation()) > Angle::Zero()) {
        // on right side
        return vector_to_oz_farthest_right.distance - oz_point_farthest_right.Distance(ref);
      }
      // on left side
      return vector_to_oz_farthest_left.distance - oz_point_farthest_left.Distance(ref);
    }
  }
  // passed midpoint of cylinder
  GeoPoint boundary_ref = GetClosestOZPoint(ref);
  return start.Distance(boundary_ref) - ref.Distance(boundary_ref);
}

bool
TaskLegLandoutDistance::SetLandoutDistanceGeometry()
{
  if (destination.GetPrevious() == nullptr) {
    return true;
  }
  const OZBoundary &oz_boundary = destination.GetBoundary();
  const GeoPoint &start = destination.GetPrevious()->GetLocationScored();
  const Angle bearing_nominal = start.Bearing(destination.GetLocation());

  Angle delta_left = Angle::Zero(); // zero or negative
  Angle delta_right = Angle::Zero(); // zero or positive

  oz_point_farthest_right = oz_point_farthest_left = destination.GetLocation();

  for (auto begin = oz_boundary.cbegin(),
         end = oz_boundary.cend(), i = begin; i != end; ++i) {
    // **i is the & to the point
    Angle bearing = start.Bearing(*i);
    if (bearing - bearing_nominal > delta_right) {
      delta_right = bearing - bearing_nominal;
      oz_point_farthest_right = *i;
    } else if (bearing - bearing_nominal < delta_left) {
      delta_left = bearing - bearing_nominal;
      oz_point_farthest_left = *i;
    }
  }
  vector_to_oz_farthest_left = start.DistanceBearing(oz_point_farthest_left);
  vector_to_oz_farthest_right = start.DistanceBearing(oz_point_farthest_right);

  return true;
}

GeoPoint
TaskLegLandoutDistance::GetClosestOZPoint(const GeoPoint &ref) const
{
  if (destination.GetPrevious() == nullptr || !ref.IsValid()) {
    return GeoPoint::Invalid();
  }

  const OZBoundary &oz_boundary = destination.GetBoundary();
  fixed dist_min = fixed(-1);
  GeoPoint point_min = GeoPoint::Invalid();
  for (auto begin = oz_boundary.cbegin(),
         end = oz_boundary.cend(), i = begin; i != end; ++i) {
    fixed distance = ref.DistanceS(*i);
    if (!positive(dist_min) || distance < dist_min) {
      dist_min = distance;
      point_min = *i;
    }
  }
  return point_min;
}
