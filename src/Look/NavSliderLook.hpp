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

#ifndef XCSOAR_NAV_SLIDER_LOOK_HPP
#define XCSOAR_NAV_SLIDER_LOOK_HPP

#include "Screen/Pen.hpp"
#include "Screen/Brush.hpp"
#include "Screen/Font.hpp"
#include "Util/Macros.hpp"

class Font;

struct NavSliderLook {

  Color border_color;

  Pen background_pen;
  Brush background_brush;
  Brush background_brush_selected;

  Pen border_pen_thin;
  Pen border_pen_thick;
  /* GDI does not support Pen::GetWidth() */
  unsigned border_pen_width_thin, border_pen_width_thick, background_pen_width;
  Font large_font, medium_font, small_font;

  void Initialise(unsigned font_scale_nav_bar_waypoint_name,
                  unsigned font_scale_nav_bar_distance);

  void ReinitialiseLayout(unsigned font_scale_nav_bar_waypoint_name,
                          unsigned font_scale_nav_bar_distance);
  const Pen &GetBorderPen(bool use_wide_pen) const {
    return use_wide_pen ? border_pen_thick : border_pen_thin;
  }
  unsigned GetBorderPenWidth(bool use_wide_pen) const {
    return use_wide_pen ? border_pen_width_thick : border_pen_width_thin;
  }
  const Brush &GetBackgroundBrush(bool selected) const {
    return selected ? background_brush_selected : background_brush;
  }

};

#endif
