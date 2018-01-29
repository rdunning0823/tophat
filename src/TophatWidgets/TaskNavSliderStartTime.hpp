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

#ifndef XCSOAR_TASK_NAV_SLIDER_START_TIME_HPP
#define XCSOAR_TASK_NAV_SLIDER_START_TIME_HPP

#include "Util/StaticString.hxx"
#include "Math/fixed.hpp"

class SliderStartTime {
public:

  typedef StaticString<120> TypeBuffer;

protected:
  bool is_ordered;
  unsigned idx;
  bool start_cylinder_proximity;
  bool settings_show_two_minute_start;
  bool flying;
  bool is_USA;
  int max_height;

public:
  SliderStartTime()
  :is_ordered(false),
   idx(0),
   start_cylinder_proximity(false),
   settings_show_two_minute_start(false),
   flying(false),
   is_USA(false),
   max_height(fixed(0)) {};

  void
  SetTypeTextFor2MinuteCount(TypeBuffer &type_buffer,
                             TypeBuffer &type_buffer_short) const;

  void Init(bool _is_ordered, unsigned _idx,
            bool _is_glider_close_to_start_cylinder,
            bool _task_settings_show_2_minutes,
            bool _flying, bool _is_USA, bool _max_height);

  /** do we display the Two Minute Start value */
  bool ShowTwoMinutes() const;

  /**
   * @return -1 if time under is invalid and should not be displayed,
   * else, elapsed seconds under start height
   */
  int GetTimeUnderStart() const;




};

#endif
