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

#include "TrackLineRenderer.hpp"
#include "Look/MapLook.hpp"
#include "Screen/Canvas.hpp"
#include "Math/Angle.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "MapSettings.hpp"

void
TrackLineRenderer::Draw(Canvas &canvas, const Angle screen_angle,
                        const Angle track_angle, const RasterPoint pos)
{
  const auto sc = (track_angle - screen_angle).SinCos();
  const fixed x = sc.first, y = sc.second;

  RasterPoint end;
  end.x = pos.x + iround(x * fixed_int_constant(400));
  end.y = pos.y - iround(y * fixed_int_constant(400));

  canvas.Select(look.track_line_pen);
  canvas.DrawLine(pos, end);
}

void
TrackLineRenderer::Draw(Canvas &canvas, const Angle screen_angle,
                        const RasterPoint pos, const NMEAInfo &basic,
                        const DerivedInfo &calculated,
                        const MapSettings &settings)
{
  if (settings.display_ground_track == DisplayGroundTrack::OFF ||
      calculated.circling)
    return;

  if (settings.display_ground_track == DisplayGroundTrack::AUTO &&
      (basic.track - calculated.heading).AsDelta().AbsoluteDegrees() < fixed(5))
    return;

  TrackLineRenderer::Draw(canvas, screen_angle, basic.track, pos);
}
