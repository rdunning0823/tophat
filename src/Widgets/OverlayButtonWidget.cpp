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

#include "OverlayButtonWidget.hpp"
#include "MapOverlayButton.hpp"
#include "Form/SymbolButton.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Canvas.hpp"
#include "Interface.hpp"

bool
OverlayButton::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (IsInside(x, y))
    return WndButton::OnMouseMove(x, y, keys);
  else
    OnCancelMode();
  return true;
}

void
OverlayButton::OnPaint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.GetWidth()),
    PixelScalar(canvas.GetHeight())
  };

  bool pressed = IsDown();
#ifdef ENABLE_OPENGL
  bool transparent = true;
#else
  bool transparent = false;
#endif
  //Todo fix the GDI rendering so it draws transparent correctly
  renderer.DrawButton(canvas, rc, HasFocus(), pressed, transparent);
  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SelectNullPen();
  if (!IsEnabled())
    canvas.Select(button_look.disabled.brush);
  else
    canvas.Select(button_look.standard.foreground_brush);
  const PixelSize bitmap_size = bmp->GetSize();
  const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
  const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
  canvas.CopyAnd(rc.left + offsetx,
                  rc.top + offsety,
                  bitmap_size.cx / 2,
                  bitmap_size.cy,
                  *bmp,
                  bitmap_size.cx / 2, 0);
}

UPixelScalar
OverlayButtonWidget::HeightFromBottomRight()
{
  return bitmap_size_raw.cy * MapOverlayButton::GetScale() + Layout::Scale(3);
}

void
OverlayButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  const IconLook &icon_look = CommonInterface::main_window->GetLook().icon;
  white_look.Initialise(Fonts::map_bold);
  bitmap_size_raw = bmp->GetSize();

  CreateButton(parent, white_look, icon_look, rc);
  Move(rc);
}

void
OverlayButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
OverlayButtonWidget::Show(const PixelRect &rc)
{
  GetWindow().Show();
}

void
OverlayButtonWidget::Hide()
{
  GetWindow().Hide();
}

UPixelScalar
OverlayButtonWidget::GetWidth() const
{
  return bitmap_size_raw.cx / 2 * MapOverlayButton::GetScale() / 2.5;
}

UPixelScalar
OverlayButtonWidget::GetHeight() const
{
  return bitmap_size_raw.cy * MapOverlayButton::GetScale() / 2.5;
}

OverlayButton &
OverlayButtonWidget::CreateButton(ContainerWindow &parent,
                                   const ButtonLook &button_look,
                                   const IconLook &icon_look,
                                   const PixelRect &rc_map)
{
  ButtonWindowStyle button_style;
  button_style.multiline();

  OverlayButton *button =
    new OverlayButton(parent, button_look, icon_look, bmp,
                      rc_map, button_style, *this, 0);
  SetWindow(button);
  return *button;
}


