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

GestureZone::GestureZone()
  :x_zone_width(Layout::GetXDPI() / 2) {}

bool
GestureZone::InZone(PixelRect map_rc, RasterPoint p)
{
  if (p.x < x_zone_width
      || p.x > (PixelScalar)map_rc.GetSize().cx - x_zone_width)
    return true;

  return false;
}

void
GestureZone::DrawZone(Canvas &canvas, PixelRect rc, bool terrain_enabled)
{
  canvas.Select(terrain_enabled ? UIGlobals::GetLook().gesture.zone_pen_thick :
      UIGlobals::GetLook().gesture.zone_pen);

  canvas.DrawLine( {x_zone_width, x_zone_width },
                   {x_zone_width, rc.GetSize().cy - x_zone_width });

  canvas.DrawLine( {rc.GetSize().cx - x_zone_width, x_zone_width },
                   {rc.GetSize().cx - x_zone_width,
                    rc.GetSize().cy - x_zone_width });


}
