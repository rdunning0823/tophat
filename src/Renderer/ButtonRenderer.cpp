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

#include "ButtonRenderer.hpp"
#include "Screen/Color.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Pen.hpp"
#include "Look/ButtonLook.hpp"

static void
BuildButtonShape(RasterPoint *points, const PixelRect &rc)
{
  unsigned scale = 1; //(rc.bottom - rc.top > 30) ? 4 : 1;

  //left to left top
  points[0].x = rc.left;
  points[0].y = rc.top + 4 * scale;
  points[1].x = rc.left + 1 * scale;
  points[1].y = rc.top + 2 * scale;

  points[2].x = rc.left + 2 * scale;
  points[2].y = rc.top + 1 * scale;
  points[3].x = rc.left + 4 * scale;
  points[3].y = rc.top;

  //top to top right
  points[4].x = rc.right - 4 * scale;
  points[4].y = points[3].y;
  points[5].x = rc.right - 2 * scale;
  points[5].y = points[2].y;

  points[6].x = rc.right - 1 * scale;
  points[6].y = points[1].y;
  points[7].x = rc.right;
  points[7].y = points[0].y;

  //right to right bottom
  points[8].x = points[7].x;
  points[8].y = rc.bottom - 4 * scale;
  points[9].x = points[6].x;
  points[9].y = rc.bottom - 2 * scale;

  points[10].x = points[5].x;
  points[10].y = rc.bottom - 1 * scale;
  points[11].x = points[4].x;
  points[11].y = rc.bottom;

  //bottom to bottom left
  points[12].x = points[3].x;
  points[12].y = points[11].y;
  points[13].x = points[2].x;
  points[13].y = points[10].y;

  points[14].x = points[1].x;
  points[14].y = points[9].y;
  points[15].x = points[0].x;
  points[15].y = points[8].y;
}

void
ButtonRenderer::DrawButton(Canvas &canvas, PixelRect rc, bool focused,
                           bool pressed, bool transparent)
{
  const ButtonLook::StateLook &_look = focused ? look.focused : look.standard;

  canvas.Select(pressed ? _look.light_border_pen : _look.dark_border_pen);
  RasterPoint points[16];
  rc.right -= 1;

   if (!transparent) {
     PixelRect rc_inner = rc;
     GrowRect(rc_inner, -1, -1);
     canvas.DrawFilledRectangle(rc_inner, _look.background_color);
   }

  canvas.SelectHollowBrush();
  BuildButtonShape(points, rc);
  canvas.DrawPolygon(points, 16);

  GrowRect(rc, -1, -1);
  BuildButtonShape(points, rc);
  canvas.DrawPolygon(points, 16);
}

PixelRect
ButtonRenderer::GetDrawingRect(PixelRect rc, bool pressed)
{
  GrowRect(rc, -2, -2);
  if (pressed)
    MoveRect(rc, 1, 1);

  return rc;
}
