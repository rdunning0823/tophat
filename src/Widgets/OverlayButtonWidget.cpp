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
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Interface.hpp"


UPixelScalar
OverlayButtonWidget::HeightFromBottomRight()
{
  return GetHeight();
}

void
OverlayButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  assert(prepared == false);
  white_look.Initialise(Fonts::map_bold);
  white_look.font = &Fonts::map_overlay_button;

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
  return MapOverlayButton::GetStandardButtonHeight() *
      MapOverlayButton::GetScale() + MapOverlayButton::GetClearBorderWidth();
}

UPixelScalar
OverlayButtonWidget::GetHeight() const
{
  return MapOverlayButton::GetStandardButtonHeight() *
      MapOverlayButton::GetScale() + MapOverlayButton::GetClearBorderWidth();
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
  ButtonWindowStyle button_style;
  button_style.multiline();
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  MapOverlayButton *button =
    new MapOverlayButton(parent, button_look, icon_look, dialog_look, bmp,
                         rc_map, button_style, *this, 0);
  SetWindow(button);
  return *button;
}
