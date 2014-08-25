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
#include "Interface.hpp"

namespace TophatMenu {

  void ShowMenu(const TCHAR *menu_name)
  {
    InputEvents::ShowMenu();
    InputEvents::setMode(menu_name);
  }

  void RotateMenu()
  {
    if (InputEvents::IsMode(menu) || InputEvents::IsMode(menu_alt_with_screens_button))
      ShowMenu(menu_1);

    else if (InputEvents::IsMode(menu_1))
      ShowMenu(menu_2);

    else if (InputEvents::IsMode(menu_2))
      ShowMenu(menu_last);

    else if (InputEvents::IsMode(menu_last))
      InputEvents::HideMenu();

    else
      ShowMenu(menu);

    return;
  }
}
