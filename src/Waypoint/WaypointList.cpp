/*
Copyright_License {

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

#include "WaypointList.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Interface.hpp"
#include "NMEA/MoreData.hpp"
#include "NMEA/Derived.hpp"
#include "Engine/GlideSolvers/GlideState.hpp"
#include "Engine/GlideSolvers/GlideResult.hpp"
#include "Engine/GlideSolvers/MacCready.hpp"

#include <algorithm>

void
WaypointListItem::ResetVector()
{
  vec.SetInvalid();
}

const GeoVector &
WaypointListItem::GetVector(const GeoPoint &location) const
{
  if (!vec.IsValid())
    vec = GeoVector(location, waypoint->location);

  return vec;
}

class WaypointDistanceCompare
{
  const GeoPoint &location;

public:
  WaypointDistanceCompare(const GeoPoint &_location):location(_location) {}

  bool operator()(const WaypointListItem &a,
                  const WaypointListItem &b) const {
    return a.GetVector(location).distance < b.GetVector(location).distance;
  }
};

void WaypointList::SortByDistance(const GeoPoint &location) {
  std::sort(begin(), end(), WaypointDistanceCompare(location));
}

class WaypointNameCompare
{
public:
  WaypointNameCompare() {}

  bool operator()(const WaypointListItem &a,
                  const WaypointListItem &b) const {
    return a.waypoint->name < b.waypoint->name;
  }
};

void WaypointList::SortByName() {
  std::sort(begin(), end(), WaypointNameCompare());
}

class WaypointElevationCompare
{
public:
  WaypointElevationCompare() {}

  bool operator()(const WaypointListItem &a,
                  const WaypointListItem &b) const {
    return a.waypoint->elevation < b.waypoint->elevation;
  }
};

void WaypointList::SortByElevation() {
  std::sort(begin(), end(), WaypointElevationCompare());
}

class WaypointArrivalAltitudeCompare
{
  const MoreData &more_data;
  const DerivedInfo &calculated;
  const GeoPoint &location;
  const ComputerSettings &settings;

public:
  WaypointArrivalAltitudeCompare(const GeoPoint &_location)
  :more_data(CommonInterface::Basic()),
   calculated(CommonInterface::Calculated()),
   location(_location),
   settings(CommonInterface::GetComputerSettings()) { }

  fixed CalcAltitudeDifferential(const WaypointListItem &a) const
  {
    // altitude differential
    const GlideState glide_state(
      location.DistanceBearing(a.waypoint->location),
      a.waypoint->elevation + settings.task.safety_height_arrival,
      more_data.nav_altitude,
      calculated.GetWindOrZero());

    const GlideResult &result =
      MacCready::Solve(settings.task.glide,
                       settings.polar.glide_polar_task,
                       glide_state);
    return result.SelectAltitudeDifference(settings.task.glide);
  }


  bool operator()(const WaypointListItem &a,
                  const WaypointListItem &b) const {

    return CalcAltitudeDifferential(a) > CalcAltitudeDifferential(b);
  }
};

void WaypointList::SortByArrivalAltitude(const GeoPoint &location) {
  std::sort(begin(), end(), WaypointArrivalAltitudeCompare(location));
}
