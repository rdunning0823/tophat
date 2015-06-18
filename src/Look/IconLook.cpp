/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "Screen/Layout.hpp"
#include "Resources.hpp"

void
IconLook::Initialise()
{
  int s = Layout::scale;
  hBmpTabTask.Load(GET_ICON(s, IDB_TASK_HD2 ,IDB_TASK_HD, IDB_TASK));
  hBmpTabWrench.Load(GET_ICON(s, IDB_WRENCH_HD2 ,IDB_WRENCH_HD, IDB_WRENCH));
  hBmpTabSettings.Load(GET_ICON(s, IDB_SETTINGS_HD2 ,IDB_SETTINGS_HD, IDB_SETTINGS));
  hBmpTabCalculator.Load(GET_ICON(s, IDB_CALCULATOR_HD ,IDB_CALCULATOR_HD, IDB_CALCULATOR));

  hBmpTabFlight.Load(GET_ICON(s, IDB_GLOBE_HD2 ,IDB_GLOBE_HD, IDB_GLOBE));
  hBmpTabSystem.Load(GET_ICON(s, IDB_DEVICE_HD2 ,IDB_DEVICE_HD, IDB_DEVICE));
  hBmpTabRules.Load(GET_ICON(s, IDB_RULES_HD2 ,IDB_RULES_HD, IDB_RULES));
  hBmpTabTimes.Load(GET_ICON(s, IDB_CLOCK_HD2 ,IDB_CLOCK_HD, IDB_CLOCK));


  hBmpScreensButton.Load(GET_ICON(s, IDB_SCREENS_BUTTON_HD2 ,IDB_SCREENS_BUTTON_HD, IDB_SCREENS_BUTTON));
  hBmpMenuButton.Load(GET_ICON(s, IDB_MENU_BUTTON_HD2 ,IDB_MENU_BUTTON_HD, IDB_MENU_BUTTON));
  hBmpCheckMark.Load(GET_ICON(s, IDB_CHECK_MARK_HD2 ,IDB_CHECK_MARK_HD, IDB_CHECK_MARK));
  hBmpSearch.Load(GET_ICON(s, IDB_SEARCH_HD2, IDB_SEARCH_HD, IDB_SEARCH));
  hBmpSearchChecked.Load(GET_ICON(s, IDB_SEARCH_CHECKED_HD2 ,IDB_SEARCH_CHECKED_HD, IDB_SEARCH_CHECKED));
  hBmpZoomOutButton.Load(GET_ICON(s, IDB_ZOOM_OUT_BUTTON_HD2 ,IDB_ZOOM_OUT_BUTTON_HD, IDB_ZOOM_OUT_BUTTON));
  hBmpZoomInButton.Load(GET_ICON(s, IDB_ZOOM_IN_BUTTON_HD2 ,IDB_ZOOM_IN_BUTTON_HD, IDB_ZOOM_IN_BUTTON));
  hBmpBearingLeftOne.Load(GET_ICON(s, IDB_BEARING_LEFT_ONE_HD2 ,IDB_BEARING_LEFT_ONE_HD, IDB_BEARING_LEFT_ONE));
  hBmpBearingLeftTwo.Load(GET_ICON(s, IDB_BEARING_LEFT_TWO_HD2 ,IDB_BEARING_LEFT_TWO_HD, IDB_BEARING_LEFT_TWO));
  hBmpBearingLeftThree.Load(GET_ICON(s, IDB_BEARING_LEFT_THREE_HD2 ,IDB_BEARING_LEFT_THREE_HD, IDB_BEARING_LEFT_THREE));
  hBmpBearingLeftFour.Load(GET_ICON(s, IDB_BEARING_LEFT_FOUR_HD2 ,IDB_BEARING_LEFT_FOUR_HD, IDB_BEARING_LEFT_FOUR));
  hBmpBearingRightOne.Load(GET_ICON(s, IDB_BEARING_RIGHT_ONE_HD2 ,IDB_BEARING_RIGHT_ONE_HD, IDB_BEARING_RIGHT_ONE));
  hBmpBearingRightTwo.Load(GET_ICON(s, IDB_BEARING_RIGHT_TWO_HD2 ,IDB_BEARING_RIGHT_TWO_HD, IDB_BEARING_RIGHT_TWO));
  hBmpBearingRightThree.Load(GET_ICON(s, IDB_BEARING_RIGHT_THREE_HD2 ,IDB_BEARING_RIGHT_THREE_HD, IDB_BEARING_RIGHT_THREE));
  hBmpBearingRightFour.Load(GET_ICON(s, IDB_BEARING_RIGHT_FOUR_HD2 ,IDB_BEARING_RIGHT_FOUR_HD, IDB_BEARING_RIGHT_FOUR));
  hBmpClose.Load(GET_ICON(s, IDB_CLOSE_HD2 ,IDB_CLOSE_HD, IDB_CLOSE));
}
