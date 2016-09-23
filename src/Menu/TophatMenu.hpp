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

#ifndef TOPHATMENU_HPP
#define TOPHATMENU_HPP

#include <tchar.h>

namespace TophatMenu {

  const TCHAR menu[] = _T("Menu");
  const TCHAR menu_1[] = _T("Menu1");
  const TCHAR menu_2[] = _T("Menu2");
  const TCHAR menu_last[] = _T("MenuLast");

  /**
   * Rotates the main "M" menu between menu, menu_1, menu_2, menu_last, none
   */
  void RotateMenu();

  /**
   * @return number of menu displayed (1 to 4).  0 if none is displayed
   */
  unsigned GetMenuIndex();

  /**
   * Shows or hides the Nav menu
   * Either Goto, Teammate or Ordered, whichever is current
   */
  void RotateNavMenu();
}
#endif /* TOPHATMENU_HPP_ */
