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

#include "Form/KeyboardNumeric.hpp"
#include "Look/DialogLook.hpp"
#include "Look/InfoBoxLook.hpp"
#include "Look/Look.hpp"
#include "Screen/Font.hpp"
#include "Util/StringUtil.hpp"
#include "UIGlobals.hpp"
#include "Screen/Canvas.hpp"
#include "Form/Button.hpp"
#include "Screen/Layout.hpp"

#include <assert.h>
#include <string.h>

static constexpr TCHAR keyboard_letters[] =
 _T("789456123.0-");

KeyboardNumericControl::KeyboardNumericControl(ContainerWindow &parent,
                                               const DialogLook &_look,
                                               PixelRect rc,
                                               OnCharacterCallback_t
                                               _on_character,
                                               bool _show_minus,
                                               const WindowStyle _style)
  :KeyboardBaseControl(parent, _look, rc, _on_character, _style,
                       *UIGlobals::GetLook().info_box.value.font),
                       show_minus(_show_minus)
{
  OnResize(PixelSize { rc.right - rc.left, rc.bottom - rc.top });
  TCHAR caption[] = _T(" ");

  for (const TCHAR *i = keyboard_letters; !StringIsEmpty(i); ++i) {
    caption[0] = *i;

    AddButton(caption);
  }
  MoveButtons();
}

void
KeyboardNumericControl::SetAllowedCharacters(const TCHAR *allowed)
{
  for (unsigned i = 0; i < num_buttons; ++i)
    buttons[i].SetVisible(allowed == NULL ||
                          _tcschr(allowed, button_values[i]) != NULL);
}

Button *
KeyboardNumericControl::FindButton(TCHAR ch)
{
  for (unsigned i = 0; i < num_buttons; ++i)
    if (button_values[i] == ch)
      return &buttons[i];

  return nullptr;
}

void
KeyboardNumericControl::MoveButtons()
{
  MoveButtonsToRow(_T("789"), 0);
  MoveButtonsToRow(_T("456"), 1);
  MoveButtonsToRow(_T("123"), 2);
  if (show_minus)
    MoveButtonsToRow(_T(".0-"), 3);
  else
    MoveButtonsToRow(_T(".0 "), 3);
}

void
KeyboardNumericControl::OnResize(PixelSize new_size)
{
  button_width = new_size.cx / 3;
  button_height = new_size.cy / 5;
  MoveButtons();
}
