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

#include "BestCruiseArrowRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Look/TaskLook.hpp"
#include "Math/Angle.hpp"
#include "Math/Screen.hpp"
#include "NMEA/Derived.hpp"
#include "Util/Macros.hpp"

void
BestCruiseArrowRenderer::Draw(Canvas &canvas, const TaskLook &look,
                              const Angle screen_angle,
                              const Angle best_cruise_angle,
                              const RasterPoint pos)
{
  canvas.Select(look.best_cruise_track_pen);
  canvas.Select(look.best_cruise_track_brush);

  RasterPoint arrow[] = { { -1, -40 }, { -1, -62 }, { -6, -62 }, {  0, -70 },
                          {  6, -62 }, {  1, -62 }, {  1, -40 }, { -1, -40 } };

  PolygonRotateShift(arrow, ARRAY_SIZE(arrow), pos,
                     best_cruise_angle - screen_angle);
  canvas.DrawPolygon(arrow, ARRAY_SIZE(arrow));
}

void
BestCruiseArrowRenderer::DrawInfoBox(Canvas &canvas,
                                     const TaskLook &look,
                                     const Angle screen_angle,
                                     const Angle best_cruise_angle,
                                     const RasterPoint pos)
{
  canvas.Select(look.best_cruise_track_pen);
  canvas.Select(look.best_cruise_track_brush);

  RasterPoint arrow[] = { { -1, 12 }, { -1, -4 }, { -6, -4 }, {  0, -12 },
                          {  6, -4 }, {  1, -4 }, {  1, 12 }, { -1, 12 } };

  PolygonRotateShift(arrow, ARRAY_SIZE(arrow), pos,
                     best_cruise_angle - screen_angle);
  canvas.DrawPolygon(arrow, ARRAY_SIZE(arrow));
}

void
BestCruiseArrowRenderer::Draw(Canvas &canvas, const TaskLook &look,
                              const Angle screen_angle, const RasterPoint pos,
                              const DerivedInfo &calculated,
                              bool draw_infobox)
{
  if (calculated.turn_mode == CirclingMode::CLIMB ||
      !calculated.task_stats.task_valid)
    return;

  const GlideResult &solution =
      calculated.task_stats.current_leg.solution_remaining;

  if (!solution.IsOk() ||
      solution.vector.distance < fixed(0.010))
    return;

  if (draw_infobox)
    BestCruiseArrowRenderer::DrawInfoBox(canvas, look, screen_angle,
                                         solution.cruise_track_bearing, pos);
  else
    BestCruiseArrowRenderer::Draw(canvas, look, screen_angle,
                                  solution.cruise_track_bearing, pos);
}
