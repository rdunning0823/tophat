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

#include "MenuBar.hpp"
#include "Screen/ContainerWindow.hpp"
#include "Screen/Layout.hpp"
#include "Input/InputEvents.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"

#include <assert.h>

/**
 * creates button positions 15 to MAX_BUTTONS -1
 * 16-20 are always on the left side of the screen
 * 21-25 are always on the right side of the screen
 * 26 is the "Cancel" position, and is bottom left
 * for portrait, Bottom right for Landscape
 * 27 is "More/Less" and located next to the Cancel button
 * 28 is on the Bottom right
 *
 * @param i. index of buttons 16-31
 * @param rc.  rc of screen
 * @return rc of button
 */
gcc_pure
static PixelRect
GetButtonPositionFixed(unsigned i, PixelRect rc)
{
  if (i < 16 || i > 31)
    i = 0;
  else
    i -= 15;

  UPixelScalar hwidth = rc.right - rc.left;
  UPixelScalar hheight = rc.bottom - rc.top;
  const bool portrait = hheight > hwidth;

  if (portrait)
    hwidth /= 3;
  else
    hwidth /= 4;

  hheight /= 6;

  if (i == 0) {
    rc.left = rc.right;
    rc.top = rc.bottom;
  } else {

    // Cancel button
    if (i == 11) {
      if (!portrait)
        rc.left = rc.right - hwidth;
      rc.top = 5 * hheight;

      // More/less button
    } else if (i == 12) {
      if (!portrait)
        rc.left = rc.right - hwidth * 2;
      else
        rc.left += hwidth;
      rc.top = 5 * hheight;

      // lower right
    } else if (i == 13) {
      rc.left = rc.right - hwidth;
      rc.top = rc.bottom - hheight;

    } else {
      rc.top += ((i - 1) % 5) * hheight;
      if (i > 5)
        rc.left = rc.right - hwidth;
    }
  }

  rc.right = rc.left + hwidth;
  rc.bottom = rc.top + hheight;

  return rc;
}


gcc_pure
static PixelRect
GetButtonPosition(unsigned i, PixelRect rc)
{
  if (i > 15)
    return GetButtonPositionFixed(i, rc);

  UPixelScalar hwidth = rc.right - rc.left;
  UPixelScalar hheight = rc.bottom - rc.top;

  if (hheight > hwidth) {
    // portrait

    hheight /= 6;

    if (i == 0) {
      rc.left = rc.right;
      rc.top = rc.bottom;
    } else if (i < 5) {
      hwidth /= 4;

      rc.left += hwidth * (i - 1);
      rc.top = rc.bottom - hheight;
    } else {
      hwidth /= 3;

      rc.left = rc.right - hwidth;

      if (IsAltair()) {
        PixelScalar k = rc.bottom - rc.top;
        // JMW need upside down button order for rotated Altair
        rc.top = rc.bottom - (i - 5) * k / 5 - hheight - Layout::Scale(20);
      } else {
        rc.top += (i - 5) * hheight;
      }
    }

    rc.right = rc.left + hwidth;
    rc.bottom = rc.top + hheight;
  } else {
    // landscape

    hwidth /= 5;
    hheight /= 5;

    if (i == 0) {
      rc.left = rc.right;
      rc.top = rc.bottom;
    } else if (i < 5) {
      rc.top += hheight * (i - 1);
    } else {
      rc.left += hwidth * (i - 5);
      rc.top = rc.bottom - hheight;
    }

    rc.right = rc.left + hwidth;
    rc.bottom = rc.top + hheight;
  }

  return rc;
}

bool
MenuBar::Button::OnClicked()
{
  if (event > 0)
    InputEvents::ProcessEvent(event);
  return true;
}

MenuBar::MenuBar(ContainerWindow &parent, const ButtonLook &look)
{
  const PixelRect rc = parent.GetClientRect();

  WindowStyle style;
  style.Hide();
  style.Border();

  for (unsigned i = 0; i < MAX_BUTTONS; ++i) {
    PixelRect button_rc = GetButtonPosition(i, rc);
    buttons[i].Create(parent, button_rc, style,
                      new SymbolButtonRenderer(look, _T("")));
  }
}

void
MenuBar::ShowButton(unsigned i, bool enabled, const TCHAR *text,
                    unsigned event, bool focused,
                    bool background_transparent)
{
  assert(i < MAX_BUTTONS);

  Button &button = buttons[i];

//  buttons[i].SetFocusedOverride(focused);
  button.SetCaption(text);
  button.SetEnabled(enabled && event > 0);
  button.SetEvent(event);
  button.ShowOnTop();
  button.SetForceTransparent(background_transparent);
}

void
MenuBar::HideButton(unsigned i)
{
  assert(i < MAX_BUTTONS);

  buttons[i].Hide();
}

void
MenuBar::OnResize(const PixelRect &rc)
{
  for (unsigned i = 0; i < MAX_BUTTONS; ++i)
    buttons[i].Move(GetButtonPosition(i, rc));
}
