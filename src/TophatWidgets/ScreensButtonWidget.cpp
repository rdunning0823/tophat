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

#include "ScreensButtonWidget.hpp"
#include "TophatWidgets/MapOverlayButton.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Screen/Canvas.hpp"
#include "Interface.hpp"
#include "Input/InputEvents.hpp"
#include "InfoBoxes/InfoBoxSettings.hpp"
#include "PageSettings.hpp"
#include "PageState.hpp"
#include "PageActions.hpp"
#include "UIState.hpp"
#include "Language/Language.hpp"

ScreensButtonWidget::ButtonPosition
ScreensButtonWidget::GetButtonPosition(InfoBoxSettings::Geometry geometry,
                                       bool landscape)
{
  if (landscape)
    switch (geometry) {
    case InfoBoxSettings::Geometry::SPLIT_8:
    case InfoBoxSettings::Geometry::LEFT_6_RIGHT_3_VARIO:
    case InfoBoxSettings::Geometry::TOP_8_VARIO:
    case InfoBoxSettings::Geometry::OBSOLETE_SPLIT_8:
        return ScreensButtonWidget::ButtonPosition::Bottom;

    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_8:
    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_10:
    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_12:
    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_4:
    case InfoBoxSettings::Geometry::BOTTOM_8_VARIO:
    case InfoBoxSettings::Geometry::RIGHT_5:
    case InfoBoxSettings::Geometry::TOP_RIGHT_7:
    case InfoBoxSettings::Geometry::RIGHT_16:
    case InfoBoxSettings::Geometry::RIGHT_24:
    case InfoBoxSettings::Geometry::RIGHT_9_VARIO:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_8:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_12:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_4:
      return ScreensButtonWidget::ButtonPosition::Left;

    case InfoBoxSettings::Geometry::TOP_LEFT_8:
    case InfoBoxSettings::Geometry::TOP_LEFT_12:
    case InfoBoxSettings::Geometry::TOP_LEFT_4:
    case InfoBoxSettings::Geometry::OBSOLETE_TOP_LEFT_8:
    case InfoBoxSettings::Geometry::OBSOLETE_TOP_LEFT_4:
      return ScreensButtonWidget::ButtonPosition::Right;
    }

  return ScreensButtonWidget::ButtonPosition::Right;
}

void
ScreensButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map,
                                       bool is_top_widget)
{
  if ((!is_panning &&
      CommonInterface::GetUISettings().screens_button_location ==
          UISettings::ScreensButtonLocation::MAP &&
          CommonInterface::SetUISettings().pages.n_pages > 1)
      || (CommonInterface::GetUIState().pages.special_page.IsDefined() && !is_panning)) {
    Show(rc);
  } else {
    Hide();
  }
}

void
ScreensButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  OverlayButtonWidget::Prepare(parent, rc);
  UpdateText();
  CommonInterface::GetLiveBlackboard().AddListener(*this);
}

void
ScreensButtonWidget::Unprepare()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  OverlayButtonWidget::Unprepare();
}

void
ScreensButtonWidget::Move(const PixelRect &rc_map)
{
  const UISettings &ui_settings = CommonInterface::GetUISettings();
  const PixelRect rc_main = UIGlobals::GetMainWindow().GetClientRect();

  PixelRect rc;

  button_position = GetButtonPosition(ui_settings.info_boxes.geometry,
                                      Layout::landscape);
  // stretch to right and bottom if adjacent to another button
  const unsigned pen_width = Layout::Scale(1);

  switch (button_position) {
  case ButtonPosition::Left:
    rc.left = 0;
    rc.right = rc.left + GetWidth();
    rc.bottom = rc_main.GetCenter().y;
    rc.top = rc.bottom - GetHeight();
  break;

  case ButtonPosition::Right:
    rc.right = rc_main.right;
    rc.left = rc.right - GetWidth();
    rc.bottom = rc_main.GetCenter().y;
    rc.top = rc.bottom - GetHeight();
  break;

  case ButtonPosition::Bottom:
    rc.left = rc_main.GetCenter().x - GetWidth() / 2 - pen_width;
    rc.right = rc.left + GetWidth();
    rc.bottom = rc_main.bottom;
    rc.top = rc.bottom - GetHeight();
  break;
}
  OverlayButtonWidget::Move(rc);
}

void
ScreensButtonWidget::UpdateText()
{
  const PagesState &state = CommonInterface::GetUIState().pages;
  TCHAR line_two[50];

  if (state.special_page.IsDefined()) {
    SetLineTwoText(_("Close"));
  } else {
    const PageLayout *pl =
        &CommonInterface::SetUISettings().pages.pages[state.current_index];
    const InfoBoxSettings &info_box_settings =
      CommonInterface::GetUISettings().info_boxes;

    assert(pl != NULL);

    pl->MakeInfoBoxSetTitle(info_box_settings, line_two);
    SetLineTwoText(line_two);
  }
  SetText(_T("S"));
}

void
ScreensButtonWidget::OnUIStateUpdate()
{
  UpdateText();
}

void
ScreensButtonWidget::OnAction(int id)
{
  InputEvents::HideMenu();
  InputEvents::eventScreenModes(_T("next"));
  UpdateText();
}
