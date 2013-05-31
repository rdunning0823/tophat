/*
Copyright_License {

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

#ifndef XCSOAR_PERIOD_CLOCK_HPP
#define XCSOAR_PERIOD_CLOCK_HPP

#include "OS/Clock.hpp"

/**
 * This is a stopwatch which saves the timestamp of an event, and can
 * check whether a specified time span has passed since then.
 */
class PeriodClock {
protected:
  typedef unsigned Stamp;

private:
  Stamp last;

public:
  /**
   * Initializes the object, setting the last time stamp to "0",
   * i.e. a check() will always succeed.  If you do not want this
   * default behaviour, call update() immediately after creating the
   * object.
   */
  constexpr
  PeriodClock():last(0) {}

protected:
  static Stamp get_now() {
    return MonotonicClockMS();
  }

public:
  bool IsDefined() const {
    return last != 0;
  }

  /**
   * Resets the clock.
   */
  void Reset() {
    last = 0;
  }

  /**
   * Returns the number of milliseconds elapsed since the last
   * update().  Returns -1 if update() was never called.
   */
  int Elapsed() const {
    if (last == 0)
      return -1;

    return get_now() - last;
  }

  /**
   * Checks whether the specified duration has passed since the last
   * update.
   *
   * @param duration the duration in milliseconds
   */
  bool Check(unsigned duration) const {
    return get_now() >= last + duration;
  }

  /**
   * Updates the time stamp, setting it to the current clock.
   */
  void Update() {
    last = get_now();
  }

  /**
   * Updates the time stamp, setting it to the current clock plus the
   * specified offset.
   */
  void UpdateWithOffset(int offset) {
    Update();
    last += offset;
  }

  /**
   * Checks whether the specified duration has passed since the last
   * update.  If yes, it updates the time stamp.
   *
   * @param duration the duration in milliseconds
   */
  bool CheckUpdate(unsigned duration) {
    Stamp now = get_now();
    if (now >= last + duration) {
      last = now;
      return true;
    } else
      return false;
  }

  /**
   * Checks whether the specified duration has passed since the last
   * update.  After that, it updates the time stamp.
   *
   * @param duration the duration in milliseconds
   */
  bool CheckAlwaysUpdate(unsigned duration) {
    Stamp now = get_now();
    bool ret = now > last + duration;
    last = now;
    return ret;
  }
};

#endif
