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

#ifndef XCSOAR_KEYBOARD_NUMERIC_WIDGET_HPP
#define XCSOAR_KEYBOARD_NUMERIC_WIDGET_HPP

#include "Widget.hpp"
#include "Form/CharacterButton.hpp"
#include "Form/ActionListener.hpp"

#include <tchar.h>

struct ButtonLook;
class WndSymbolButton;

class KeyboardNumericWidget : public NullWidget {
public:
  typedef bool (*OnCharacterCallback_t)(unsigned ch);

protected:
  static constexpr unsigned MAX_BUTTONS = 40;

  const ButtonLook &look;

  OnCharacterCallback_t on_character;

  UPixelScalar button_width;
  UPixelScalar button_height;

  unsigned num_buttons;
  CharacterButton buttons[MAX_BUTTONS];

  /**
   * show the minus sign
   */
  bool show_minus;

public:
  KeyboardNumericWidget(const ButtonLook &_look,
                        OnCharacterCallback_t _on_character, bool _show_minus)
    :look(_look), on_character(_on_character), num_buttons(0),
     show_minus(_show_minus) {}

  /**
   * Show only the buttons representing the specified character list.
   */
  void SetAllowedCharacters(const TCHAR *allowed);

private:
  void OnResize(const PixelRect &rc);

  gcc_pure
  Button *FindButton(unsigned ch);

  void MoveButton(unsigned ch, PixelScalar left, PixelScalar top);
  void ResizeButton(unsigned ch, UPixelScalar width, UPixelScalar height);
  void ResizeButtons();
  void SetButtonsSize();
  void MoveButtonsToRow(const PixelRect &rc,
                        const TCHAR *buttons, unsigned row,
                        PixelScalar offset_left = 0);
  void MoveButtons(const PixelRect &rc);

  gcc_pure
  static bool IsLandscape(const PixelRect &rc) {
    return rc.right - rc.left >= rc.bottom - rc.top;
  }

  void AddButton(ContainerWindow &parent, const TCHAR *caption, unsigned ch);

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;
  virtual void Move(const PixelRect &rc) override;
};

#endif
