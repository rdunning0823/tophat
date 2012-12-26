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

#include "KineticManager.hpp"

void
KineticManager::Reverse()
{
  MoveTo(mouse_down_x);
  mode = REVERSED;
}

void
KineticManager::MoveTo(int x)
{
  stopping_time = default_stopping_time / 2;
  last = GetPosition();
  steady = false;
  mode = MOVETO;
  fixed distance = fixed(x - last);
  clock.Update();
  v = (distance * fixed(2)) / fixed(stopping_time);
  end = x;
}

bool
KineticManager::IsReversed()
{
  return mode == REVERSED;
}

bool
KineticManager::IsMoveTo()
{
  return mode == MOVETO;
}

void
KineticManager::MouseDown(int x)
{
  mode = NORMAL;
  mouse_down_x = last = x;
  stopping_time = default_stopping_time;
  clock.Update();
  v = fixed(0);
}

void
KineticManager::MouseMove(int x)
{
  steady = false;
  // Get time since last position update
  int dt = clock.Elapsed();

  // Filter fast updates to get a better velocity
  if (dt < 15)
    return;

  // Update clock for next event
  clock.Update();

  // Calculate value delta
  int dx = x - last;

  // Calculate value-based velocity
  v = fixed(dx) / dt;

  // Save value for next event
  last = x;
}

void
KineticManager::MouseUp(int x, unsigned max_distance_mouse_dn)
{
  int dt = clock.Elapsed();
  if (dt > 200) {
    end = x;
    steady = true;
    return;
  }

  // Calculate distance of the kinetic movement
  fixed distance_basic = (v / fixed(2)) * fixed(stopping_time);
  int dist_no_max = last - mouse_down_x + (int)distance_basic;

  fixed distance_final = distance_basic;

  // if a max_distance exists, then adjust end and v if needed
  if (max_distance_mouse_dn > 0 &&
      abs(dist_no_max) > (int)max_distance_mouse_dn) {
    distance_final = (fixed)(max_distance_mouse_dn *
        ((distance_basic > fixed(0)) ? fixed(1) : fixed(-1)) -
        fixed(last) + fixed(mouse_down_x));
    v = (distance_final * fixed(2)) / fixed(stopping_time);
  }
  end = last + (int)distance_final;
}

int
KineticManager::GetPosition()
{
  // Get time that has passed since the end of the manual movement
  int t = clock.Elapsed();
  // If more time has passed than allocated for the kinetic movement
  if (t >= stopping_time) {
    // Stop the kinetic movement and return the precalculated end position
    steady = true;
    return end;
  }

  // Calculate the current position of the kinetic movement
  int x = last + (int)(v * t - v * t * t / (2 * stopping_time));
  if (x == end)
    steady = true;
  return x;
}

bool
KineticManager::IsSteady()
{
  return steady;
}
