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

#ifndef XCSOAR_TEXT_WIDGET_HPP
#define XCSOAR_TEXT_WIDGET_HPP

#include "WindowWidget.hpp"
#include "Screen/Point.hpp"

#include <tchar.h>

class Color;
struct DialogLook;

/**
 * A #Widget implementation that displays multi-line text.
 */
class TextWidget : public WindowWidget {
public:
  PixelScalar fixed_height;
  unsigned num_rows_text;
protected:
  bool bold;
  bool v_center;
public:

  void SetText(const TCHAR *text);
  void SetColor(Color _color);
  void SetBold(bool val) {
    bold = val;
  }
  void SetVAlignCenter()
  {
    v_center = true;
  }

  TextWidget()
  : fixed_height(0u), num_rows_text(1u),
    bold(false), v_center(false){}

  TextWidget(unsigned _rows_text)
  : fixed_height(0u), num_rows_text(_rows_text),
    bold(false), v_center(false){}

  /* virtual methods from class Widget */
  PixelSize GetMinimumSize() const override;
  PixelSize GetMaximumSize() const override;

  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  void Unprepare() override;
  /**
   * if non-zero, determines the height of the widget
   */
  virtual void SetFixedHeight(PixelScalar height) {
    fixed_height = height;
  }
};

#endif
