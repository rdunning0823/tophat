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

#ifndef XCSOAR_INFO_BOX_LAYOUT_HPP
#define XCSOAR_INFO_BOX_LAYOUT_HPP

#include "InfoBoxSettings.hpp"
#include "Screen/Point.hpp"
#include "Compiler.h"

namespace InfoBoxLayout
{
  struct Layout {
    InfoBoxSettings::Geometry geometry;

    bool landscape;

    PixelSize control_size;

    unsigned count;
    PixelRect positions[InfoBoxSettings::Panel::MAX_CONTENTS];

    PixelRect vario;

    PixelRect remaining;

    bool HasVario() const {
      return vario.right > vario.left && vario.bottom > vario.top;
    }

    void ClearVario() {
      vario.left = vario.top = vario.right = vario.bottom = 0;
    }
  };

  gcc_pure
  Layout
  Calculate(PixelRect rc, InfoBoxSettings::Geometry geometry);

  gcc_const
  int
  GetBorder(InfoBoxSettings::Geometry geometry, bool landscape, unsigned i);

  /**
   * will this geometry display InfoBoxes across the bottom of the screen
   * when in portrait mode?
   * @param geometry
   * @return bool
   */
  gcc_const
  bool
  HasInfoBoxesOnBottom(InfoBoxSettings::Geometry geometry);
};

#endif
