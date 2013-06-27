/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#ifndef XCSOAR_KEYBOARD_CONTROL_HPP
#define XCSOAR_KEYBOARD_CONTROL_HPP

#include "Screen/ContainerWindow.hpp"
#include "Screen/ButtonWindow.hpp"

#include <tchar.h>

struct DialogLook;

class KeyboardBaseControl : public ContainerWindow {
public:
  typedef void (*OnCharacterCallback_t)(TCHAR key);

protected:
  const DialogLook &look;

  OnCharacterCallback_t on_character;

  enum {
    MAX_BUTTONS = 40,
  };

  UPixelScalar button_width;
  UPixelScalar button_height;

  unsigned num_buttons;
  ButtonWindow buttons[MAX_BUTTONS];
  TCHAR button_values[MAX_BUTTONS];

  const Font &button_font;

public:

  KeyboardBaseControl(ContainerWindow &parent,
                      const DialogLook &_look,
                      PixelRect rc,
                      OnCharacterCallback_t _on_character,
                      const WindowStyle _style,
                      const Font &_button_font);

  /**
   * Show only the buttons representing the specified character list.
   */
  virtual void SetAllowedCharacters(const TCHAR *allowed) {};

  void SetOnCharacterCallback(OnCharacterCallback_t _on_character) {
    on_character = _on_character;
  }

protected:
  virtual void OnPaint(Canvas &canvas) override;
  virtual bool OnCommand(unsigned id, unsigned code) override;

  void AddButton(const TCHAR *caption);

  /**
   * Resizes the button to specified width and height values according to display pixels!
   *
   *
   * @param ch
   * @param width   Width measured in display pixels!
   * @param height  Height measured in display pixels!
   */
  void MoveButton(TCHAR ch, PixelScalar left, PixelScalar top);
  void ResizeButton(TCHAR ch, UPixelScalar width, UPixelScalar height);
  void ResizeButtons();
  void SetButtonsSize();
  void MoveButtonsToRow(const TCHAR *buttons, int row,
                        PixelScalar offset_left = 0);

private:
  gcc_pure
  virtual ButtonWindow *FindButton(TCHAR ch) = 0;

};

class KeyboardControl : public KeyboardBaseControl {

public:
  KeyboardControl(ContainerWindow &parent, const DialogLook &look,
                  PixelRect rc,
                  OnCharacterCallback_t on_character,
                  const WindowStyle _style = WindowStyle());

  /**
   * Show only the buttons representing the specified character list.
   */
  virtual void SetAllowedCharacters(const TCHAR *allowed) override;

protected:
  virtual void OnResize(PixelSize new_size) override;

private:
  gcc_pure
  virtual ButtonWindow *FindButton(TCHAR ch);

  void MoveButtons();

  gcc_pure
  bool IsLandscape() const {
    return GetWidth() >= GetHeight();
  }
};

#endif
