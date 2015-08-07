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

#include "TophatMenu.hpp"
#include "Input/InputEvents.hpp"
#include "ActionInterface.hpp"
#include "UIState.hpp"
#include "Util/StaticString.hpp"
#include "Engine/Task/TaskType.hpp"
#include "NMEA/Derived.hpp"
#include "Task/Stats/CommonStats.hpp"

namespace TophatMenu {

  unsigned GetMenuIndex()
  {
    return CommonInterface::GetUIState().main_menu_index;
  }

  static void ShowMenu(const TCHAR *menu_name)
  {
    InputEvents::ShowMenu();
    InputEvents::setMode(menu_name);
  }

  void RotateMenu()
  {
    int new_menu = 0;
    switch (GetMenuIndex()) {
    case 0:
      ShowMenu(menu);
      new_menu = 1;
      break;
    case 1:
      ShowMenu(menu_1);
      new_menu = 2;
      break;
    case 2:
      ShowMenu(menu_2);
      new_menu = 3;
      break;
    case 3:
      ShowMenu(menu_last);
      new_menu = 4;
      break;
    case 4:
      InputEvents::HideMenu();
      new_menu = 0;
      break;
    }

    CommonInterface::SetUIState().main_menu_index = new_menu;
  }

  void
  RotateNavMenu()
  {
    StaticString<20> menu_ordered;
    StaticString<20> menu_goto;

    menu_ordered = _T("NavOrdered");
    menu_goto = _T("NavGoto");
    TaskType mode = XCSoarInterface::Calculated().common_stats.task_type;

    if (InputEvents::IsMode(menu_ordered.buffer())
        || InputEvents::IsMode(menu_goto.buffer()))
      InputEvents::HideMenu();
    else if (mode == TaskType::GOTO || mode == TaskType::ABORT) {
      InputEvents::ShowMenu();
      InputEvents::setMode(menu_goto.buffer());
    } else {
      InputEvents::ShowMenu();
      InputEvents::setMode(menu_ordered.buffer());
    }

    return;
  }
}
