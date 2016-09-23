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

#ifndef XCSOAR_SYMBOL_BUTTON_RENDERER_HPP
#define XCSOAR_SYMBOL_BUTTON_RENDERER_HPP

#include "TextButtonRenderer.hpp"
#include "Util/StaticString.hxx"
#include "Screen/Icon.hpp"

#include <tchar.h>

/**
 * A #ButtonRenderer instance that renders a regular button frame and
 * a symbol.
 */
class SymbolButtonRenderer : public TextButtonRenderer {

public:

  enum PrefixIcon {
    NONE,
    CHECK_MARK,
    HOME,
    SEARCH,
    SEARCH_CHECKED,
    SPEEDOMETER,
  } prefix_icon;

  SymbolButtonRenderer(const ButtonLook &_look,
                       StaticString<96>::const_pointer _caption)
    :TextButtonRenderer(_look, _caption), prefix_icon(PrefixIcon::NONE) {}

  gcc_pure
  virtual unsigned GetMinimumButtonWidth() const;

  void SetPrefixIcon(PrefixIcon type) {
    prefix_icon = type;
  }

private:
  virtual void DrawButton(Canvas &canvas, const PixelRect &rc,
                          bool enabled, bool focused, bool pressed,
                          bool transparent_background_force) const;

  /**
   * @param transparent_background_force. draws transparent background
   */
  void DrawSymbol(Canvas &canvas, PixelRect rc,
                  bool enabled, bool focused, bool pressed,
                  bool transparent_background_force) const;

  /**
   * @return. Pointer to icon with this prefix, or null
   */
  const MaskedIcon *GetIcon(PrefixIcon prefix_icon) const;

  /**
   * Displays icon if exists plus caption based on prefix_icon value
   * @param text
   *
   */
  void DrawIconAndText(Canvas &canvas, PixelRect rc,
                       const TCHAR *text, const MaskedIcon *icon,
                       bool enabled, bool
                       focused, bool pressed,
                       bool transparent_background_force) const;
};

#endif
