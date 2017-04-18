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

#ifndef XCSOAR_TASK_NAV_SLIDER_SHAPE_HPP
#define XCSOAR_TASK_NAV_SLIDER_SHAPE_HPP

#include "Screen/Point.hpp"
#include "Math/fixed.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Look/Look.hpp"
#include "Engine/Task/Factory/TaskFactoryType.hpp"
#include "MapSettings.hpp"
#include "Util/StaticString.hxx"

#include <assert.h>
#include <stdint.h>

struct DialogLook;
struct NavSliderLook;
class Font;
class Canvas;
struct Waypoint;
enum class TaskType : uint8_t;

class SliderShape {
public:

  typedef StaticString<120> TypeBuffer;
  typedef StaticString<12> GRBuffer;
  typedef StaticString<30> DistanceBuffer;


  // Direction of bearing to be drawn
  enum BearingDirection {
    Left,
    Right,
    // no bearing
    None
  };

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
    NotVisible,
  };

  PixelRect inner_rect;
  PixelRect outer_rect;

protected:
  RasterPoint points[8];

  const DialogLook &dialog_look;
  const NavSliderLook &nav_slider_look;
  const UISettings &ui_settings;
  const WaypointRendererSettings &wp_renderer_settings;

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
   nav_slider_look(UIGlobals::GetLook().nav_slider),
   ui_settings(CommonInterface::GetUISettings()),
   wp_renderer_settings(CommonInterface::GetMapSettings().waypoint),
   bearing_icon_hor_margin(0) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *bmp_bearing;
    bmp_bearing = &icon_look.hBmpBearingRightOne;
    bearing_icon_size = bmp_bearing->GetSize();
  };

  /**
   * Draws bearing symbol
   * returns direction bearing symbol.
   */
  unsigned DrawBearing(Canvas &canvas, const PixelRect &rc_outer,
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
   * The summed height of all three fonts.
   * A proxy for whether a change in font height requires resizing the shape
   */
  unsigned GetSumFontHeight();

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
   * point[0] is left top line of slider
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
   * draws white background for entire shape
   * @param poly.  the finalized point set
   */
  void DrawBackgroundAll(Canvas &canvas, const RasterPoint poly[]);

  /**
   * draws the full outline but draws the top with narrow line
   * @param poly.  the finalized point set
   * @use_wide_pen. use the wide pen for the border width
   */
  void DrawOutlineAll(Canvas &canvas, const RasterPoint poly[],
                      bool use_wide_pen);
  /**
   * draws the outline of the slider shape
   * @param rc.  rect of the slider shape.  Note that this may overextend
   * the canvas size
   * @use_wide_pen. use the wide pen for the border width
   * @return true if the outline is visible, false if it's off the canvas
   */
  bool DrawOutline(Canvas &canvas, const PixelRect &rc, bool use_wide_pen);

  void DrawInvalid(Canvas &canvas, const PixelRect rc_outer, const PixelRect rc,
                   unsigned idx,
                   bool selected, bool use_wide_pen);
  /**
   * Draws the text and the outline of the shape
   * @param rc_outer. rc of list item.  This may not be visible in the canvas
   */
  void Draw(Canvas &canvas, const PixelRect rc_outer,
            unsigned idx, bool selected, bool is_current_tp,
            const TCHAR *tp_name,
            const Waypoint *twp,
            bool has_entered, bool has_exited,
            TaskType task_mode, TaskFactoryType task_factory_type,
            unsigned task_size,
            bool tp_valid, fixed tp_distance, bool distance_valid,
            fixed tp_altitude_difference,
            bool altitude_difference_valid,
            Angle delta_bearing,
            bool bearing_valid,
            fixed gradient,
            bool gr_valid,
            bool use_wide_pen,
            bool navigate_to_target,
            int time_under_max_start,
            bool show_time_under_max_start);


  /**
   * returns the font to be used for the "type display" in
   * upper left quadrant of bar
   */
  const Font& GetTypeFont(bool is_start, int time_under_max_start) const;
  const Font& GetNameFont() const;
  const Font& GetAltitudeFont() const;
  const Font& GetDistanceFont() const;

  /**
   * text to be displayed as the distance plus option prefix
   */
  void GetDistanceText(DistanceBuffer &distance_buffer,
                       fixed tp_distance, bool distance_valid,
                       bool navigate_to_target,
                       unsigned task_size,
                       bool is_start,
                       bool is_finish);

  /**
   * text to be displayed for gradient
   */
  void GetGRText(GRBuffer &gr_buffer, fixed gradient, bool valid);

  /**
   * sets the text to be used for the "type display" in
   * upper left quadrant of bar
   *
   * @param type_buffer: the buffer that holds the text
   * @param type_buffer_short: the buffer that holds the text.  Abbreviated
   * @param enable_index: is the configuration enabled to show the tp indexes
   * @param time_under_max_start: US task time under max start
   * @param show_time_under_max_start true if show time under message
   */
  void GetTypeText(TypeBuffer &type_buffer, TypeBuffer &type_buffer_short,
                   TaskType task_mode,
                   unsigned idx, unsigned task_size, bool is_start,
                   bool is_finish, bool is_aat, bool navigate_to_target,
                   bool enable_index,
                   int time_under_max_start,
                   bool show_time_under_max_start);

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
