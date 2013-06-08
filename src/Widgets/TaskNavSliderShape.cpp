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
#include "Look/IconLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"

#include "Interface.hpp"
#include "Components.hpp"
#include "UIGlobals.hpp"

#include "Task/Ordered/OrderedTask.hpp"
#include "Terrain/TerrainSettings.hpp"
#include "Screen/Color.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/UserUnits.hpp"

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
  return *dialog_look.text_font;
}

const Font &
SliderShape::GetMediumFont()
{
  return *dialog_look.caption.font;
}

void
SliderShape::DrawOutline(Canvas &canvas, const PixelRect &rc, unsigned width)
{
  UPixelScalar x_offset = rc.left;
  UPixelScalar y_offset =  0;
  RasterPoint poly[8];
  for (unsigned i=0; i < 8; i++) {
    poly[i].x = GetPoint(i).x + x_offset;
    poly[i].y = GetPoint(i).y + y_offset;
  }

  canvas.Select(Pen(width, COLOR_BLACK));
  DrawClippedPolygon(canvas, poly, 8, rc);
}

#ifdef _WIN32
void
SliderShape::PaintBackground(Canvas &canvas, unsigned idx,
                             unsigned list_length,
                             const DialogLook &dialog_look,
                             const PixelRect rc_outer)
{
  // clear area b/c Win32 does not draw background transparently
  UPixelScalar x_offset = rc_outer.left;
  if (idx == 0) {
    RasterPoint left_mid = GetPoint(7);
    canvas.DrawFilledRectangle(0, 0, x_offset + left_mid.x, rc_outer.bottom,
        Brush(dialog_look.list.GetBackgroundColor(
                false, true, false)));
  }
  if (idx == (list_length - 1)) {
    RasterPoint right_mid = GetPoint(3);
    canvas.DrawFilledRectangle(x_offset + right_mid.x + 1, 0,
        x_offset + right_mid.x + GetHintWidth() + 1,
        rc_outer.bottom,
        Brush(dialog_look.list.GetBackgroundColor(
                false, true, false)));
  }
}
#endif

void
SliderShape::DrawText(Canvas &canvas, const PixelRect rc_outer,
                      unsigned idx, bool selected, const char *tp_name,
                      bool has_entered, bool has_exited,
                      TaskType task_mode, unsigned task_size,
                      bool tp_valid, fixed tp_distance, bool distance_valid,
                      fixed tp_altitude_difference,
                      bool altitude_difference_valid)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const IconLook &icon_look = UIGlobals::GetIconLook();
  const MapSettings &settings_map = CommonInterface::GetMapSettings();
  const TerrainRendererSettings &terrain = settings_map.terrain;

  bool draw_checkmark = (task_mode == TaskType::ORDERED)
      && (task_size > 1)
      && ((idx > 0 && has_entered) || (idx == 0 && has_exited));

  StaticString<120> buffer;
  const Font &name_font = GetLargeFont();
  const Font &small_font = GetSmallFont();
  const Font &medium_font = GetMediumFont();
  UPixelScalar width;
  PixelRect rc = rc_outer;
  rc.left += GetHintWidth();
  rc.right -= GetHintWidth();
  unsigned border_width = Layout::ScalePenWidth(terrain.enable ? 1 : 2);

  if (!tp_valid) {
    canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
    canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
      selected, true, false)));
    DrawOutline(canvas, rc_outer, border_width);
    canvas.Select(small_font);
    buffer = _("Click to navigate");
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.left + (rc.right - rc.left - width) / 2,
                    rc.top + (rc.bottom - rc.top - small_font.GetHeight()) / 2,
                    buffer.c_str());
#ifdef _WIN32
    PaintBackground(canvas, idx, task_size, dialog_look, rc_outer);
#endif
    return;
  }

  canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
  canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
    selected, true, false)));
  DrawOutline(canvas, rc_outer, border_width);
#ifdef _WIN32
    PaintBackground(canvas, idx, task_size, dialog_look, rc_outer);
#endif

  const unsigned line_one_y_offset = rc.top + GetLine1Y();
  const unsigned line_two_y_offset = rc.top + GetLine2Y();

  // Draw turnpoint name
  canvas.Select(name_font);
  PixelSize bitmap_size {0, 0};
  UPixelScalar left_bitmap;
  const Bitmap *bmp = &icon_look.hBmpCheckMark;
  if (draw_checkmark) {
    bitmap_size = bmp->GetSize();
  }
  width = canvas.CalcTextWidth(tp_name) + bitmap_size.cx / 2;
  if (width > (rc_outer.right - rc_outer.left)) {
    canvas.DrawClippedText(rc_outer.left + bitmap_size.cx / 2,
                           line_two_y_offset,
                           rc_outer.right - rc_outer.left - bitmap_size.cx / 2,
                           tp_name);
    left_bitmap = rc_outer.left;
  } else {
    left_bitmap = rc_outer.left + (rc_outer.right - rc_outer.left - width) / 2;
    canvas.DrawText(left_bitmap + bitmap_size.cx / 2,
                    line_two_y_offset, tp_name);
  }

  // draw checkmark next to name if oz entered
  if (draw_checkmark) {
    const int offsety = line_two_y_offset +
        (rc.bottom - line_two_y_offset - bitmap_size.cy) / 2;
    canvas.CopyAnd(left_bitmap,
                    rc.top + offsety,
                    bitmap_size.cx / 2,
                    bitmap_size.cy,
                    *bmp,
                    bitmap_size.cx / 2, 0);
  }

  UPixelScalar distance_width = 0u;
  UPixelScalar label_width = 0u;
  UPixelScalar height_width = 0u;
  StaticString<100> distance_buffer(_T(""));
  StaticString<100> height_buffer(_T(""));

  // Draw distance to turnpoint
  if (distance_valid) {

    canvas.Select(medium_font);
    FormatUserDistanceSmart(tp_distance, distance_buffer.buffer(), true);
    distance_width = canvas.CalcTextWidth(distance_buffer.c_str());
    canvas.DrawText(rc.right - Layout::FastScale(2) - distance_width,
                    line_one_y_offset, distance_buffer.c_str());
  }

  // calculate but don't yet draw label "goto" abort, tp#
  switch (task_mode) {
  case TaskType::ORDERED:
    if (task_size == 0)
      buffer = _("Go'n home:");

    else if (idx == 0)
      buffer = _("Start");
    else if (idx + 1 == task_size)
        buffer = _("Finish");
    else
      _stprintf(buffer.buffer(), _T("TP %u"), idx);

    break;
  case TaskType::GOTO:
  case TaskType::ABORT:
    buffer = _("Goto:");
    break;

  case TaskType::NONE:
    buffer = _("Go'n home:");

    break;
  }
  label_width = canvas.CalcTextWidth(buffer.c_str());
  canvas.Select(small_font);

  // draw arrival altitude centered between label and distance.
  // draw label if room
  canvas.Select(small_font);
  if (altitude_difference_valid) {
    FormatRelativeUserAltitude(tp_altitude_difference, height_buffer.buffer(),
                               true);
    height_width = canvas.CalcTextWidth(height_buffer.c_str());
    width = distance_width + height_width;
    UPixelScalar offset = rc.left;
    if ((PixelScalar)width < (rc.right - rc.left - label_width -
        Layout::FastScale(15))) {
      canvas.DrawText(rc.left + Layout::FastScale(2),
                      line_one_y_offset, buffer.c_str());
      offset = rc.left + label_width +
          (rc.right - rc.left - width - label_width) / 2;
    }
    canvas.DrawText(offset, line_one_y_offset, height_buffer.c_str());
  }

#ifdef NOT_DEFINED_EVER
  // bearing delta waypoint for ordered when not start
  // or for non ordered task
  // TODO make this configurable to show delta or true bearing
  bool do_bearing = false;
  Angle bearing;
  if (tp.bearing_valid && task_mode ==
      TaskType::ORDERED && idx > 0) {
    do_bearing = true;
    bearing = tp.delta_bearing;
  } else if (task_mode != TaskType::ORDERED &&
      tp.bearing_valid) {
    do_bearing = true;
    bearing = tp.delta_bearing;
  }

  if (false && do_bearing) {
    FormatAngleDelta(buffer.buffer(), buffer.MAX_SIZE, bearing);
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.Select(small_font);
    canvas.text((rc.left + rc.right - width) / 2,
                line_one_y_offset, buffer.c_str());
  }
#endif

}

void
SliderShape::Resize(UPixelScalar map_width)
{
  const UPixelScalar large_font_height = GetLargeFont().GetHeight();
  const UPixelScalar medium_font_height = GetMediumFont().GetHeight();

  const UPixelScalar total_height = large_font_height
      + medium_font_height - Layout::Scale(2);
  const UPixelScalar arrow_point_bluntness = Layout::Scale(4);

  SetLine1Y(0u);
  SetLine2Y(total_height - large_font_height);
  SetLine3Y(0u);

  //top
  points[0].x = Layout::Scale(20);
  points[0].y = 1;
  points[1].x = Layout::Scale(340);
  points[1].y = 1;

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
