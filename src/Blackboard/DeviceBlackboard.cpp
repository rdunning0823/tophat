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

#include "Blackboard/DeviceBlackboard.hpp"
#include "Protection.hpp"
#include "Device/MultipleDevices.hpp"
#include "Simulator.hpp"
#include "RadioFrequency.hpp"
#include "Computer/BasicComputer.hpp"

#include <algorithm>

/**
 * Initializes the DeviceBlackboard
 */
DeviceBlackboard::DeviceBlackboard()
  :devices(nullptr)
{
  // Clear the gps_info and calculated_info
  gps_info.Reset();
  calculated_info.Reset();

  // Set GPS assumed time to system time
  gps_info.UpdateClock();
  gps_info.date_time_utc = BrokenDateTime::NowUTC();
  gps_info.time = fixed(gps_info.date_time_utc.GetSecondOfDay());

  std::fill_n(per_device_data, unsigned(NUMDEV), gps_info);

  real_data = simulator_data = replay_data = gps_info;

  simulator.Init(simulator_data);

  real_clock.Reset();
  replay_clock.Reset();
}

/**
 * Sets the location and altitude to loc and alt
 *
 * Called at startup when no gps data available yet
 * @param loc New location
 * @param alt New altitude
 */
void
DeviceBlackboard::SetStartupLocation(const GeoPoint &loc, const fixed alt)
{
  ScopeLock protect(mutex);

  if (Calculated().flight.flying)
    return;

  for (unsigned i = 0; i < unsigned(NUMDEV); ++i)
    if (!per_device_data[i].location_available)
      per_device_data[i].SetFakeLocation(loc, alt);

  if (!real_data.location_available)
    real_data.SetFakeLocation(loc, alt);

  if (is_simulator()) {
    simulator_data.SetFakeLocation(loc, alt);
    simulator.Touch(simulator_data);
  }

  ScheduleMerge();
}

/**
 * Stops the replay
 */
void DeviceBlackboard::StopReplay() {
  ScopeLock protect(mutex);

  replay_data.Reset();

  ScheduleMerge();
}

void
DeviceBlackboard::ProcessSimulation()
{
  if (!is_simulator())
    return;

  ScopeLock protect(mutex);

  simulator.Process(simulator_data, calculated_info);
  ScheduleMerge();
}

void
DeviceBlackboard::SetSimulatorLocation(const GeoPoint &location)
{
  ScopeLock protect(mutex);
  NMEAInfo &basic = simulator_data;

  basic.track = location.Bearing(basic.location).Reciprocal();
  basic.location = location;

  ScheduleMerge();
}

void
DeviceBlackboard::SetSpeedFromTAS(fixed true_air_speed)
{
  fixed vgnd;
  {
    ScopeLock protect(mutex);
    const NMEAInfo &basic = simulator_data;
    const DerivedInfo &calculated = calculated_info;
    vgnd = simulator.CalcSpeedFromTAS(basic, calculated, true_air_speed);
  }
  SetSpeed(vgnd);
}

void
DeviceBlackboard::SetSpeedFromIAS(fixed indicated_air_speed)
{
  fixed vgnd;
  {
    ScopeLock protect(mutex);
    const NMEAInfo &basic = simulator_data;
    const DerivedInfo &calculated = calculated_info;
    vgnd = simulator.CalcSpeedFromIAS(basic, calculated, indicated_air_speed);
  }
  SetSpeed(vgnd);
}

/**
 * Sets the GPS speed and indicated airspeed to val
 *
 * not in use
 * @param val New speed
 */
void
DeviceBlackboard::SetSpeed(fixed val)
{
  ScopeLock protect(mutex);
  NMEAInfo &basic = simulator_data;

  basic.ground_speed = val;

  ScheduleMerge();
}

/**
 * Sets the TrackBearing to val
 *
 * not in use
 * @param val New TrackBearing
 */
void
DeviceBlackboard::SetTrack(Angle val)
{
  ScopeLock protect(mutex);
  simulator_data.track = val.AsBearing();

  ScheduleMerge();
}

/**
 * Sets the altitude and barometric altitude to val
 *
 * not in use
 * @param val New altitude
 */
void
DeviceBlackboard::SetAltitude(fixed val)
{
  ScopeLock protect(mutex);
  NMEAInfo &basic = simulator_data;

  basic.gps_altitude = val;

  ScheduleMerge();
}

/**
 * Reads the given derived_info usually provided by the
 * GlideComputerBlackboard and saves it to the own Blackboard
 * @param derived_info Calculated information usually provided
 * by the GlideComputerBlackboard
 */
void
DeviceBlackboard::ReadBlackboard(const DerivedInfo &derived_info)
{
  calculated_info = derived_info;
}

/**
 * in Sim mode, the simulator engine needs the airspeed info
 * that is generated in the Basic Computer and written to NMEAInfo
 * So copy it to the device blackboard
 */
void
DeviceBlackboard::ReadSimulatorAirspeeds(const NMEAInfo &basic)
{
  assert(is_simulator());
  simulator_data.airspeed_real = basic.airspeed_real; // always false for sim
  simulator_data.airspeed_available = basic.airspeed_available;
  simulator_data.indicated_airspeed = basic.indicated_airspeed;
  simulator_data.true_airspeed = basic.true_airspeed;
  simulator_data.attitude.heading = basic.attitude.heading;
  simulator_data.attitude.heading_available = basic.attitude.heading_available;
}

/**
 * Reads the given settings usually provided by the InterfaceBlackboard
 * and saves it to the own Blackboard
 * @param settings ComputerSettings usually provided by the
 * InterfaceBlackboard
 */
void
DeviceBlackboard::ReadComputerSettings(const ComputerSettings &settings)
{
  computer_settings = settings;
}

void
DeviceBlackboard::ExpireWallClock()
{
  ScopeLock protect(mutex);
  if (!Basic().alive)
    return;

  bool modified = false;
  for (unsigned i = 0; i < unsigned(NUMDEV); ++i) {
    NMEAInfo &basic = per_device_data[i];
    if (!basic.alive)
      continue;

    basic.ExpireWallClock();
    if (!basic.alive)
      modified = true;
  }

  if (modified)
    ScheduleMerge();
}

void
DeviceBlackboard::ScheduleMerge()
{
  TriggerMergeThread();
}

void
DeviceBlackboard::Merge()
{
  NMEAInfo &basic = SetBasic();

  real_data.Reset();
  for (unsigned i = 0; i < unsigned(NUMDEV); ++i) {
    if (!per_device_data[i].alive)
      continue;

    per_device_data[i].UpdateClock();
    per_device_data[i].Expire();
    real_data.Complement(per_device_data[i]);
  }

  real_clock.Normalise(real_data);

  if (replay_data.alive) {
    replay_data.Expire();
    basic = replay_data;

    /* WrapClock operates on the replay_data copy to avoid feeding
       back BrokenDate modifications to the NMEA parser, as this would
       trigger its time warp checks */
    replay_clock.Normalise(basic);
  } else if (simulator_data.alive) {
    simulator_data.UpdateClock();
    simulator_data.Expire();
    basic = simulator_data;
  } else {
    basic = real_data;
  }
}

void
DeviceBlackboard::MergeSimulatorComputed()
{
  simulator_data.attitude = gps_info.attitude;
}

void
DeviceBlackboard::SetBallast(fixed fraction, fixed overload,
                             OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutBallast(fraction, overload, env);
}

void
DeviceBlackboard::SetBugs(fixed bugs, OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutBugs(bugs, env);
}

void
DeviceBlackboard::SetQNH(AtmosphericPressure qnh, OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutQNH(qnh, env);
}

void
DeviceBlackboard::SetMC(fixed mc, OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutMacCready(mc, env);
}

void
DeviceBlackboard::SetActiveFrequency(RadioFrequency frequency,
                                     const TCHAR *name,
                                     OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutActiveFrequency(frequency, name, env);
}

void
DeviceBlackboard::SetStandbyFrequency(RadioFrequency frequency,
                                      const TCHAR *name,
                                      OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PutStandbyFrequency(frequency, name, env);
}

void
DeviceBlackboard::PlayAlarm(OperationEnvironment &env)
{
  if (devices != nullptr)
    devices->PlayAlarm(env);
}
