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

#ifndef XCSOAR_SYMBOL_RENDERER_HPP
#define XCSOAR_SYMBOL_RENDERER_HPP

#include "Screen/Point.hpp"

class Canvas;

namespace SymbolRenderer
{
  enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
  };

  /**
   * @param no_margins.  scales 1:1 and eliminates margins if true
   */
  void DrawArrow(Canvas &canvas, PixelRect rc, Direction direction,
                 bool no_margins = false);
  void DrawDoubleArrow(Canvas &canvas, PixelRect rc, Direction direction,
                       bool no_margins = false);
  void DrawSign(Canvas &canvas, PixelRect rc, bool plus);
  void DrawPause(Canvas &canvas, PixelRect rc);
  void DrawStop(Canvas &canvas, PixelRect rc);
}

#endif
