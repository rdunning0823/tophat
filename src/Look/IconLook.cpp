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

#include "resource.h"

void
IconLook::Initialise()
{
  hBmpTabTask.Load(Layout::scale > 1 ? IDB_TASK_HD : IDB_TASK);
  hBmpTabWrench.Load(Layout::scale > 1 ? IDB_WRENCH_HD : IDB_WRENCH);
  hBmpTabSettings.Load(Layout::scale > 1 ? IDB_SETTINGS_HD : IDB_SETTINGS);
  hBmpTabCalculator.Load(Layout::scale > 1 ? IDB_CALCULATOR_HD : IDB_CALCULATOR);

  hBmpTabFlight.Load(Layout::scale > 1 ? IDB_GLOBE_HD : IDB_GLOBE);
  hBmpTabSystem.Load(Layout::scale > 1 ? IDB_DEVICE_HD : IDB_DEVICE);
  hBmpTabRules.Load(Layout::scale > 1 ? IDB_RULES_HD : IDB_RULES);
  hBmpTabTimes.Load(Layout::scale > 1 ? IDB_CLOCK_HD : IDB_CLOCK);


  hBmpScreensButton.Load(Layout::scale > 1 ? IDB_SCREENS_BUTTON_HD : IDB_SCREENS_BUTTON);
  hBmpMenuButton.Load(Layout::scale > 1 ? IDB_MENU_BUTTON_HD : IDB_MENU_BUTTON);
  hBmpCheckMark.Load(Layout::scale > 1 ? IDB_CHECK_MARK_HD : IDB_CHECK_MARK);
  hBmpSearch.Load(Layout::scale > 1 ? IDB_SEARCH_HD : IDB_SEARCH);
  hBmpSearchChecked.Load(Layout::scale > 1 ? IDB_SEARCH_CHECKED_HD : IDB_SEARCH_CHECKED);
  hBmpZoomOutButton.Load(Layout::scale > 1 ? IDB_ZOOM_OUT_BUTTON_HD : IDB_ZOOM_OUT_BUTTON);
  hBmpZoomInButton.Load(Layout::scale > 1 ? IDB_ZOOM_IN_BUTTON_HD : IDB_ZOOM_IN_BUTTON);
  hBmpBearingLeftOne.Load(Layout::scale > 1 ? IDB_BEARING_LEFT_ONE_HD : IDB_BEARING_LEFT_ONE);
  hBmpBearingLeftTwo.Load(Layout::scale > 1 ? IDB_BEARING_LEFT_TWO_HD : IDB_BEARING_LEFT_TWO);
  hBmpBearingLeftThree.Load(Layout::scale > 1 ? IDB_BEARING_LEFT_THREE_HD : IDB_BEARING_LEFT_THREE);
  hBmpBearingLeftFour.Load(Layout::scale > 1 ? IDB_BEARING_LEFT_FOUR_HD : IDB_BEARING_LEFT_FOUR);
  hBmpBearingRightOne.Load(Layout::scale > 1 ? IDB_BEARING_RIGHT_ONE_HD : IDB_BEARING_RIGHT_ONE);
  hBmpBearingRightTwo.Load(Layout::scale > 1 ? IDB_BEARING_RIGHT_TWO_HD : IDB_BEARING_RIGHT_TWO);
  hBmpBearingRightThree.Load(Layout::scale > 1 ? IDB_BEARING_RIGHT_THREE_HD : IDB_BEARING_RIGHT_THREE);
  hBmpBearingRightFour.Load(Layout::scale > 1 ? IDB_BEARING_RIGHT_FOUR_HD : IDB_BEARING_RIGHT_FOUR);
}
