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

#ifndef REPLAY_HPP
#define REPLAY_HPP

#include "Event/Timer.hpp"
#include "Math/fixed.hpp"
#include "NMEA/Info.hpp"
#include "Time/PeriodClock.hpp"

#include <tchar.h>
#include <windef.h> /* for MAX_PATH */

class Logger;
class ProtectedTaskManager;
class AbstractReplay;
class CatmullRomInterpolator;

class Replay final
  : private Timer
{
public:
  enum PlayState {
    NOFILE,
    NOTSTARTED,
    PLAYING,
    PAUSED,
    FASTFORWARD,
  };

private:
  PlayState play_state;

  fixed time_scale;

  AbstractReplay *replay;

  Logger *logger;
  ProtectedTaskManager &task_manager;

  TCHAR path[MAX_PATH];

  /**
   * The time of day according to replay input.  This is negative if
   * unknown.
   */
  fixed virtual_time;

  /**
   *  the first time read from the file
   */
  fixed first_virtual_time;

  /**
   * If this value is not negative, then we're in fast-forward mode:
   * replay is going as quickly as possible.  This value denotes the
   * time stamp when we will stop going fast-forward.  If
   * #virtual_time is negative, then this is the duration, and
   * #virtual_time will be added as soon as it is known.
   */
  fixed fast_forward;

  fixed rewind;

  /**
   * Keeps track of the wall-clock time between two Update() calls.
   */
  PeriodClock clock;

  /**
   * The last NMEAInfo returned by the #AbstractReplay instance.  It
   * is held back until #virtual_time has passed #next_data.time.
   */
  NMEAInfo next_data;

  CatmullRomInterpolator *cli;

public:
  Replay(Logger *_logger, ProtectedTaskManager &_task_manager)
    :play_state(PlayState::NOFILE), time_scale(fixed(1)), replay(nullptr),
     logger(_logger), task_manager(_task_manager), cli(nullptr) {
    path[0] = _T('\0');
  }

  ~Replay() {
    Stop();
  }

  void SetPlayState(PlayState state) {
    play_state = state;
  }

  bool IsNoFile() {
    return play_state == PlayState::NOFILE;
  }
  bool IsNotStarted() {
    return play_state == PlayState::NOTSTARTED;
  }

  bool IsPlaying() {
    return play_state == PlayState::PLAYING;
  }

  bool IsPaused() {
    return play_state == PlayState::PAUSED;
  }

  bool IsFastForward() {
    return play_state == PlayState::FASTFORWARD;
  }

  PlayState GetPlayState() {
    return play_state;
  }

  bool IsActive() const {
    return replay != nullptr;
  }

private:
  bool Update();

public:
  void Stop();
  bool Start(const TCHAR *_path);

  const TCHAR *GetFilename() const {
    return path;
  }

  fixed GetTimeScale() const {
    return time_scale;
  }

  void SetTimeScale(const fixed _time_scale) {
    time_scale = _time_scale;
  }

  /**
   * Start fast-forwarding the replay by the specified number of
   * seconds.  This replays the given amount of time from the input
   * time as quickly as possible.
   * @return True if was able to enter fast-forward mode.
   */
  bool FastForward(fixed delta_s) {
    if (IsActive() && !negative(virtual_time)) {
      fast_forward = virtual_time + delta_s;
      return true;
    } else
      return false;
  }

  void FastForwardCancel() {
    fast_forward = fixed(-1);
  }

  bool Rewind(fixed delta_s);

  /**
   *  returns virtual time, or -1 if unknown
   */
  fixed GetTime() {
    return virtual_time;
  }

  bool CheckFastForward() {
    return !negative(fast_forward);
  }

private:
  void OnTimer() override;
};

#endif
