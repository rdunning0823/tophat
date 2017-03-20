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

#include "FlyingComputer.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "Engine/Navigation/Aircraft.hpp"
#include "Task/SaveFile.hpp"

void
FlyingComputer::Reset()
{
  delta_time.Reset();

  stationary_clock.Clear();
  moving_clock.Clear();
  climbing_clock.Clear();
  moving_since = fixed(-1);
  stationary_since = fixed(-1);
  climbing_altitude = fixed(0);
  climbing_clock_dt_since_update = fixed(0);
  sinking_since = fixed(-1);
  powered_since = fixed(-1);
  unpowered_since = fixed(-1);
  last_ground_altitude = fixed(-1);
}

void
FlyingComputer::CheckRelease(FlyingState &state, fixed time,
                             const GeoPoint &location, fixed altitude)
{
  if (!state.flying || !negative(state.release_time) ||
      stationary_clock.IsDefined())
    return;

  if (negative(sinking_since)) {
    sinking_since = time;
    sinking_location = location;
    sinking_altitude = altitude;
    return;
  }

  if (time < sinking_since || altitude >= sinking_altitude) {
    /* cancel release detection when the aircraft has been climbing
       more than it has lost recently */
    sinking_since = fixed(-1);
    return;
  }

  if (time - sinking_since >= fixed(10)) {
    /* assume release from tow if aircraft has not gained any height
       for 10 seconds; there will be false negatives if the glider
       enters a thermal right after releasing */
    state.release_time = sinking_since;
    state.release_location = sinking_location;
    state.far_location.SetInvalid();
    state.far_distance = fixed(-1);
  }
}

void
FlyingComputer::Check(FlyingState &state, fixed time)
{
  // Logic to detect takeoff and landing is as follows:
  //   detect takeoff when above threshold speed for 10 seconds
  //
  //   detect landing when below threshold speed for 30 seconds

  if (!state.flying) {
    // We are moving for 10sec now
    if (moving_clock >= fixed(10)) {
      // We certainly must be flying after 10sec movement
      assert(!negative(moving_since));

      state.flying = true;
      state.takeoff_time = moving_since;
      state.takeoff_location = moving_at;
      state.takeoff_altitude = moving_at_altitude;
      state.flight_time = fixed(0);

      /* when a new flight starts, forget the old release and power-on/off time */
      state.release_time = fixed(-1);
      state.power_on_time = fixed(-1);
      state.power_off_time = fixed(-1);
      state.far_location.SetInvalid();
      state.far_distance = fixed(-1);
    }
  } else {
    // update time of flight
    state.flight_time = time - state.takeoff_time;

    // We are not moving anymore for 60sec now
    if (!moving_clock.IsDefined()) {
      // We are probably not flying anymore
      assert(!negative(stationary_since));

      state.flying = false;
      state.flight_time = stationary_since - state.takeoff_time;
      state.landing_time = stationary_since;
      state.landing_location = stationary_at;
    }
  }

  // If we are not certainly flying we are probably on the ground
  // To make sure that we are, wait for 10sec to make sure there
  // is no more movement
  state.on_ground = !state.flying && stationary_clock >= fixed(10);
}

void
FlyingComputer::Moving(FlyingState &state, fixed time, fixed dt,
                       const GeoPoint &location, fixed altitude)
{
  // Increase InFlight countdown for further evaluation
  moving_clock.Add(dt);

  if (negative(moving_since)) {
    moving_since = time;
    moving_at = location;
    moving_at_altitude = altitude;
  }

  // We are moving so we are certainly not on the ground
  stationary_clock.Clear();
  stationary_since = fixed(-1);

  // Update flying state
  Check(state, time);
}

void
FlyingComputer::Stationary(FlyingState &state, fixed time, fixed dt,
                           const GeoPoint &location)
{
  // Decrease InFlight countdown for further evaluation
  if (moving_clock.IsDefined()) {
    moving_clock.Subtract(dt);
    if (!moving_clock.IsDefined())
      moving_since = fixed(-1);
  }

  stationary_clock.Add(dt);

  if (negative(stationary_since)) {
    stationary_since = time;
    stationary_at = location;
  }

  // Update flying state
  Check(state, time);
}

gcc_pure
static bool
CheckTakeOffSpeed(fixed takeoff_speed, const NMEAInfo &basic)
{
  // Speed too high for being on the ground
  // don't use airspeed b/c CAI302's TAS does not fall below 5-10 m/s
  return basic.ground_speed >= takeoff_speed;
}

/**
 * After take-off has been detected, we check if the ground speed goes
 * below a certain threshold that indicates the aircraft has ceased
 * flying.  To avoid false positives while wave/ridge soaring, this
 * threshold is half of the given take-off speed.
 * Don't use ASI because a breeze on the ground can easily trigger
 * false motion
 */
gcc_pure
static bool
CheckLandingSpeed(fixed takeoff_speed, const NMEAInfo &basic)
{
  const fixed speed = basic.ground_speed;
  // Speed too high for being on the ground
  return speed < Half(takeoff_speed);
}

gcc_pure
static bool
CheckAltitudeAGL(const DerivedInfo &calculated)
{
  return calculated.altitude_agl_valid && calculated.altitude_agl >= fixed(300);
}

inline bool
FlyingComputer::CheckClimbing(fixed dt, fixed altitude)
{
  enum Constants {
    DeltaInterval = 4,
    DeltaHeight = 2,
  };
  climbing_clock_dt_since_update += dt;

  if (climbing_clock_dt_since_update > fixed(DeltaInterval)) {
    dt = climbing_clock_dt_since_update;
    climbing_clock_dt_since_update = fixed(0);

    /* is fix DeltaHeight m above low point during DeltaInterval? */
    if (altitude > climbing_altitude + fixed(DeltaHeight))
      climbing_clock.Add(dt);
    else
      climbing_clock.Subtract(dt);

    climbing_altitude = altitude;
  } else
    climbing_altitude = std::min(altitude, climbing_altitude);

  return climbing_clock >= dt + fixed(1);
}

inline void
FlyingComputer::CheckPowered(fixed dt, const NMEAInfo &basic, FlyingState &flying)
{
  if (basic.engine_noise_level > 500 &&
      negative(powered_since)) {
    powered_since = basic.time;
    powered_at = basic.location;

    unpowered_since = fixed(-1);
    unpowered_at.SetInvalid();
  } else if (basic.engine_noise_level <= 350 &&
             negative(unpowered_since)) {
    unpowered_since = basic.time;
    unpowered_at = basic.location;

    powered_since = fixed(-1);
    powered_at.SetInvalid();
  }

  if (!negative(powered_since) && negative(unpowered_since) && basic.time - powered_since >= fixed(30)) {
    flying.powered = true;
    flying.power_on_time = powered_since;
    flying.power_on_location = powered_at;
  } else if (!negative(unpowered_since) && basic.time - unpowered_since >= fixed(30)) {
    flying.powered = false;
    flying.power_off_time = unpowered_since;
    flying.power_off_location = unpowered_at;
  }
}

void
FlyingComputer::Compute(fixed takeoff_speed,
                        const NMEAInfo &basic,
                        const DerivedInfo &calculated,
                        FlyingState &flying)
{
  if (!basic.time_available || !basic.location_available)
    return;

  const fixed dt = delta_time.Update(basic.time, fixed(0.5), fixed(20));
  if (negative(dt)) {
    Reset();
    flying.Reset();
  }

  if (!positive(dt))
    return;

  const auto any_altitude = basic.GetAnyAltitude();

  if (!basic.airspeed_available && !calculated.altitude_agl_valid &&
      any_altitude.first && !negative(last_ground_altitude) &&
      any_altitude.second > last_ground_altitude + fixed(250)) {
    /* lower the threshold for "not moving" when the aircraft is high
       above the take-off airfield and there's no airspeed probe; this
       shall reduce the risk of false landing detection when flying in
       strong head wind (e.g. ridge or wave) */
    fixed dh = any_altitude.second - last_ground_altitude;

    if (dh > fixed(1000))
      takeoff_speed /= 4;
    else if (dh > fixed(500))
      takeoff_speed /= 2;
    else
      takeoff_speed = takeoff_speed * 2 / 3;
  }

  if (CheckTakeOffSpeed(takeoff_speed, basic) ||
      CheckAltitudeAGL(calculated))
    Moving(flying, basic.time, dt, basic.location, basic.gps_altitude);
  else if (!flying.flying ||
           (CheckLandingSpeed(takeoff_speed, basic) &&
            (!any_altitude.first || !CheckClimbing(dt, any_altitude.second))))
    Stationary(flying, basic.time, dt, basic.location);

  /// if we've been sitting on the ground, then don't resume task on takeoff
  if (positive(stationary_since) && (basic.time - stationary_since > fixed(120))) {
    RemoveTaskState();
  }

  if (basic.engine_noise_level_available)
    CheckPowered(dt, basic, flying);

  if (any_altitude.first) {
    if (flying.on_ground)
      last_ground_altitude = any_altitude.second;

    CheckRelease(flying, basic.time, basic.location, any_altitude.second);
  } else
    sinking_since = fixed(-1);

  if (flying.flying && flying.release_location.IsValid()) {
    fixed distance = basic.location.Distance(flying.release_location);
    if (distance > flying.far_distance) {
      flying.far_location = basic.location;
      flying.far_distance = distance;
    }
  }
}

void
FlyingComputer::Compute(fixed takeoff_speed,
                        const AircraftState &state, fixed dt,
                        FlyingState &flying)
{
  if (negative(state.time))
    return;

  if (state.ground_speed > takeoff_speed)
    Moving(flying, state.time, dt, state.location, state.altitude);
  else
    Stationary(flying, state.time, dt, state.location);
}

void
FlyingComputer::Finish(FlyingState &flying, fixed time)
{
  if (flying.flying && stationary_clock >= fixed(5))
    moving_clock.Clear();

  Check(flying, time);
}
