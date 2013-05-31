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

#include "AircraftSim.hpp"
#include "Geo/GeoVector.hpp"

void
AircraftSim::Start(const GeoPoint& location_start,
                   const GeoPoint& location_last,
                   const fixed& altitude)
{
  state.Reset();
  state.location = location_start;
  state.altitude = altitude;
  state.time = fixed_zero;
  state.wind.norm = fixed_zero;
  state.wind.bearing = Angle();
  state.ground_speed = fixed(16);
  state_last = state;
  state_last.location = location_last;

  // start with aircraft moving since this isn't a real replay (no time on ground)
  state.flying = true;
}


GeoPoint
AircraftSim::GetEndPoint(const Angle heading, const fixed timestep) const
{
  GeoPoint ref = GeoVector(state.true_airspeed*timestep, heading).EndPoint(state.location);
  return GeoVector(state.wind.norm*timestep,
                   state.wind.bearing+ Angle::Degrees(fixed_180)).EndPoint(ref);
}

void
AircraftSim::Integrate(const Angle heading, const fixed timestep)
{
  GeoPoint v = GetEndPoint(heading, timestep);
  state.track = state.location.Bearing(v);
  state.ground_speed = v.Distance(state.location)/timestep;
  state.location = v;
  state.altitude += state.vario*timestep;
  state.time += timestep;
}


void
AircraftSim::SetWind(const fixed speed, const Angle direction)
{
  state.wind.norm = speed;
  state.wind.bearing = direction;
}


bool
AircraftSim::Update(const Angle heading, const fixed timestep)
{
  state_last = state;
  Integrate(heading, timestep);
  return true;
}
