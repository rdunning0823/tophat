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

#include "Form/Button.hpp"
#include "Form/ActionListener.hpp"
#include "Look/ButtonLook.hpp"
#include "Screen/Key.h"
#include "Asset.hpp"
#include "Hardware/Vibrator.hpp"

WndSymbolButton::WndSymbolButton(ContainerWindow &parent,
                                 const ButtonLook &_look,
                                 const TCHAR *_caption,
                                 const PixelRect &rc,
                                 WindowStyle style,
                                 ActionListener &_listener,
                                 int _id)
{
  Button::Create(parent, rc, style, new SymbolButtonRenderer(_look,_caption),
                 _listener, _id);
}

void
WndSymbolButton::Create(ContainerWindow &parent,
                        const ButtonLook &_look,
                        const TCHAR *_caption,
                        const PixelRect &rc,
                        WindowStyle style,
                        ActionListener &_listener,
                        int _id)
{
  Button::Create(parent, rc, style, new SymbolButtonRenderer(_look,_caption),
                 _listener, _id);
}

void
WndSymbolButton::SetCaption(const TCHAR *caption)
{
  assert(caption != nullptr);

  SymbolButtonRenderer &r = (SymbolButtonRenderer&)GetRenderer();
  r.SetCaption(caption);

  Invalidate();
}

void
WndSymbolButton::SetPrefixIcon(SymbolButtonRenderer::PrefixIcon type)
{
  ((SymbolButtonRenderer&)GetRenderer()).SetPrefixIcon(type);
  Invalidate();
}

Button::~Button() {
  /* we must override ~Window(), because in ~Window(), our own
     OnDestroy() method won't be called (during object destruction,
     this object loses its identity) */
  Destroy();
}

void
Button::Create(ContainerWindow &parent,
               const PixelRect &rc,
               WindowStyle style,
               ButtonRenderer *_renderer)
{
  dragging = down = selected = false;
  renderer = _renderer;

  PaintWindow::Create(parent, rc, style);
}

void
Button::Create(ContainerWindow &parent, const ButtonLook &look,
               const TCHAR *caption, const PixelRect &rc,
               WindowStyle style, bool use_large_font)
{
  Create(parent, rc, style,
         new TextButtonRenderer(look, caption, use_large_font));
}

void
Button::Create(ContainerWindow &parent, const PixelRect &rc,
               WindowStyle style, ButtonRenderer *_renderer,
               ActionListener &_listener, int _id)
{
  listener = &_listener;
  id = _id;

  Create(parent, rc, style, _renderer);
}

void
Button::Create(ContainerWindow &parent, const ButtonLook &look,
               const TCHAR *caption, const PixelRect &rc,
               WindowStyle style,
               ActionListener &_listener, int _id) {
  Create(parent, rc, style,
         new TextButtonRenderer(look, caption),
         _listener, _id);
}

void
Button::SetCaption(const TCHAR *caption)
{
  assert(caption != nullptr);

  TextButtonRenderer &r = *(TextButtonRenderer *)renderer;
  r.SetCaption(caption);

  Invalidate();
}

StaticString<96>::const_pointer
Button::GetCaption()
{
  TextButtonRenderer &r = *(TextButtonRenderer *)renderer;
  return r.GetCaption();
}

void
Button::SetSelected(bool _selected)
{
  if (_selected == selected)
    return;

  selected = _selected;
  Invalidate();
}

unsigned
Button::GetMinimumWidth() const
{
  return renderer->GetMinimumButtonWidth();
}

void
Button::SetDown(bool _down)
{
  if (_down == down)
    return;

#ifdef HAVE_VIBRATOR
  VibrateShort();
#endif

  down = _down;
  Invalidate();
}

bool
Button::OnClicked()
{
  if (listener != nullptr) {
    listener->OnAction(id);
    return true;
  }

  return false;
}

void
Button::Click()
{
  SetDown(false);
  OnClicked();
}

void
Button::OnDestroy()
{
  assert(renderer != nullptr);

  delete renderer;

  PaintWindow::OnDestroy();
}

bool
Button::OnKeyCheck(unsigned key_code) const
{
  switch (key_code) {
  case KEY_RETURN:
    return true;

  default:
    return PaintWindow::OnKeyCheck(key_code);
  }
}

bool
Button::OnKeyDown(unsigned key_code)
{
  switch (key_code) {
#ifdef GNAV
  case VK_F4:
    // using F16 also as Enter-Key. This allows to use the RemoteStick of Altair to do a "click" on the focused button
  case VK_F16:
#endif
  case KEY_RETURN:
  case KEY_SPACE:
    Click();
    return true;

  default:
    return PaintWindow::OnKeyDown(key_code);
  }
}

bool
Button::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (dragging) {
    SetDown(IsInside(x, y));
    return true;
  } else
    return PaintWindow::OnMouseMove(x, y, keys);
}

bool
Button::OnMouseDown(PixelScalar x, PixelScalar y)
{
  if (IsTabStop())
    SetFocus();

  SetDown(true);
  SetCapture();
  dragging = true;
  return true;
}

bool
Button::OnMouseUp(PixelScalar x, PixelScalar y)
{
  if (!dragging)
    return true;

  dragging = false;
  ReleaseCapture();

  if (!down)
    return true;

  Click();
  return true;
}

void
Button::OnSetFocus()
{
  PaintWindow::OnSetFocus();
  Invalidate();
}

void
Button::OnKillFocus()
{
  PaintWindow::OnKillFocus();
  Invalidate();
}

void
Button::OnCancelMode()
{
  dragging = false;
  SetDown(false);

  PaintWindow::OnCancelMode();
}

void
Button::OnPaint(Canvas &canvas)
{
  assert(renderer != nullptr);

  const bool pressed = down;
  const bool focused = HasCursorKeys()
    ? HasFocus() || (selected && !HasPointer())
    : pressed;

  renderer->DrawButton(canvas, GetClientRect(),
                       IsEnabled(), focused, pressed,
                       transparent_background_force);
}
