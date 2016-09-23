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

#include "MainMenuButtonWidget.hpp"
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
#include "Util/StaticString.hxx"

void
MainMenuButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map,
                                       bool is_top_widget)
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
  UpdateText(0);
  CommonInterface::GetLiveBlackboard().AddListener(*this);
}

void
MainMenuButtonWidget::Unprepare()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  OverlayButtonWidget::Unprepare();
}

void
MainMenuButtonWidget::Move(const PixelRect &rc_map)
{
  PixelRect rc;
  rc.right = rc_map.right;
  rc.left = rc.right - GetWidth();
  rc.bottom = rc_map.bottom;
  rc.top = rc.bottom - GetHeight();

  OverlayButtonWidget::Move(rc);
}

void
MainMenuButtonWidget::UpdateText(unsigned page)
{
  StaticString<20> line_two;
  if (page > 0)
    line_two.Format(_T("%u / 4"), page);
  else
    line_two = _T("");

  SetLineTwoText(line_two.c_str());

  SetText(_T("M"));
}

void
MainMenuButtonWidget::OnUIStateUpdate()
{
  UpdateText(TophatMenu::GetMenuIndex());
}

void
MainMenuButtonWidget::OnAction(int id)
{
  UISettings &ui_settings = CommonInterface::SetUISettings();
  ui_settings.clear_gesture_help = true;
  TophatMenu::RotateMenu();
  UpdateText(TophatMenu::GetMenuIndex());
}
