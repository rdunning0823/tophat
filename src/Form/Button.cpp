/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
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

#include "Form/Button.hpp"
#include "Form/ActionListener.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Key.h"
#include "Screen/Canvas.hpp"
#include "Asset.hpp"

WndButton::WndButton(ContainerWindow &parent, const DialogLook &_look,
                     const TCHAR *Caption, const PixelRect &rc,
                     ButtonWindowStyle style,
                     ClickNotifyCallback _click_callback,
                     LeftRightNotifyCallback _left_callback,
                     LeftRightNotifyCallback _right_callback)
  :look(_look), renderer(look.button),
   listener(NULL),
   click_callback(_click_callback),
   left_callback(_left_callback),
   right_callback(_right_callback)
{
  style.EnableCustomPainting();
  set(parent, Caption, rc, style);
  SetFont(*look.button.font);
}


WndButton::WndButton(ContainerWindow &parent, const DialogLook &_look,
                     const TCHAR *caption, const PixelRect &rc,
                     ButtonWindowStyle style,
                     ActionListener *_listener, int _id)
  :look(_look), renderer(look.button),
#ifdef USE_GDI
   id(_id),
#endif
   listener(_listener),
   click_callback(NULL), left_callback(NULL), right_callback(NULL)
{
  style.EnableCustomPainting();
#ifdef USE_GDI
  /* use BaseButtonWindow::COMMAND_BOUNCE_ID */
  set(parent, caption, rc, style);
#else
  /* our custom SDL/OpenGL button doesn't need this hack */
  set(parent, caption, _id, rc, style);
#endif
  SetFont(*look.button.font);
}

bool
WndButton::OnClicked()
{
  if (listener != NULL) {
#ifndef USE_GDI
    unsigned id = GetID();
#endif
    listener->OnAction(id);
    return true;
  }

  // Call the OnClick function
  if (click_callback != NULL) {
    click_callback(*this);
    return true;
  }

  return ButtonWindow::OnClicked();
}

bool
WndButton::on_left()
{
  // call on Left key function
  if (left_callback != NULL) {
    left_callback(*this);
    return true;
  }
  return false;
}

bool
WndButton::on_right()
{
  // call on Left key function
  if (right_callback != NULL) {
    right_callback(*this);
    return true;
  }
  return false;
}

#ifdef USE_GDI

void
WndButton::OnSetFocus()
{
  ButtonWindow::OnSetFocus();

  /* GDI's "BUTTON" class on Windows CE Core (e.g. Altair) does not
     repaint when the window gets focus, but our custom style requires
     it */
  ::InvalidateRect(hWnd, NULL, false);
}

void
WndButton::OnKillFocus()
{
  ButtonWindow::OnKillFocus();

  /* GDI's "BUTTON" class does not repaint when the window loses
     focus, but our custom style requires it */
  ::InvalidateRect(hWnd, NULL, false);
}

#endif

bool
WndButton::OnKeyCheck(unsigned key_code) const
{
  switch (key_code) {
  case VK_LEFT:
    return left_callback != NULL;

  case VK_RIGHT:
    return right_callback != NULL;

  default:
    return ButtonWindow::OnKeyCheck(key_code);
  }
}

bool
WndButton::OnKeyDown(unsigned key_code)
{
  switch (key_code) {
  case VK_LEFT:
    return on_left();

  case VK_RIGHT:
    return on_right();
  }

  return ButtonWindow::OnKeyDown(key_code);
}

void
WndButton::OnPaint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.get_width()),
    PixelScalar(canvas.get_height())
  };

  const bool focused = HasFocus();
  const bool pressed = is_down();

  renderer.DrawButton(canvas, rc, focused, pressed);

  // If button has text on it
  tstring caption = get_text();
  if (caption.empty())
    return;

  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SetBackgroundTransparent();
  if (!IsEnabled())
    canvas.SetTextColor(look.button.disabled.color);
  else if (focused)
    canvas.SetTextColor(look.button.focused.foreground_color);
  else
    canvas.SetTextColor(look.button.standard.foreground_color);

  canvas.Select(*(look.button.font));

#ifndef USE_GDI
  canvas.formatted_text(&rc, caption.c_str(), GetTextStyle());
#else
  unsigned style = DT_CENTER | DT_NOCLIP | DT_WORDBREAK;

  PixelRect text_rc = rc;
  canvas.formatted_text(&text_rc, caption.c_str(), style | DT_CALCRECT);
  text_rc.right = rc.right;

  PixelScalar offset = rc.bottom - text_rc.bottom;
  if (offset > 0) {
    offset /= 2;
    text_rc.top += offset;
    text_rc.bottom += offset;
  }

  canvas.formatted_text(&text_rc, caption.c_str(), style);
#endif
}
