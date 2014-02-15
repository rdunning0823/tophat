/*
 * TophatMenu.hpp
 *
 *  Created on: Jun 13, 2013
 *      Author: rob
 */

#ifndef TOPHATMENU_HPP
#define TOPHATMENU_HPP

#include "Input/InputEvents.hpp"

namespace TophatMenu {

  const TCHAR menu[] = _T("Menu");
  const TCHAR menu_1[] = _T("Menu1");
  const TCHAR menu_2[] = _T("Menu2");
  const TCHAR menu_last[] = _T("MenuLast");

  void ShowMenu(const TCHAR *menu_name);
  void ShowMenu(const TCHAR *menu_name)
  {
    InputEvents::ShowMenu();
    InputEvents::setMode(menu_name);
  }

  void RotateMenu();
  void RotateMenu()
  {
    if (InputEvents::IsMode(menu))
      ShowMenu(menu_1);

    else if (InputEvents::IsMode(menu_1))
      ShowMenu(menu_2);

    else if (InputEvents::IsMode(menu_2))
      ShowMenu(menu_last);

    else if (InputEvents::IsMode(menu_last))
      InputEvents::HideMenu();

    else ShowMenu(menu);

    return;
  }

}
#endif /* TOPHATMENU_HPP_ */
