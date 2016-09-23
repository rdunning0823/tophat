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

#include "OverlayButtonWidget.hpp"
#include "MapOverlayButton.hpp"
#include "UIGlobals.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/NavSliderLook.hpp"
#include "Look/OverlayButtonLook.hpp"
#include "Interface.hpp"


OverlayButtonWidget::OverlayButtonWidget()
    : prepared(false), bmp(nullptr),
      overlay_button_look(UIGlobals::GetLook().overlay_button) {}

UPixelScalar
OverlayButtonWidget::HeightFromBottomRight()
{
  return overlay_button_look.scaled_button_width;
}

UPixelScalar
OverlayButtonWidget::GetWidth() const
{
  return overlay_button_look.scaled_button_width;
}


UPixelScalar
OverlayButtonWidget::GetHeight() const
{
  return overlay_button_look.scaled_button_width;
}

void
OverlayButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  assert(prepared == false);
  white_look.Initialise(overlay_button_look.large_font);
#ifdef WIN32
  white_look.background_transparent = false;
#else
  white_look.background_transparent = true;
#endif

  const IconLook &icon_look = CommonInterface::main_window->GetLook().icon;
  CreateButton(parent, white_look, icon_look, rc);
  Move(rc);
  prepared = true;
}

void
OverlayButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
OverlayButtonWidget::Move(const PixelRect &rc)
{
  PixelRect rc_old = GetWindow().GetPosition();
  if (rc.left != rc_old.left || rc.right != rc_old.right ||
      rc.bottom != rc_old.bottom || rc.top != rc_old.top)
    WindowWidget::Move(rc);
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

PixelRect
OverlayButtonWidget::GetPosition() const
{
  if (!prepared)
    return {0, 0, 1, 1};

  return WindowWidget::GetWindow().GetPosition();
}

MapOverlayButton &
OverlayButtonWidget::CreateButton(ContainerWindow &parent,
                                   const ButtonLook &button_look,
                                   const IconLook &icon_look,
                                   const PixelRect &rc_map)
{
  WindowStyle button_style;

  MapOverlayButton *button =
    new MapOverlayButton(parent, button_look, icon_look, overlay_button_look, bmp,
                         rc_map, button_style, *this, 0);
  SetWindow(button);
  return *button;
}
