/*
 * Copyright (C) 2003-2010 Tobias Bieniek <Tobias.Bieniek@gmx.de>
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

#ifndef GESTURE_ZONE_HPP
#define GESTURE_ZONE_HPP

#include "Screen/Canvas.hpp"
#include "Screen/Point.hpp"
#include "Time/PeriodClock.hpp"

#include <tchar.h>

struct GestureLook;

/**
 * A class the defines a valid area of the screen where gestures can be started
 * and also can paint that zone on the screen
 */
class GestureZone
{
public:
  PixelScalar x_zone_width;

  /** timer used to show help at startup */
  PeriodClock clock_since_start;

  /** is this not the first time we're displaying the zone */
  bool draw_initialized;

  /** how many ms after startup do we display the gesture help? */
  unsigned help_duration;

  const GestureLook &gesture_look;

  /**
   * is the zone available, or is it not for the current system
   */
  bool available;

  GestureZone();

  /**
   * check if initialized, and initialized if not
   */
  void CheckInitialize();

  /**
   * draws outline of the gesture zone and calls CheckInitialize()
   * @param rc rc of map minus any overlay areas on top or bottom
   * @param terrain_enabled is the terrain being displayed
   */
  virtual void DrawZone(Canvas &canvas, PixelRect rc, bool terrain_enabled);

  /**
   * sets x_zone_width based on the map size
   */
  virtual void SetZoneWidth(PixelRect rc_map);

  /**
   * return true if p is in the gesture zone
   */
  virtual bool InZone(PixelRect rc_map, RasterPoint p);

  /**
   * restarts gesture help animation so it's drawn when zone is drawn
   */
  virtual void RestartZoneHelp();

  /**
   * Stops showing gesture help when the zone is drawn
   */
  virtual void ClearZoneHelp();

  /**
   * is the help animation currently being displayed
   */
  bool IsHelpVisible();

protected:
  /**
   * draws zone help and calls CheckInitialize()
   */
  virtual void DrawZoneHelp(Canvas &canvas, PixelRect rc);

  /**
   * returns the rect that outlines where the zone borders are drawn
   */
  PixelRect GetZoneRect(PixelRect rc_map);

};
#endif
