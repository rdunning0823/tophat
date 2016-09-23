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

#include "ReplayButtonWidget.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "UISettings.hpp"
#include "Dialogs/ReplayDialog.hpp"
#include "Replay/Replay.hpp"
#include "Components.hpp"
#include "Input/InputEvents.hpp"

void
ReplayButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map,
                                       bool is_top_widget)
{
  const UISettings &ui_settings = CommonInterface::GetUISettings();

  if (is_map && !is_main_window_widget && !is_top_widget &&
      replay != nullptr &&
      replay->IsActive() && !ui_settings.replay_dialog_visible)
    Show(rc);
  else
    Hide();
}

void
ReplayButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  OverlayButtonWidget::Prepare(parent, rc);
  UpdateText(_("Replay"));
}

void
ReplayButtonWidget::Move(const PixelRect &rc_map)
{
  if (!prepared)
    return;

  // make button wider and not as tall as other buttons
  int width = GetWidth() * 2;
  int height = (GetHeight() * 2) / 3;

  PixelRect rc;
  rc.left = rc_map.GetCenter().x - width / 2;
  rc.right = rc.left + width;
  rc.top = rc_map.top + offset_top;
  rc.bottom = rc.top + height;

  OverlayButtonWidget::Move(rc);
}

void
ReplayButtonWidget::UpdateText(const TCHAR *text)
{
  SetText(text);
}

void
ReplayButtonWidget::OnAction(int id)
{
  InputEvents::HideMenu();
  ShowReplayDialog();
}
