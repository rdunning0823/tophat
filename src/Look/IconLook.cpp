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

#include "IconLook.hpp"
#include "Resources.hpp"

void
IconLook::Initialise()
{
  valid = true;
  hBmpScreensButton.LoadResource(IDB_SCREENS_BUTTON, IDB_SCREENS_BUTTON_HD, IDB_SCREENS_BUTTON_HD2);
  hBmpMenuButton.LoadResource(IDB_MENU_BUTTON, IDB_MENU_BUTTON_HD, IDB_MENU_BUTTON_HD2);
  hBmpCheckMark.LoadResource(IDB_CHECK_MARK, IDB_CHECK_MARK_HD, IDB_CHECK_MARK_HD2);
  hBmpSearch.LoadResource(IDB_SEARCH, IDB_SEARCH_HD, IDB_SEARCH_HD2);
  hBmpSearchChecked.LoadResource(IDB_SEARCH_CHECKED, IDB_SEARCH_CHECKED_HD, IDB_SEARCH_CHECKED_HD2);
  hBmpZoomOutButton.LoadResource(IDB_ZOOM_OUT_BUTTON, IDB_ZOOM_OUT_BUTTON_HD, IDB_ZOOM_OUT_BUTTON_HD2);
  hBmpZoomInButton.LoadResource(IDB_ZOOM_IN_BUTTON, IDB_ZOOM_IN_BUTTON_HD, IDB_ZOOM_IN_BUTTON_HD2);
  hBmpBearingLeftOne.LoadResource(IDB_BEARING_LEFT_ONE, IDB_BEARING_LEFT_ONE_HD, IDB_BEARING_LEFT_ONE_HD2);
  hBmpBearingLeftTwo.LoadResource(IDB_BEARING_LEFT_TWO, IDB_BEARING_LEFT_TWO_HD, IDB_BEARING_LEFT_TWO_HD2);
  hBmpBearingLeftThree.LoadResource(IDB_BEARING_LEFT_THREE, IDB_BEARING_LEFT_THREE_HD, IDB_BEARING_LEFT_THREE_HD2);
  hBmpBearingLeftFour.LoadResource(IDB_BEARING_LEFT_FOUR, IDB_BEARING_LEFT_FOUR_HD, IDB_BEARING_LEFT_FOUR_HD2);
  hBmpBearingRightOne.LoadResource(IDB_BEARING_RIGHT_ONE, IDB_BEARING_RIGHT_ONE_HD, IDB_BEARING_RIGHT_ONE_HD2);
  hBmpBearingRightTwo.LoadResource(IDB_BEARING_RIGHT_TWO, IDB_BEARING_RIGHT_TWO_HD, IDB_BEARING_RIGHT_TWO_HD2);
  hBmpBearingRightThree.LoadResource(IDB_BEARING_RIGHT_THREE, IDB_BEARING_RIGHT_THREE_HD, IDB_BEARING_RIGHT_THREE_HD2);
  hBmpBearingRightFour.LoadResource(IDB_BEARING_RIGHT_FOUR, IDB_BEARING_RIGHT_FOUR_HD, IDB_BEARING_RIGHT_FOUR_HD2);
  hBmpClose.LoadResource(IDB_CLOSE ,IDB_CLOSE_HD, IDB_CLOSE_HD2);
  icon_home.LoadResource(IDB_HOME ,IDB_HOME_HD, IDB_HOME_HD2);
  icon_backspace.LoadResource(IDB_BACKSPACE ,IDB_BACKSPACE_HD, IDB_BACKSPACE_HD2);

  hBmpTabTask.LoadResource(IDB_TASK, IDB_TASK_HD, IDB_TASK_HD2);
  hBmpTabWrench.LoadResource(IDB_WRENCH, IDB_WRENCH_HD, IDB_WRENCH_HD2);
  hBmpTabSettings.LoadResource(IDB_SETTINGS, IDB_SETTINGS_HD, IDB_SETTINGS_HD2);
  hBmpTabSettingsNavBar.LoadResource(IDB_SETTINGS_NAVBAR, IDB_SETTINGS_NAVBAR_HD, IDB_SETTINGS_NAVBAR_HD2);
  hBmpTabCalculator.LoadResource(IDB_CALCULATOR, IDB_CALCULATOR_HD, IDB_CALCULATOR_HD2);
  hBmpSpeedometer.LoadResource(IDB_SPEEDOMETER, IDB_SPEEDOMETER_HD, IDB_SPEEDOMETER_HD2);
  hBmpLayers.LoadResource(IDB_LAYERS, IDB_LAYERS_HD, IDB_LAYERS_HD2);

  hBmpTabFlight.LoadResource(IDB_GLOBE, IDB_GLOBE_HD, IDB_GLOBE_HD2);
  hBmpTabSystem.LoadResource(IDB_DEVICE, IDB_DEVICE_HD, IDB_DEVICE_HD2);
  hBmpTabRules.LoadResource(IDB_RULES, IDB_RULES_HD, IDB_RULES_HD2);
  hBmpTabTimes.LoadResource(IDB_CLOCK, IDB_CLOCK_HD, IDB_CLOCK_HD2);

  target_icon.LoadResource(IDB_TARGET, IDB_TARGET_HD, IDB_TARGET_HD2);
  task_turn_point_icon.LoadResource(IDB_TASKTURNPOINT, IDB_TASKTURNPOINT_HD, IDB_TASKTURNPOINT_HD2);
}
