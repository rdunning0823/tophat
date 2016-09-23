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

#include "MapWindow.hpp"
#include "Look/MapLook.hpp"
#include "Renderer/BestCruiseArrowRenderer.hpp"
#include "Renderer/CompassRenderer.hpp"
#include "Renderer/TrackLineRenderer.hpp"
#include "Renderer/WindArrowRenderer.hpp"

void
MapWindow::DrawWind(Canvas &canvas, const RasterPoint &Start,
                    const PixelRect &rc) const
{
  if (IsPanning())
    return;

  WindArrowRenderer wind_arrow_renderer(look.wind);
  switch (GetMapSettings().wind_arrow_location) {
  case WindArrowLocation::MAP_AND_INFOBOX:
  case WindArrowLocation::MAP_ONLY:
    wind_arrow_renderer.Draw(canvas, render_projection.GetScreenAngle(),
                             Start, rc, Calculated(), GetMapSettings());
    break;
  case WindArrowLocation::INFOBOX_ONLY:
  case WindArrowLocation::NOWHERE:
    break;
  }
}

void
MapWindow::DrawCompass(Canvas &canvas, const PixelRect &rc) const
{
  if (!compass_visible)
    return;

  PixelRect rc_compass = rc;
  rc_compass.top += nav_slider_bar_visible_height;
  CompassRenderer compass_renderer(look);
  compass_renderer.Draw(canvas, render_projection.GetScreenAngle(), rc_compass);
}

void
MapWindow::DrawBestCruiseTrack(Canvas &canvas, const RasterPoint aircraft_pos) const
{
  if (Basic().location_available)
    BestCruiseArrowRenderer::Draw(canvas, look.task,
                                  render_projection.GetScreenAngle(),
                                  aircraft_pos, Calculated());
}

void
MapWindow::DrawTrackBearing(Canvas &canvas, const RasterPoint aircraft_pos, bool circling) const
{
  if (!Basic().location_available)
    return;

  bool wind_relative = GetMapSettings().trail.wind_drift_enabled && circling;

  TrackLineRenderer track_line_renderer(look);
  track_line_renderer.Draw(canvas, render_projection,
                           aircraft_pos, Basic(), Calculated(), GetMapSettings(),
                           wind_relative);
}
