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

#include "MainMenuButtonWidget.hpp"
#include "Form/SymbolButton.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Menu/TophatMenu.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Canvas.hpp"
#include "Interface.hpp"

void
MainMenuButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map)
{
  if (!is_panning)
    Show(rc);
  else
    Hide();
}

void
MainMenuButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  OverlayButtonWidget::Prepare(parent, rc);
  SetText(_T("M"));
}

void
MainMenuButtonWidget::Move(const PixelRect &rc_map)
{
  PixelRect rc;
  rc.right = rc_map.right;
  rc.left = rc.right - GetWidth();
  rc.bottom = rc_map.bottom;
  rc.top = rc.bottom - GetHeight();

  WindowWidget::Move(rc);
  GetWindow().Move(rc);
}

void
MainMenuButtonWidget::OnAction(int id)
{
  UISettings &ui_settings = CommonInterface::SetUISettings();
  ui_settings.clear_gesture_help = true;
  TophatMenu::RotateMenu();
}
