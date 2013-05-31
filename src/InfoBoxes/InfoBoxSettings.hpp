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

#ifndef XCSOAR_INFO_BOX_SETTINGS_HPP
#define XCSOAR_INFO_BOX_SETTINGS_HPP

#include "Util/StaticString.hpp"
#include "Compiler.h"
#include "InfoBoxes/Content/Factory.hpp"

#include <stdint.h>

enum InfoBoxBorderAppearance_t {
  apIbBox = 0,
  apIbTab
};

struct InfoBoxSettings {
  struct Panel {
    static const unsigned MAX_CONTENTS = 24;

    StaticString<32u> name;
    InfoBoxFactory::Type contents[MAX_CONTENTS];

    void Clear();

    gcc_pure
    bool IsEmpty() const;
  };

  static const unsigned int MAX_PANELS = 8;
  static const unsigned int PREASSIGNED_PANELS = 3;

  /**
   * Auto-switch to the "final glide" panel if above final glide?
   * This setting affects the #DisplayMode, and is checked by
   * GetNewDisplayMode().
   */
  bool use_final_glide;

  enum class Geometry : uint8_t {
    /** default, infoboxes along top and bottom, map in middle */
    TOP_4_BOTTOM_4 = 0,

    /** both infoboxes along bottom */
    BOTTOM_8 = 1,

    /** both infoboxes along top */
    TOP_8 = 2,

    /** infoboxes along both sides */
    LEFT_4_RIGHT_4 = 3,

    /** infoboxes along left side */
    LEFT_8 = 4,

    /** infoboxes along right side */
    RIGHT_8 = 5,

    /** infoboxes GNAV (9 right + vario) */
    RIGHT_9_VARIO = 6,

    /** infoboxes (5) along right side (square screen) */
    RIGHT_5 = 7,

    /** 12 infoboxes along right side (i.e. like GNav without vario) */
    RIGHT_12 = 8,

    /** 24 infoboxes along right side (3x8) */
    RIGHT_24 = 9,

    /** 12 infoboxes along bottom */
    BOTTOM_12 = 10,

    /** 12 infoboxes along top */
    TOP_12 = 11,

    /** 6 left, 3 right + vario */
    LEFT_6_RIGHT_3_VARIO = 12,

    /** 8 bottom + vario */
    BOTTOM_8_VARIO = 13,
    TOP_4 = 14,
    BOTTOM_4 = 15,
    RIGHT_4 = 16,
    LEFT_4 = 17,
  } geometry;

  bool inverse, use_colors;

  InfoBoxBorderAppearance_t border_style;

  Panel panels[MAX_PANELS];

  void SetDefaults();
};

#endif
