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

#include "GestureZone.hpp"
#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Look/GestureLook.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Renderer/TextInBox.hpp"
#include "Hardware/DisplayDPI.hpp"

#include <algorithm>

GestureZone::GestureZone()
  :x_zone_width(Display::GetXDPI() / 2), draw_initialized(false),
   help_duration(15000), gesture_look(UIGlobals::GetLook().gesture),
   available(false){}

void
GestureZone::SetZoneWidth(PixelRect rc_map)
{
  unsigned screen_width = rc_map.GetSize().cx;
  /**
   * use 1/2 inch min up to 1 inch, but not more than 1/4 the width of the screen
   */
  x_zone_width = std::max(Display::GetXDPI() / 2,
                          std::min(Display::GetXDPI(), screen_width / 4));
}

bool
GestureZone::InZone(PixelRect map_rc, RasterPoint p)
{
  if (!available)
    return false;

  PixelRect rc = GetZoneRect(map_rc);
  rc.top = map_rc.top;
  rc.bottom = map_rc.bottom;
  return !rc.IsInside(p);
}

void
GestureZone::CheckInitialize()
{
  if (!draw_initialized)
    clock_since_start.Update();
  draw_initialized = true;
}

void
GestureZone::RestartZoneHelp()
{
  clock_since_start.Update();
}

void
GestureZone::ClearZoneHelp()
{
  clock_since_start.UpdateWithOffset(-help_duration);
}

PixelRect
GestureZone::GetZoneRect(PixelRect rc_map)
{
  PixelRect rc = rc_map;
  bool landscape = rc.GetSize().cx > rc.GetSize().cy;

  rc.Grow(-1 * x_zone_width, -1 * x_zone_width);
  if (!landscape)
    rc.bottom -= x_zone_width / 2;

  return rc;
}

void
GestureZone::DrawZoneHelp(Canvas &canvas, PixelRect rc)
{
  CheckInitialize();

  canvas.SetBackgroundOpaque();
  canvas.SetBackgroundColor(true ? COLOR_WHITE : COLOR_BLACK);
  canvas.SetTextColor(true ? COLOR_RED : COLOR_WHITE);
  canvas.Select(*UIGlobals::GetDialogLook().caption.font);

  unsigned y_middle = rc.top + rc.GetSize().cy / 2;
  int arrow_tip = x_zone_width / 2;
  const TCHAR *message1 = _("Swipe from edge");
  const TCHAR *message2 = _("to switch screens");

  const PixelSize ts1 = canvas.CalcTextSize(message1);
  const PixelSize ts2 = canvas.CalcTextSize(message2);
  int y_arrow = y_middle - arrow_tip * 0.7;

  TextInBoxMode style;
  style.shape = LabelShape::FILLED;
  style.align = TextInBoxMode::Alignment::LEFT;
  style.opaque = true;

  const unsigned mod = 12;
  unsigned length_ratio = (clock_since_start.Elapsed() / 250) % mod;
  unsigned length_max = x_zone_width * 2;
  unsigned length = (length_max * length_ratio) / mod ;


  const Bitmap *bmp_hand = &gesture_look.hBmpHandPointer;
  PixelSize bmp_hand_size = bmp_hand->GetSize();

  TextInBox(canvas, message1, rc.left + (rc.GetSize().cx - ts1.cx) / 2,
            y_arrow - 2 * ts1.cy - Layout::GetTextPadding(), style, rc);
  TextInBox(canvas, message2, rc.left + (rc.GetSize().cx - ts2.cx) / 2,
            y_arrow - ts2.cy - Layout::GetTextPadding(), style, rc);

  // moving to right
  int x_tip = rc.left - x_zone_width / 2 + length;
  int x_base = rc.left - x_zone_width / 2;

  canvas.DrawLine( {rc.left - 1, rc.top },
                   {rc.left - 1, rc.bottom });

  canvas.Select(gesture_look.pen);
  canvas.DrawLine( {x_base, y_arrow },
                   {x_tip, y_arrow });
  // changing CopyOr to CopyNotOr for 6.7.2 merge where UNIX does not have this method
  // changed to Copy with 6.8 merge
  canvas.Copy(x_tip - 0.2 * bmp_hand_size.cx, y_arrow,
              bmp_hand_size.cx / 2, bmp_hand_size.cy,
              *bmp_hand,
              0, 0);
  canvas.Copy(x_tip - 0.2 * bmp_hand_size.cx, y_arrow,
              bmp_hand_size.cx / 2, bmp_hand_size.cy,
              *bmp_hand,
              bmp_hand_size.cx / 2, 0);
}

bool
GestureZone::IsHelpVisible()
{
  CheckInitialize();
  return !clock_since_start.Check(help_duration);
}

void
GestureZone::DrawZone(Canvas &canvas, PixelRect rc_map, bool terrain_enabled)
{
  CheckInitialize();

  if (!available)
    return;

  SetZoneWidth(rc_map);
  PixelRect rc = GetZoneRect(rc_map);

  bool show_help = IsHelpVisible();

  canvas.Select(false ? gesture_look.pen :
      gesture_look.zone_pen);

  canvas.DrawLine( {rc.left, rc.top },
                   {rc.left, rc.bottom });

  canvas.DrawLine( {rc.right, rc.top },
                   {rc.right, rc.bottom });

  if (show_help)
    DrawZoneHelp(canvas, rc);
}
