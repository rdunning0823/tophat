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

#include "Device/Simulator.hpp"
#include "NMEA/Info.hpp"
#include "../Simulator.hpp"
#include "Geo/Math.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Thread/Guard.hpp"
#include "Interface.hpp"
#include "Atmosphere/AirDensity.hpp"

#include <stdio.h>

void
Simulator::Init(NMEAInfo &basic)
{
  /* just in case DeviceBlackboard::SetStartupLocation never gets
     called, set some dummy values that are better than uninitialised
     values */

  basic.location = GeoPoint::Zero();
  basic.track = Angle::Zero();
  basic.ground_speed = fixed(0);
  basic.gps_altitude = fixed(0);
  last_airspeed = fixed(0);
  skip_next_glide_speed_calc = false;
}

void
Simulator::Touch(NMEAInfo &basic)
{
  assert(is_simulator());

  basic.UpdateClock();
  basic.alive.Update(basic.clock);
  basic.gps.simulator = true;
  basic.gps.real = false;

  basic.location_available.Update(basic.clock);
  basic.track_available.Update(basic.clock);
  basic.ground_speed_available.Update(basic.clock);
  basic.gps_altitude_available.Update(basic.clock);

  basic.time_available.Update(basic.clock);
  basic.time += fixed(1);
  basic.date_time_utc = basic.date_time_utc + 1;
}

void
Simulator::UpdateGlideAltitude(NMEAInfo &basic, const GlidePolar &polar)
{
  basic.gps_altitude = basic.gps_altitude - polar.SinkRate(basic.true_airspeed);
}

void
Simulator::UpdateGlideSpeed(NMEAInfo &basic, const DerivedInfo &calculated)
{
  fixed new_tas = basic.indicated_airspeed *
      AirDensityRatio(basic.gps_altitude_available.IsValid() ?
      basic.gps_altitude : fixed(0));

  basic.ground_speed = CalcSpeedFromTAS(basic, calculated,
                                        new_tas);
}

void
Simulator::Process(NMEAInfo &basic, const DerivedInfo &calculated)
{
  assert(is_simulator());

  Touch(basic);
  Angle with_random(basic.track + Angle::Degrees(((int)basic.time % 2) - 1));
  basic.location = FindLatitudeLongitude(basic.location, with_random,
                                         basic.ground_speed);
  if (!basic.airspeed_available.IsValid() || !positive(basic.true_airspeed))
    return;

  {
    assert(protected_task_manager != nullptr);
    ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
    const GlidePolar &polar = task_manager->GetGlidePolar();
    fixed airspeed =  basic.true_airspeed;

    if (airspeed > (polar.GetSMin() / 2)) {
      if (last_airspeed < (polar.GetSMin() / 2)) {
        // boost altitude to 3000ft
        basic.gps_altitude += fixed(914);
      } else {
        // gliding
        UpdateGlideAltitude(basic, polar);
        if (!skip_next_glide_speed_calc)
          UpdateGlideSpeed(basic, calculated);
        else
          skip_next_glide_speed_calc = false;
      }
    }

    basic.ProvideNettoVario(fixed(0));
    last_airspeed = airspeed;
  }
}

fixed
Simulator::CalcSpeedFromIAS(const NMEAInfo &basic,
                            const DerivedInfo &calculated, fixed ias)
{
  fixed tas = ias * AirDensityRatio(basic.gps_altitude_available.IsValid() ?
      basic.gps_altitude : fixed(0));

  return CalcSpeedFromTAS(basic, calculated, tas);
}

fixed
Simulator::CalcSpeedFromTAS(const NMEAInfo &basic,
                            const DerivedInfo &calculated,
                            fixed true_air_speed)
{
  const SpeedVector wind = calculated.wind;

  if (positive(true_air_speed) || wind.IsNonZero()) {
    fixed x0 = basic.attitude.heading.fastsine() * true_air_speed;
    fixed y0 = basic.attitude.heading.fastcosine() * true_air_speed;
    x0 -= wind.bearing.fastsine() * wind.norm;
    y0 -= wind.bearing.fastcosine() * wind.norm;

    return SmallHypot(x0, y0);
  }
  return fixed(0);
}
