/*
  Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#include "Replay.hpp"
#include "IgcReplay.hpp"
#include "NmeaReplay.hpp"
#include "DemoReplayGlue.hpp"
#include "Util/StringUtil.hpp"
#include "Util/Clamp.hpp"
#include "OS/PathName.hpp"
#include "IO/FileLineReader.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Logger/Logger.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "CatmullRomInterpolator.hpp"

#include <assert.h>

bool
Replay::Rewind(fixed delta_s) {
  if (IsActive() && !negative(virtual_time)) {
    fixed rewind_save = std::max(virtual_time - delta_s, first_virtual_time);
    fixed first_virtual_time_save = first_virtual_time;
    Start(path);
    first_virtual_time = first_virtual_time_save;
    virtual_time = rewind = rewind_save;
    return true;
  } else
    return false;
}

void
Replay::Stop()
{
  play_state = Replay::PlayState::NOTSTARTED;

  if (replay == nullptr)
    return;

  Timer::Cancel();

  delete replay;
  replay = nullptr;

  delete cli;
  cli = nullptr;

  device_blackboard->StopReplay();

  if (logger != nullptr)
    logger->ClearBuffer();
}

bool
Replay::Start(const TCHAR *_path)
{
  assert(_path != nullptr);

  /* make sure the old AbstractReplay instance has cleaned up before
     creating a new one */
  Stop();

  _tcscpy(path, _path);

  if (StringIsEmpty(path)) {
    replay = new DemoReplayGlue(task_manager);
  } else if (MatchesExtension(path, _T(".igc"))) {
    auto reader = new FileLineReaderA(path);
    if (reader->error()) {
      delete reader;
      return false;
    }

    replay = new IgcReplay(reader);

    cli = new CatmullRomInterpolator(fixed(0.98));
    cli->Reset();
  } else {
    auto reader = new FileLineReaderA(path);
    if (reader->error()) {
      delete reader;
      return false;
    }

    replay = new NmeaReplay(reader,
                            CommonInterface::GetSystemSettings().devices[0]);
  }

  if (logger != nullptr)
    logger->ClearBuffer();

  virtual_time = fixed(-1);
  first_virtual_time = fixed(-1);
  fast_forward = fixed(-1);
  rewind = fixed(-1);
  next_data.Reset();
  play_state = Replay::PlayState::PLAYING;

  Timer::Schedule(100);

  return true;
}

bool
Replay::Update()
{
  if (replay == nullptr)
    return false;

  if (!positive(time_scale) || play_state == PlayState::PAUSED) {
    /* replay is paused */
    /* to avoid a big fast-forward with the next
       PeriodClock::ElapsedUpdate() call below after unpausing, update
       the clock each time we're called while paused */
    clock.Update();
    return true;
  }

  const fixed old_virtual_time = virtual_time;

  if (!negative(virtual_time)) {
    /* update the virtual time */
    assert(clock.IsDefined());

    if (!negative(rewind)) {
      clock.Update();
      virtual_time = rewind;

    } else if (negative(fast_forward)) {
      if (negative(first_virtual_time))
          first_virtual_time = next_data.time;
      virtual_time += clock.ElapsedUpdate() * time_scale / 1000;

    } else {
      clock.Update();
      virtual_time += fixed(60);
      if (virtual_time >= fast_forward)
        fast_forward = fixed(-1);
    }
  } else {
    /* if we ever received a valid time from the AbstractReplay, then
       virtual_time must be initialised */
    assert(!next_data.time_available);
  }

  if (!negative(rewind)) {
    while (true) {
      if (!replay->Update(next_data)) {
        Stop();
        return false;
      }

      assert(!next_data.gps.real);

      if (next_data.time_available) {
        if (next_data.time >= virtual_time) {
          rewind = fixed(-1);
          break;
        }
      }
    }
  } else if (cli == nullptr || !negative(fast_forward)) {
    if (next_data.time_available && virtual_time < next_data.time)
      /* still not time to use next_data */
      return true;

    {
      ScopeLock protect(device_blackboard->mutex);
      device_blackboard->SetReplayState() = next_data;
      device_blackboard->ScheduleMerge();
    }

    while (true) {
      if (!replay->Update(next_data)) {
        Stop();
        return false;
      }

      assert(!next_data.gps.real);

      if (next_data.time_available) {

        if (negative(virtual_time)) {
          virtual_time = next_data.time;
          if (!negative(fast_forward))
            fast_forward += virtual_time;
          clock.Update();
          break;
        }

        if (next_data.time >= virtual_time)
          break;

        if (next_data.time < old_virtual_time) {
          /* time warp; that can happen on midnight wraparound during
             NMEA replay */
          virtual_time = next_data.time;
          break;
        }
      }
    }
  } else {
    while (cli->NeedData(virtual_time)) {
      if (!replay->Update(next_data)) {
        Stop();
        return false;
      }

      assert(!next_data.gps.real);

      if (next_data.time_available)

        cli->Update(next_data.time, next_data.location,
                    next_data.gps_altitude,
                    next_data.pressure_altitude);
    }

    if (negative(virtual_time)) {
      virtual_time = cli->GetMaxTime();
      if (!negative(fast_forward))
        fast_forward += virtual_time;
      clock.Update();
    }

    const CatmullRomInterpolator::Record r = cli->Interpolate(virtual_time);
    const GeoVector v = cli->GetVector(virtual_time);

    NMEAInfo data = next_data;
    data.clock = virtual_time;
    data.alive.Update(data.clock);
    data.ProvideTime(virtual_time);
    data.location = r.location;
    data.location_available.Update(data.clock);
    data.ground_speed = v.distance;
    data.ground_speed_available.Update(data.clock);
    data.track = v.bearing;
    data.track_available.Update(data.clock);
    data.gps_altitude = r.gps_altitude;
    data.gps_altitude_available.Update(data.clock);
    data.ProvidePressureAltitude(r.baro_altitude);
    data.ProvideBaroAltitudeTrue(r.baro_altitude);

    {
      ScopeLock protect(device_blackboard->mutex);
      device_blackboard->SetReplayState() = data;
      device_blackboard->ScheduleMerge();
    }
  }

  return true;
}

void
Replay::OnTimer()
{
  if (!Update())
    return;

  unsigned schedule;
  if (!positive(time_scale))
    schedule = 1000;
  else if (!negative(fast_forward))
    schedule = 10;
  else if (negative(virtual_time) || !next_data.time_available)
    schedule = 500;
  else if (cli != nullptr)
    schedule = 1000;
  else {
    fixed delta_s = (next_data.time - virtual_time) / time_scale;
    int delta_ms = int(delta_s * 1000);
    schedule = Clamp(delta_ms, 100, 3000);
  }

  Timer::Schedule(schedule);
}
