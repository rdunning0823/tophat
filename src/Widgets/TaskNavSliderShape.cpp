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
#include "Asset.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Compatibility.hpp"
#endif

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

SliderShape::VisibilityLevel
SliderShape::GetVisibilityLevel(Canvas &canvas, RasterPoint poly[])
{
  const PixelRect rc = canvas.GetRect();
  RasterPoint left_tip = poly[7];
  RasterPoint left_body = poly[5];
  left_body.x += Layout::Scale(1);

  RasterPoint right_tip = poly[2];
  RasterPoint right_body = poly[1];
  right_body.x -= Layout::Scale(1);

  if (rc.IsInside(left_tip) && rc.IsInside(right_tip))
    return Full;

  if (rc.IsInside(left_tip)) {
    if (rc.IsInside(left_body))
      return LeftTipAndBody;
    else
      return LeftTip;
  } else if (rc.IsInside(right_tip)) {
    if (rc.IsInside(right_body))
      return RightTipAndBody;
    else
      return RightTip;
  } else
    return None;
}

void
SliderShape::DrawBackgroundAll(Canvas &canvas, const RasterPoint poly[])
{
  /* clear background */
  canvas.SelectWhitePen();
  canvas.DrawPolygon(poly, 8);
  if (IsKobo()) {
    const unsigned y = 2;
    canvas.Select(Pen(2, COLOR_WHITE));
    assert(canvas.GetRect().IsInside({poly[0].x, y}));
    assert(canvas.GetRect().IsInside({poly[1].x, y}));
    canvas.DrawLine(poly[0].x, y, poly[1].x, y);
  }
}

void
SliderShape::DrawOutlineAll(Canvas &canvas, const RasterPoint poly[],
                            unsigned width, const Color color)
{
  /* draw with normal width but don't draw top line */
  canvas.Select(Pen(width, color));
  canvas.DrawTwoLines(poly[1], poly[2], poly[3]);
  canvas.DrawTwoLines(poly[3], poly[4], poly[5]);
  canvas.DrawTwoLines(poly[5], poly[6], poly[7]);
  canvas.DrawLine(poly[7].x, poly[7].y, poly[0].x, poly[0].y);
}

bool
SliderShape::DrawOutline(Canvas &canvas, const PixelRect &rc, unsigned width)
{
  PixelRect canvas_rect = canvas.GetRect();

  PixelScalar x_offset = rc.left;
  PixelScalar y_offset =  0;
  RasterPoint poly[8];
  RasterPoint poly_raw[8];

  assert (width > 0);
  /* KOBO dithering centers odd shaped widths within 1/2 pixel,
   * and we need to stay within the canvas or memory gets corrupted
   * The lines have square ends so the diagonal ones actually go
   * a pixel past the end vertically and horizontally */
#ifdef KOBO
  PixelScalar width_offset = 1;
  PixelScalar top_line_offset = 2;
#else
  PixelScalar width_offset = 0;
  PixelScalar top_line_offset = 0;
#endif

  for (unsigned i=0; i < 8; i++) {
    PixelScalar x = GetPoint(i).x + x_offset;
    PixelScalar y = GetPoint(i).y + y_offset;

    poly_raw[i].y = y;
    poly_raw[i].x = x;

    x = std::max(x, PixelScalar(canvas_rect.left + width / 2 + width_offset));
    x = std::min(x, PixelScalar(canvas_rect.right - width / 2 - 1));
    y = std::max(y, PixelScalar(canvas_rect.top + top_line_offset));
    y = std::min(y, PixelScalar(canvas_rect.bottom - width / 2 - 1));

    poly[i].y = y;
    poly[i].x = x;

    assert(canvas_rect.IsInside({x, y}));
  }

  const VisibilityLevel visibility = GetVisibilityLevel(canvas, poly_raw);

  if (visibility == None)
    return false;

#ifdef _WIN32
  canvas.Select(Pen(width, COLOR_BLACK));
  canvas.DrawPolygon(poly, 8);
  return true;
#endif

  switch (visibility) {
  case Full:
  case LeftTipAndBody:
  case RightTipAndBody:
    DrawBackgroundAll(canvas, poly);
    DrawOutlineAll(canvas, poly, width, COLOR_BLACK);
    break;

  /** some or all of the left tip, but no body */
  case LeftTip:
  case RightTip:
    canvas.SelectWhitePen();
    canvas.DrawPolygon(poly, 8); // could this be the problem? repeated poly points?
    canvas.Select(Pen(width, COLOR_BLACK));
    if (visibility == LeftTip)
      canvas.DrawTwoLines(poly[0], poly[6], poly[5]);
    else
      canvas.DrawTwoLines(poly[1], poly[2], poly[4]);
    break;

  /* no part is visible */
  case None:
    gcc_unreachable();
    break;
  }

  return true;
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
        Brush(COLOR_WHITE));
  }
  if (idx == (list_length - 1)) {
    RasterPoint right_mid = GetPoint(3);
    canvas.DrawFilledRectangle(x_offset + right_mid.x, 0,
        x_offset + right_mid.x + GetHintWidth() + 1,
        rc_outer.bottom,
        Brush(COLOR_WHITE));
  }
}
#endif

void
SliderShape::Draw(Canvas &canvas, const PixelRect rc_outer,
                  unsigned idx, bool selected, bool is_current_tp,
                  const TCHAR *tp_name, bool has_entered, bool has_exited,
                  TaskType task_mode, TaskFactoryType task_factory_type,
                  unsigned task_size,
                  bool tp_valid, fixed tp_distance, bool distance_valid,
                  fixed tp_altitude_difference,
                  bool altitude_difference_valid,
                  Angle delta_bearing,
                  bool bearing_valid,
                  unsigned border_width)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const IconLook &icon_look = UIGlobals::GetIconLook();

  bool draw_checkmark = (task_mode == TaskType::ORDERED)
      && (task_size > 1)
      && ((idx > 0 && has_entered) || (idx == 0 && has_exited));

  StaticString<120> buffer;
  const Font &name_font = GetLargeFont();
  const Font &small_font = GetSmallFont();
  const Font &medium_font = GetMediumFont();
  UPixelScalar width;
  PixelScalar left;
  PixelRect rc = rc_outer;
  rc.left += 3 * GetHintWidth() / 2;
  rc.right -= 3 * GetHintWidth() / 2;

  if (!tp_valid) {
    canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
    canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
      selected, true, false)));
    DrawOutline(canvas, rc_outer, border_width);
    canvas.Select(small_font);
    buffer = _("Click to navigate");
    width = canvas.CalcTextWidth(buffer.c_str());
    left = rc.left + (rc.right - rc.left - width) / 2;
    if (left > 0)
      canvas.TextAutoClipped(left,
                             rc.top + (rc.bottom - rc.top -
                                 small_font.GetHeight()) / 2,
                             buffer.c_str());
#ifdef _WIN32
    if (HasDraggableScreen()) // PC or WM
      PaintBackground(canvas, idx, 1, dialog_look, rc_outer);
#endif
    return;
  }

  canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
  canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
    selected, true, false)));
  if (!DrawOutline(canvas, rc_outer, border_width))
    return;

#ifdef _WIN32
  if (HasDraggableScreen()) // PC or WM
    PaintBackground(canvas, idx,
                    (task_mode == TaskType::GOTO) ? 1 : task_size,
                    dialog_look, rc_outer);
#endif

  const unsigned line_one_y_offset = rc.top + GetLine1Y();
  const unsigned line_two_y_offset = rc.top + GetLine2Y();

  UPixelScalar distance_width = 0u;
  UPixelScalar label_width = 0u;
  UPixelScalar height_width = 0u;
  StaticString<100> distance_buffer(_T(""));
  StaticString<100> height_buffer(_T(""));

  // calculate but don't yet draw label "goto" abort, tp#
  switch (task_mode) {
  case TaskType::ORDERED:
    if (task_size == 0)
      buffer = _("Go'n home:");

    else if (idx == 0)
      buffer = _("Start");
    else if (idx + 1 == task_size)
        buffer = _("Finish");
    else if (task_factory_type ==  TaskFactoryType::AAT)
      _stprintf(buffer.buffer(), _T("%s %u"), _("Center"), idx);
    else
      _stprintf(buffer.buffer(), _T("%s %u"), _("TP"), idx);

    break;
  case TaskType::GOTO:
  case TaskType::ABORT:
    buffer = _("Goto:");
    break;

  case TaskType::NONE:
    buffer = _("Go'n home:");

    break;
  }
  canvas.Select(small_font);
  label_width = canvas.CalcTextWidth(buffer.c_str());

  // Draw arrival altitude right upper corner
  if (altitude_difference_valid) {

    canvas.Select(small_font);
    FormatRelativeUserAltitude(tp_altitude_difference, height_buffer.buffer(),
                               true);
    height_width = canvas.CalcTextWidth(height_buffer.c_str());
    left = rc.right - height_width;
    if (left > 0)
      canvas.TextAutoClipped(left, line_one_y_offset, height_buffer.c_str());
  }

  // bearing chevrons for ordered when not start
  // or for non ordered task
  int bearing_direction = 0; // directiong of bearing if drawn
  bool do_bearing = false;
  Angle bearing;
  if (is_current_tp && bearing_valid && task_mode ==
      TaskType::ORDERED && idx > 0) {
    do_bearing = true;
    bearing = delta_bearing;
  } else if (task_mode != TaskType::ORDERED &&
      bearing_valid) {
    do_bearing = true;
    bearing = delta_bearing;
  }

  // draw distance centered between label and altitude.
  // draw label if room
  if (distance_valid) {
    FormatUserDistance(tp_distance, distance_buffer.buffer(), true, 1);
    canvas.Select(medium_font);
    distance_width = canvas.CalcTextWidth(distance_buffer.c_str());

    UPixelScalar offset = rc.left;
    if ((PixelScalar)(distance_width + height_width) <
        (PixelScalar)(rc.right - rc.left - label_width -
            Layout::FastScale(15))) {
      canvas.Select(small_font);
      left = rc.left;
      if (left > 0)
        canvas.TextAutoClipped(left, line_one_y_offset, buffer.c_str());
      offset = rc.left + label_width +
          (rc.right - rc.left - distance_width - height_width
              - label_width) / 2;

    }

    canvas.Select(medium_font);
    left = offset;
    if (left > 0)
      canvas.TextAutoClipped(left, line_one_y_offset, distance_buffer.c_str());

    if (do_bearing)
      bearing_direction = DrawBearing(canvas, rc_outer,bearing);

  }

  // Draw tp name, truncated to leave space before rt. bearing if drawn
  canvas.Select(name_font);
  PixelSize bitmap_size {0, 0};
  UPixelScalar left_bitmap;
  const Bitmap *bmp = &icon_look.hBmpCheckMark;
  if (draw_checkmark) {
    bitmap_size = bmp->GetSize();
#ifdef ENABLE_OPENGL
    /* this seams to be a is this a bug? */
    bitmap_size.cx *= 2;
#endif
  }
  PixelRect rc_name(rc_outer.left + GetHintWidth(), rc_outer.top,
                    rc_outer.right - GetHintWidth(), rc_outer.bottom);

  width = canvas.CalcTextWidth(tp_name) + bitmap_size.cx / 2;

  if ((PixelScalar)width > (rc_name.right - rc_name.left)) {
    if (is_current_tp && bearing_direction != 1)
        rc_name.right += GetHintWidth() / 2;
    if (is_current_tp && bearing_direction == 1)
        rc_name.right -= Layout::Scale(5);

    left_bitmap = rc_name.left;

  } else
    left_bitmap = rc_name.left + (rc_name.right - rc_name.left - width) / 2;

  // TODO make clip to show bearing bitmap and also clip for canvas
  canvas.DrawClippedText(left_bitmap + bitmap_size.cx / 2,
                  line_two_y_offset,
                  rc_name.right - rc_name.left - bitmap_size.cx / 2, tp_name);

  // draw checkmark next to name if oz entered
  if (draw_checkmark) {

    const int offsety = ((PixelScalar)line_two_y_offset + bitmap_size.cy <= rc.bottom) ?
        (line_two_y_offset + (rc.bottom - line_two_y_offset - bitmap_size.cy)
            / 2 - Layout::Scale(1)) : rc.bottom - bitmap_size.cy - Layout::Scale(1);
    RasterPoint upper_left(left_bitmap, rc.top + offsety);
    RasterPoint lower_right(upper_left.x + bitmap_size.cx / 2,
                            upper_left.y + bitmap_size.cy);
    if (canvas.GetRect().IsInside(upper_left) && canvas.GetRect().IsInside(lower_right)) {
#ifdef ENABLE_OPENGL
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    const GLEnable scope(GL_TEXTURE_2D);
    const GLBlend blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLTexture &texture = *bmp->GetNative();
    texture.Bind();
    texture.Draw(upper_left.x, upper_left.y);
#else
    canvas.CopyAnd(upper_left.x,
                   upper_left.y,
                   bitmap_size.cx / 2,
                   bitmap_size.cy,
                   *bmp,
                   bitmap_size.cx / 2, 0);
#endif
    }

  }
}

int
SliderShape::DrawBearing(Canvas &canvas, const PixelRect &rc_outer, const Angle &bearing)
{
  enum bearing_levels {
    first = 2,
    second = 10,
    third = 20,
    fourth = 30,
  };
  const IconLook &icon_look = UIGlobals::GetIconLook();
  const Bitmap *bmp_bearing = nullptr;
  int direction = 0;
  if (bearing.AsDelta().Degrees() > fixed(first)) {
    if (bearing.AsDelta().Degrees() > fixed(fourth))
      bmp_bearing = &icon_look.hBmpBearingRightFour;
    else if (bearing.AsDelta().Degrees() > fixed(third))
      bmp_bearing = &icon_look.hBmpBearingRightThree;
    else if (bearing.AsDelta().Degrees() > fixed(second))
      bmp_bearing = &icon_look.hBmpBearingRightTwo;
    else
      bmp_bearing = &icon_look.hBmpBearingRightOne;
    direction = 1;
  }

  if (bearing.AsDelta().Degrees() < fixed(-first)) {
    if (bearing.AsDelta().Degrees() < fixed(-fourth))
      bmp_bearing = &icon_look.hBmpBearingLeftFour;
    else if (bearing.AsDelta().Degrees() < fixed(-third))
      bmp_bearing = &icon_look.hBmpBearingLeftThree;
    else if (bearing.AsDelta().Degrees() < fixed(-second))
      bmp_bearing = &icon_look.hBmpBearingLeftTwo;
    else
      bmp_bearing = &icon_look.hBmpBearingLeftOne;
    direction = -1;
  }

  if (direction == 0)
    return 0;

  PixelSize bmp_bearing_size = bmp_bearing->GetSize();
  const PixelScalar vert_margin = points[2].y - bmp_bearing_size.cy / 2;

  UPixelScalar x_offset = (direction == -1) ? bearing_icon_hor_margin + 1 :
      GetWidth() - bearing_icon_hor_margin -
#ifdef ENABLE_OPENGL
      bmp_bearing_size.cx; // not sure why this bug exists 672 introduced
#else
  bmp_bearing_size.cx / 2;
#endif

  RasterPoint upper_left(rc_outer.left + x_offset, vert_margin);
  RasterPoint lower_right(upper_left.x + bmp_bearing_size.cx / 2,
                          upper_left.y + bmp_bearing_size.cy);

  if (canvas.GetRect().IsInside(upper_left) &&
      canvas.GetRect().IsInside(lower_right)) {
#ifdef ENABLE_OPENGL
    OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    const GLEnable scope(GL_TEXTURE_2D);
    const GLBlend blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLTexture &texture = *bmp_bearing->GetNative();
    texture.Bind();
    texture.Draw(upper_left.x, upper_left.y);
#else
    canvas.CopyAnd(upper_left.x,
                   upper_left.y,
                   bmp_bearing_size.cx / 2, bmp_bearing_size.cy,
                   *bmp_bearing,
                   bmp_bearing_size.cx / 2, 0);
#endif
  }
  return direction;
}

void
SliderShape::Resize(UPixelScalar map_width)
{
  const UPixelScalar large_font_height = GetLargeFont().GetHeight();
  const UPixelScalar medium_font_height = GetMediumFont().GetHeight();
  const UPixelScalar arrow_point_bluntness = 0;
  const UPixelScalar raw_total_width = Layout::Scale(360);

  UPixelScalar total_height = large_font_height
      + medium_font_height - Layout::Scale(2);

  total_height = std::max(total_height, UPixelScalar(
      bearing_icon_size.cy + bearing_icon_hor_margin));

  UPixelScalar raw_hint_width =
      (total_height - arrow_point_bluntness) / 2;  // make 45 degree angle

  raw_hint_width = std::max(raw_hint_width, UPixelScalar(
      bearing_icon_size.cx / 2 + bearing_icon_hor_margin));

  total_height = std::max(total_height, UPixelScalar(
      raw_hint_width * 2 + arrow_point_bluntness));

  SetLine1Y(0u);
  SetLine2Y(total_height - large_font_height - 1);
  SetLine3Y(0u);

  //top
  points[0].x = raw_hint_width;
  points[0].y = 0;
  points[1].x = raw_total_width - raw_hint_width;
  points[1].y = 0;

  //right arrow tip
  points[2].x = raw_total_width;
  points[2].y = (total_height - arrow_point_bluntness) / 2;
  points[3].x = raw_total_width;
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

  PixelScalar amount_to_grow_x = map_width
      - (GetHintWidth() * 2) - raw_total_width - 1;
  PixelScalar neg_min_grow = points[5].x - points[4].x;
  amount_to_grow_x = (amount_to_grow_x < neg_min_grow) ? neg_min_grow :
      amount_to_grow_x;

  for (unsigned i = 1; i <= 4; i++)
    points[i].x += amount_to_grow_x;
}
