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

#include "ZoomInButtonWidget.hpp"
#include "UIGlobals.hpp"
#include "Look/ButtonLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Input/InputEvents.hpp"
#include "MainWindow.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/Look.hpp"
#include "UIState.hpp"
#include "Look/GlobalFonts.hpp"
#include "Look/GlobalFonts.hpp"
#include "Input/InputEvents.hpp"
#include "Widgets/MapOverlayButton.hpp"
#include "Screen/Canvas.hpp"

void
ZoomInButtonWidget::Prepare(ContainerWindow &parent,
                            const PixelRect &rc)
{
  white_look.Initialise(Fonts::map_bold);
/*  white_look.SetBackgroundColor(COLOR_WHITE);
  white_look.button.standard.background_color = COLOR_WHITE;
  white_look.button.focused.background_color = COLOR_WHITE;*/

  button_size_raw.cx = button_size_raw.cy = Fonts::map_bold.GetHeight();

  CreateButton(parent, white_look, UIGlobals::GetIconLook(), rc);
  Move(rc);
}

void
ZoomInButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
ZoomInButtonWidget::Show(const PixelRect &rc)
{
  GetWindow().Show();
}

void
ZoomInButtonWidget::Hide()
{
  GetWindow().Hide();
}

void
ZoomInButtonWidget::Move(const PixelRect &rc_map)
{
  UPixelScalar clear_border_width = Layout::Scale(2);
  PixelRect rc;

  if (Layout::landscape) {
    rc.left = rc_map.left;
    rc.right = rc.left + GetWidth() + 2 * clear_border_width;
    rc.bottom = rc_map.bottom;
    rc.top = rc.bottom - GetHeight() - 2 * clear_border_width;
  } else {
    rc.left = rc_map.left;
    rc.right = rc.left + (GetWidth() + 2 * clear_border_width);
    rc.bottom = rc_map.bottom - GetHeight() - 2 * clear_border_width;
    rc.top = rc.bottom - (GetHeight() + 2 * clear_border_width);
  }

  WindowWidget::Move(rc);
}

UPixelScalar
ZoomInButtonWidget::GetWidth() const
{
  return button_size_raw.cx * MapOverlayButton::GetScale();
}

UPixelScalar
ZoomInButtonWidget::GetHeight() const
{
  return GetWidth();
}

void
ZoomInButtonWidget::OnAction(int id)
{
  InputEvents::eventZoom(_T("+"));
  InputEvents::HideMenu();
}

void
ZoomInButtonWidget::UpdateVisibility(const PixelRect &rc,
                                     bool is_panning,
                                     bool is_main_window_widget,
                                     bool is_map)
{
  if (is_map && !is_main_window_widget)
    Show(rc);
  else
    Hide();
}

UPixelScalar
ZoomInButtonWidget::HeightFromBottomLeft()
{
  if (Layout::landscape)
    return GetHeight() + Layout::Scale(3);
  else
    return GetHeight() * 2 + Layout::Scale(6);
}

ZoomButton &
ZoomInButtonWidget::CreateButton(ContainerWindow &parent,
                                 const ButtonLook &button_look,
                                 const IconLook &icon_look,
                                 const PixelRect &rc_map)
{
  ButtonWindowStyle button_style;
  button_style.multiline();

  ZoomButton *button = new ZoomButton(parent, button_look, icon_look, rc_map,
                                      button_style, true, *this, 0);
  SetWindow(button);
  return *button;
}
