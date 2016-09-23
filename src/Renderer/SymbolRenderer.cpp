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

#include "SymbolRenderer.hpp"
#include "Screen/Canvas.hpp"

#include <algorithm>

void
SymbolRenderer::DrawArrow(Canvas &canvas, PixelRect rc, Direction direction,
                          bool no_margins)
{
  assert(direction == UP || direction == DOWN ||
         direction == LEFT || direction == RIGHT);

  PixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / (no_margins ? 2 : 5);
  RasterPoint center = rc.GetCenter();
  RasterPoint arrow[3];

  if (direction == LEFT || direction == RIGHT) {
    arrow[0].x = center.x + (direction == LEFT ? size : -size);
    arrow[0].y = center.y + size;
    arrow[1].x = center.x + (direction == LEFT ? -size : size);
    arrow[1].y = center.y;
    arrow[2].x = center.x + (direction == LEFT ? size : -size);
    arrow[2].y = center.y - size;
  } else if (direction == UP || direction == DOWN) {
    arrow[0].x = center.x + size;
    arrow[0].y = center.y + (direction == UP ? size : -size);
    arrow[1].x = center.x;
    arrow[1].y = center.y + (direction == UP ? -size : size);
    arrow[2].x = center.x - size;
    arrow[2].y = center.y + (direction == UP ? size : -size);
  }

  canvas.DrawTriangleFan(arrow, 3);
}

void
SymbolRenderer::DrawDoubleArrow(Canvas &canvas, PixelRect rc, Direction direction,
                                bool no_margins)
{
  assert(direction == LEFT || direction == RIGHT);

  PixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / (no_margins ? 2 : 5);
  PixelScalar arrow_size = (size * 5) / 9;
  RasterPoint center = rc.GetCenter();
  RasterPoint arrow1[3], arrow2[3];

  arrow1[0].x = center.x + (direction == LEFT ? arrow_size * 2: -arrow_size * 2);
  arrow1[0].y = center.y + arrow_size;
  arrow1[1].x = center.x;
  arrow1[1].y = center.y;
  arrow1[2].x = arrow1[0].x;
  arrow1[2].y = center.y - arrow_size;
  for (unsigned i = 0; i < 3; ++i) {
    arrow2[i].x = arrow1[i].x + (direction == LEFT ? -2 * arrow_size : 2 * arrow_size);
    arrow2[i].y = arrow1[i].y;
  }

  canvas.DrawTriangleFan(arrow1, 3);
  canvas.DrawTriangleFan(arrow2, 3);
}

void
SymbolRenderer::DrawPause(Canvas &canvas, PixelRect rc)
{
  PixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / 5;
  RasterPoint center = rc.GetCenter();

  // Draw vertical bars
  canvas.Rectangle(center.x - size, center.y - size,
                   center.x - size / 3, center.y + size);

  canvas.Rectangle(center.x + size / 3, center.y - size,
                   center.x + size, center.y + size);
}

void
SymbolRenderer::DrawStop(Canvas &canvas, PixelRect rc)
{
  PixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / 5;
  RasterPoint center = rc.GetCenter();

  // Draw square
  canvas.Rectangle(center.x - size, center.y - size,
                   center.x + size, center.y + size);
}

void
SymbolRenderer::DrawSign(Canvas &canvas, PixelRect rc, bool plus)
{
  PixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / 5;
  RasterPoint center = rc.GetCenter();

  // Draw horizontal bar
  canvas.Rectangle(center.x - size, center.y - size / 3,
                   center.x + size, center.y + size / 3);

  if (plus)
    // Draw vertical bar
    canvas.Rectangle(center.x - size / 3, center.y - size,
                     center.x + size / 3, center.y + size);
}
