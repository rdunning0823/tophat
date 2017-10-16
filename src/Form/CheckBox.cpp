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

#include "Form/CheckBox.hpp"
#include "Form/ActionListener.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Key.h"
#include "Asset.hpp"
#include "Util/Macros.hpp"

void
CheckBoxControl::Create(ContainerWindow &parent, const DialogLook &_look,
                        tstring::const_pointer _caption,
                        const PixelRect &rc,
                        const WindowStyle style,
                        ActionListener &_listener, int _id)
{
  checked = dragging = pressed = false;
  look = &_look;
  caption = _caption;

  text_renderer.SetCenter(false);
  text_renderer.SetVCenter();
  text_renderer.SetControl();

  align_style = BoxAlignment::Left;
  listener = &_listener;
  id = _id;
  PaintWindow::Create(parent, rc, style);
}

void
CheckBoxControl::SetState(bool value)
{
  if (value == checked)
    return;

  checked = value;
  Invalidate();
}

void
CheckBoxControl::SetPressed(bool value)
{
  if (value == pressed)
    return;

  pressed = value;
  Invalidate();
}

bool
CheckBoxControl::OnClicked()
{
  if (listener != nullptr) {
    listener->OnAction(id);
    return true;
  }

  return false;
}

bool
CheckBoxControl::OnKeyCheck(unsigned key_code) const
{
  switch (key_code) {
  case KEY_RETURN:
    return true;

  default:
    return false;
  }
}

bool
CheckBoxControl::OnKeyDown(unsigned key_code)
{
  switch (key_code) {
#ifdef GNAV
  // JMW added this to make data entry easier
  case KEY_APP4:
#endif
  case KEY_RETURN:
  case KEY_SPACE:
    SetState(!GetState());
    OnClicked();
    return true;
  }

  return PaintWindow::OnKeyDown(key_code);
}

bool
CheckBoxControl::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (dragging) {
    SetPressed(IsInside(x, y));
    return true;
  } else
    return PaintWindow::OnMouseMove(x, y, keys);
}

bool
CheckBoxControl::OnMouseDown(PixelScalar x, PixelScalar y)
{
  if (IsTabStop())
    SetFocus();

  SetPressed(true);
  SetCapture();
  dragging = true;
  return true;
}

bool
CheckBoxControl::OnMouseUp(PixelScalar x, PixelScalar y)
{
  if (!dragging)
    return true;

  dragging = false;
  ReleaseCapture();

  if (!pressed)
    return true;

  SetPressed(false);
  SetState(!GetState());
  OnClicked();
  return true;
}

void
CheckBoxControl::OnSetFocus()
{
  PaintWindow::OnSetFocus();
  Invalidate();
}

void
CheckBoxControl::OnKillFocus()
{
  PaintWindow::OnKillFocus();
  Invalidate();
}

void
CheckBoxControl::OnCancelMode()
{
  dragging = false;
  SetPressed(false);

  PaintWindow::OnCancelMode();
}

void
CheckBoxControl::OnPaint(Canvas &canvas)
{
  const auto &cb_look = look->check_box;
  const bool focused = HasCursorKeys() && HasFocus();

  canvas.Select(*cb_look.font);

  if (focused)
    canvas.Clear(cb_look.focus_background_brush);
  else if (HaveClipping())
    canvas.Clear(look->background_brush);

  const auto &state_look = IsEnabled()
    ? (pressed
       ? cb_look.pressed
       : (focused
          ? cb_look.focused
          : cb_look.standard))
    : cb_look.disabled;

  unsigned box_size = canvas.GetHeight() - Layout::Scale(4);

  PixelRect rc_text_max = canvas.GetRect();
  rc_text_max.right = std::max((int)rc_text_max.left, (int)rc_text_max.right - (int)box_size);
  PixelSize text_size = text_renderer.GetSize(canvas, rc_text_max, caption.c_str());
  if (rc_text_max.GetSize().cx < text_size.cx + (int)box_size)
    box_size /= 2;

  const int margin = Layout::Scale(2);
  PixelRect rc_text = canvas.GetRect();
  switch (align_style) {
  case Right:
    rc_text.right = rc_text.right - box_size - margin;
    break;
  case Left:
  case Full:
    rc_text.right = rc_text.left + text_size.cx;
    break;
  }
  rc_text.left = std::max((int)rc_text.left, std::max(0, (int)rc_text.right - (int)text_size.cx));
  rc_text.top = (int)canvas.GetHeight() > rc_text.GetSize().cy ?
      (canvas.GetHeight() - rc_text.GetSize().cy) / 2
      : 0;
  rc_text.bottom = canvas.GetHeight();

  unsigned box_left;
  switch (align_style) {
  case Right:
  case Full:
    box_left = std::max(0, (int)canvas.GetRect().right - (int)box_size - margin);
    break;
  case Left:
  default:
    box_left = std::max(0, std::min((int)rc_text.right, (int)canvas.GetWidth() - (int)box_size));
    break;
  }

  assert(canvas.GetHeight() > box_size);
  unsigned box_top = (canvas.GetHeight() - box_size) / 2;
  canvas.Select(state_look.box_brush);
  canvas.Select(state_look.box_pen);
  canvas.Rectangle(box_left, box_top, box_left + box_size, box_top + box_size);

  if (checked) {
    canvas.Select(state_look.check_brush);
    canvas.SelectNullPen();

    RasterPoint check_mark[] = {
      {-8, -2},
      {-3, 6},
      {7, -9},
      {8, -5},
      {-3, 9},
      {-9, 2},
    };

    unsigned top = canvas.GetHeight() / 2;
    for (unsigned i = 0; i < ARRAY_SIZE(check_mark); ++i) {
      check_mark[i].x = (check_mark[i].x * (int)box_size) / 24 + box_left + box_size / 2;
      check_mark[i].y = (check_mark[i].y * (int)box_size) / 24 + top;
    }

    canvas.DrawPolygon(check_mark, ARRAY_SIZE(check_mark));
  }

  canvas.SetTextColor(state_look.text_color);
  canvas.SetBackgroundTransparent();
  text_renderer.Draw(canvas, rc_text, caption.c_str(), false);
}
