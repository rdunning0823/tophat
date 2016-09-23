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

#ifndef XCSOAR_KEYBOARD_NUMERIC_CONTROL_HPP
#define XCSOAR_KEYBOARD_NUMERIC_CONTROL_HPP

#include "Keyboard.hpp"

#include <tchar.h>

struct DialogLook;
/**
 * a data input screen that looks like a phone numeric keypad
 */
class KeyboardNumericControl : public KeyboardBaseControl {
protected:
  /**
   * show the "-" sign
   */
  bool show_minus;

public:
  KeyboardNumericControl(ContainerWindow &parent, const DialogLook &look,
                         PixelRect rc,
                         OnCharacterCallback_t on_character,
                         bool show_minus,
                         const WindowStyle _style = WindowStyle());

  /**
   * Show only the buttons representing the specified character list.
   */
  void SetAllowedCharacters(const TCHAR *allowed);

protected:
  virtual void OnResize(PixelSize new_size) override;

private:
  gcc_pure
  Button *FindButton(TCHAR ch);

  void MoveButton(TCHAR ch, PixelScalar left, PixelScalar top);

  /**
   *  moves the buttons 0-9, "." and optionally "-" to the current screen size
   */
  void MoveButtons();
};

#endif
