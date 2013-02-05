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

#include "Widgets/TaskNavSliderShape.hpp"

#include "Look/DialogLook.hpp"
#include "Look/InfoBoxLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"


static
void DrawClippedPolygon(Canvas &canvas, const RasterPoint* points,
                        unsigned size, const PixelRect rc)
{
  RasterPoint points_clipped[8];
  assert(size <= 8);

  for (unsigned i = 0; i < size; i++) {
    if (points[i].x < rc.left)
      points_clipped[i].x = rc.left;
    else if (points[i].x > rc.right)
      points_clipped[i].x = rc.right;
    else points_clipped[i].x = points[i].x;

    if (points[i].y < rc.top)
      points_clipped[i].y = rc.top;
    else if (points[i].y > rc.bottom)
      points_clipped[i].y = rc.bottom;
    else points_clipped[i].y = points[i].y;
  }
  canvas.DrawPolygon(points_clipped, size);
}

const Font &
SliderShape::GetLargeFont()
{
  return *infobox_look.value.font;
}

const Font &
SliderShape::GetSmallFont()
{
  return *dialog_look.list.font;
}

void
SliderShape::Draw(Canvas &canvas, const PixelRect &rc)
{
  UPixelScalar x_offset = rc.left;
  UPixelScalar y_offset =  0;
  RasterPoint poly[8];
  for (unsigned i=0; i < 8; i++) {
    poly[i].x = GetPoint(i).x + x_offset;
    poly[i].y = GetPoint(i).y + y_offset;
  }

  canvas.Select(Pen(2, COLOR_BLACK));
  DrawClippedPolygon(canvas, poly, 8, rc);
}

void
SliderShape::Resize(UPixelScalar map_width)
{
  const UPixelScalar large_font_height = GetLargeFont().GetHeight();
  const UPixelScalar small_font_height = GetSmallFont().GetHeight();

  const UPixelScalar total_height = large_font_height + 2 * small_font_height
      - Layout::Scale(3);
  const UPixelScalar arrow_point_bluntness = Layout::Scale(4);


  SetLine1Y(0u);
  SetLine2Y((total_height - large_font_height) / 2);
  SetLine3Y(total_height - small_font_height);

  //top
  points[0].x = Layout::Scale(20);
  points[0].y = 0;
  points[1].x = Layout::Scale(340);
  points[1].y = 0;

  //right arrow tip
  points[2].x = Layout::Scale(360);
  points[2].y = (total_height - arrow_point_bluntness) / 2;
  points[3].x = Layout::Scale(360);
  points[3].y = (total_height + arrow_point_bluntness) / 2;

  //bottom
  points[4].x = points[1].x;
  points[4].y = total_height;
  points[5].x = points[0].x;
  points[5].y = total_height;

  //left arrow tip
  points[6].x = 0;
  points[6].y = points[3].y;
  points[7].x = 0;
  points[7].y = points[2].y;

  const UPixelScalar width_original = GetWidth();

  PixelScalar amount_to_grow_x = map_width
      - (GetHintWidth() * 2) - width_original - 1;
  PixelScalar neg_min_grow = points[5].x - points[4].x;
  amount_to_grow_x = (amount_to_grow_x < neg_min_grow) ? neg_min_grow :
      amount_to_grow_x;

  for (unsigned i = 1; i <= 4; i++)
    points[i].x += amount_to_grow_x;

}
