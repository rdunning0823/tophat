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

#include "KeyboardNumericWidget.hpp"
#include "Look/ButtonLook.hpp"
#include "Util/StringUtil.hpp"
#include "Util/CharUtil.hpp"
#include "Screen/Canvas.hpp"
#include "Form/Button.hpp"
#include "Screen/Layout.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"

#include <assert.h>
#include <string.h>

static constexpr TCHAR keyboard_letters[] =
  _T("1234567890.-");

void
KeyboardNumericWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  OnResize(rc);

  TCHAR caption[] = _T(" ");

  for (const TCHAR *i = keyboard_letters; !StringIsEmpty(i); ++i) {
    caption[0] = *i;

    AddButton(parent, caption, *i);
  }
}

void
KeyboardNumericWidget::Show(const PixelRect &rc)
{
  OnResize(rc);

  for (unsigned i = 0; i < num_buttons; ++i)
    buttons[i].Show();
}

void
KeyboardNumericWidget::Hide()
{
  for (unsigned i = 0; i < num_buttons; ++i)
    buttons[i].Hide();
}

void
KeyboardNumericWidget::Move(const PixelRect &rc)
{
  OnResize(rc);
}

void
KeyboardNumericWidget::SetAllowedCharacters(const TCHAR *allowed)
{
  for (unsigned i = 0; i < num_buttons; ++i)
    buttons[i].SetVisible(allowed == nullptr ||
                          StringFind(allowed, buttons[i].GetCharacter())
                          != nullptr);
}

Button *
KeyboardNumericWidget::FindButton(unsigned ch)
{
  for (unsigned i = 0; i < num_buttons; ++i)
    if (buttons[i].GetCharacter() == ch)
      return &buttons[i];

  return nullptr;
}

/**
 * Move button to the specified left and top coordinates.
 *
 * The coordinates SHOULD BE in pixels of the screen (i.e. after scaling!)
 *
 * @param ch
 * @param left    Number of pixels from the left (in screen pixels)
 * @param top     Number of pixels from the top (in screen pixels)
 */
void
KeyboardNumericWidget::MoveButton(unsigned ch, PixelScalar left, PixelScalar top)
{
  Button *kb = FindButton(ch);
  if (kb)
    kb->Move(left, top);
}

/**
 * Resizes the button to specified width and height values according to display pixels!
 *
 *
 * @param ch
 * @param width   Width measured in display pixels!
 * @param height  Height measured in display pixels!
 */
void
KeyboardNumericWidget::ResizeButton(unsigned ch,
                       UPixelScalar width, UPixelScalar height)
{
  Button *kb = FindButton(ch);
  if (kb)
    kb->Resize(width, height);
}

void
KeyboardNumericWidget::ResizeButtons()
{
  for (unsigned i = 0; i < num_buttons; ++i)
    buttons[i].Resize(button_width, button_height);
}

void
KeyboardNumericWidget::MoveButtonsToRow(const PixelRect &rc,
                       const TCHAR *buttons, unsigned row,
                       PixelScalar offset)
{
  if (StringIsEmpty(buttons))
    return;

  for (unsigned i = 0; buttons[i] != _T('\0'); i++) {
    MoveButton(buttons[i],
               rc.left + i * button_width + offset,
               rc.top + row * button_height);
  }
}

void
KeyboardNumericWidget::MoveButtons(const PixelRect &rc)
{
  MoveButtonsToRow(rc, _T("789"), 0);
  MoveButtonsToRow(rc, _T("456"), 1);
  MoveButtonsToRow(rc, _T("123"), 2);
  if (show_minus)
    MoveButtonsToRow(rc, _T(".0-"), 3);
  else
    MoveButtonsToRow(rc, _T(".0 "), 3);
}

void
KeyboardNumericWidget::OnResize(const PixelRect &rc)
{
  const PixelSize new_size = rc.GetSize();
  button_width = new_size.cx / 3;
  button_height = new_size.cy / 5;

  ResizeButtons();
  MoveButtons(rc);
}

void
KeyboardNumericWidget::AddButton(ContainerWindow &parent,
                       const TCHAR *caption, unsigned ch)
{
  assert(num_buttons < MAX_BUTTONS);

  WindowStyle style;
  style.Hide();

  PixelRect rc;
  rc.left = 0;
  rc.top = 0;
  rc.right = button_width;
  rc.bottom = button_height;

  CharacterButton &button = buttons[num_buttons++];
  button.Create(parent, look, caption, rc, on_character, ch, style);
}
