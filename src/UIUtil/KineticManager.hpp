/*
 * Copyright (C) 2011 Tobias Bieniek <Tobias.Bieniek@gmx.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KINETIC_MANAGER_HPP
#define KINETIC_MANAGER_HPP

#include "Time/PeriodClock.hpp"
#include "Math/fixed.hpp"
#include <stdlib.h> /* for abs() */

/**
 * A manager class that can be used for kinetic scrolling
 */
class KineticManager
{
public:
  enum Mode {
    /**
     * kenetic scroll until stopped
     */
    NORMAL,
    /**
     * kenetic scroll back to start of original scrolling
     */
    REVERSED,
    /**
     * scroll to predefined location and stop
     */
    MOVETO,
  };

private:
  /** Default time in ms until the kinetic movement is stopped */
  const int default_stopping_time;

  /**
   * Time in ms of the current scolls customzied stopping time
   */
  int stopping_time;

  /** Whether the kinetic movement is still active */
  bool steady;

  /** Position at the end of the manual movement */
  int last;

  /** Precalculated final position of the kinetic movement */
  int end;

  /** Speed at the end of the manual movement */
  fixed v;

  /** location where mouse was down.  Used to calculate stop location */
  int mouse_down_x;

  /** Clock that is used for the kinetic movement */
  PeriodClock clock;

  /** type of movement */
  Mode mode;

public:
  KineticManager(int _stopping_time = 1000)
    :default_stopping_time(_stopping_time), steady(true) {}

  /** Needs to be called once the manual movement is started */
  void MouseDown(int x);
  /** Needs to be called on every manual mouse move event */
  void MouseMove(int x);
  /**
   * Needs to be called at the end of the manual movement and
   * starts the kinetic movement
   */
  void MouseUp(int x) {
    MouseUp(x, 0);
  }
  /**
   * @param max_distance to allow to scroll from mouse_down_x
   * set to 0 to scroll unlimitted
   */
  void MouseUp(int x, unsigned max_distance_mouse_dn);
  /**
   * Returns the current position of the kinetic movement.
   * Sets the steady flag to true if the kinetic motion is
   * not active (@see IsSteady())
   */
  int GetPosition();

  /** Returns whether the kinetic movement is still active */
  bool IsSteady();

  /**
   * Reverses the scroll with the same velocity, the other direction
   * back to the originating location from the current location
   */
  void Reverse();

  /**
   * Moves from the current location to a specific location at a constant speed
   * @parm x is the destination position for the motion to end
   */
  void MoveTo(int x);

  /** returns true if in REVERSED mode */
  bool IsReversed();

  /**
   * returns true if in MOVETO mode
   */
  bool IsMoveTo();

  Mode GetMode() {
    return mode;
  }
  /**
   * returns x position where mouse down occurred
   */
  int GetMouseDownX() {
    return mouse_down_x;
  }

  /**
   * returns absolute velocity based on drag speed, rounded to an integer
   */
  unsigned GetVelocity() {
    return abs((int)v);
  }
};

#endif
