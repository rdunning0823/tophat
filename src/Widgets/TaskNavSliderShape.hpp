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

#ifndef XCSOAR_TASK_NAV_SLIDER_SHAPE_HPP
#define XCSOAR_TASK_NAV_SLIDER_SHAPE_HPP

#include "Screen/Point.hpp"
#include "Math/fixed.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Engine/Task/Factory/TaskFactoryType.hpp"

#include <assert.h>
#include <stdint.h>

struct DialogLook;
struct InfoBoxLook;
class Font;
class Canvas;
enum class TaskType : uint8_t;

class SliderShape {
private:
  /**
   * how much of the shape is visible in the canvas
   */
  enum VisibilityLevel {
    /** the entire shape is visible */
    Full,

    /** some or all of the left tip, but no body */
    LeftTip,
    RightTip,

    /** all of the left tip and some of the body */
    LeftTipAndBody,
    RightTipAndBody,

    /* no part is visible */
    None,
  };

  PixelRect inner_rect;
  PixelRect outer_rect;

protected:
  RasterPoint points[8];

  const DialogLook &dialog_look;
  const InfoBoxLook &infobox_look;

  /**
   * height of the bearing icon
   */
  PixelSize bearing_icon_size;
  UPixelScalar bearing_icon_hor_margin;

  /**
   * the y positions relative to the top of the slider for each line of text
   */
  UPixelScalar text_line_one_y;
  UPixelScalar text_line_two_y;
  UPixelScalar text_line_three_y;

public:
  SliderShape()
  :dialog_look(UIGlobals::GetDialogLook()),
   infobox_look(UIGlobals::GetLook().info_box),
   bearing_icon_hor_margin(0) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const Bitmap *bmp_bearing;
    bmp_bearing = &icon_look.hBmpBearingRightOne;
    bearing_icon_size = bmp_bearing->GetSize();
  };

  /**
   * Draws bearing symbol
   * returns direction bearing symbol.
   *   -1 if to left, +1 if to right
   *   or 0 if no bearing is drawn
   */
  int DrawBearing(Canvas &canvas, const PixelRect &rc_outer,
                   const Angle &bearing);

  UPixelScalar GetWidth() const {
    return points[2].x - points[6].x;
  }

  UPixelScalar GetHeight() const {
    return points[4].y - points[0].y + 1;
  }

  UPixelScalar GetHintWidth() const {
    return points[2].x - points[4].x;
  }

  UPixelScalar GetOverScrollWidth() const {
    return 1 * GetHintWidth();
  }

  const RasterPoint GetPoint(unsigned i) {
    assert(i < 8);
    return points[i];
  }

  /**
   * the font used to display the turnpoint name
   */
  const Font &GetLargeFont();

  /**
   * the font used to display the info about the turnpoint
   */
  const Font &GetSmallFont();

  /**
   * the font used to display the Distance about the turnpoint
   */
  const Font &GetMediumFont();

  /**
   * the y position of line one text for painting
   */
  void SetLine1Y(UPixelScalar y) {
    text_line_one_y = y;
  }
  void SetLine2Y(UPixelScalar y) {
    text_line_two_y = y;
  }
  void SetLine3Y(UPixelScalar y) {
    text_line_three_y = y;
  }
  /**
   * the y position of line one text for painting
   */
  UPixelScalar GetLine1Y() {
    return text_line_one_y;
  }
  UPixelScalar GetLine2Y() {
    return text_line_two_y;
  }
  UPixelScalar GetLine3Y() {
    return text_line_three_y;
  }
  /**
   * returns large rectangle excluding tips of slider shape
   */
  const PixelRect &GetInnerRect() {
    inner_rect.left = points[0].x;
    inner_rect.right = points[4].x;
    inner_rect.top = points[0].y;
    inner_rect.bottom = points[4].y;
    return inner_rect;
  }

  /**
   * returns large rectangle circumscribing the tips of the slider
   */
  const PixelRect &GetOuterRect() {
    outer_rect.left = points[7].x;
    outer_rect.right = points[2].x;
    outer_rect.top = points[0].y;
    outer_rect.bottom = points[4].y;
    return outer_rect;
  }

  /**
   * resizes the slider to fit horizontally in the width of the rc
   * @return height to set ItemHeight() of ListControl
   */
  void Resize(UPixelScalar map_width);

  /**
   * @param poly.  Points of the Shape outline
   * @param canvas
   * @return enum describing what portion of shape is visible in canvas
   */
  VisibilityLevel GetVisibilityLevel(Canvas &canvas, RasterPoint poly[]);

  /**
   * draws the full outline but draws the top with narrow line
   * @param poly.  the finalized point set
   * @param width. the width of all points except the top line
   * @param color
   */
  void DrawOutlineAll(Canvas &canvas, const RasterPoint poly[],
                      unsigned width, const Color color);
  /**
   * draws the outline of the slider shape
   * @param rc.  rect of the slider shape.  Note that this may overextend
   * the canvas size
   * @border_width. the pen border width
   * @return true if the outline is visible, false if it's off the canvas
   */
  bool DrawOutline(Canvas &canvas, const PixelRect &rc, unsigned border_width);

  /**
   * Draws the text and the outline of the shape
   * @param rc_outer. rc of list item.  This may not be visible in the canvas
   */
  void Draw(Canvas &canvas, const PixelRect rc_outer,
            unsigned idx, bool selected, bool is_current_tp,
            const TCHAR *tp_name,
            bool has_entered, bool has_exited,
            TaskType task_mode, TaskFactoryType task_factory_type,
            unsigned task_size,
            bool tp_valid, fixed tp_distance, bool distance_valid,
            fixed tp_altitude_difference,
            bool altitude_difference_valid,
            Angle delta_bearing,
            bool bearing_valid,
            unsigned border_width);

#ifdef _WIN32
  /**
   * clears background adjacent to slider.
   * WIN32 does not draw transparent like OpenGL does
   */
  void PaintBackground(Canvas &canvas, unsigned idx,
                       unsigned task_size,
                       const DialogLook &dialog_look,
                       const PixelRect rc_outer);
#endif
};

#endif
