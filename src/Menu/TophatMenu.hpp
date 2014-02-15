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
  void RotateMenu();
  void RotateMenu()
  {
    if (InputEvents::IsMode(menu))
      InputEvents::setMode(menu_1);

    else if (InputEvents::IsMode(menu_1))
      InputEvents::setMode(menu_2);

    else if (InputEvents::IsMode(menu_2))
      InputEvents::setMode(menu_last);

    else if (InputEvents::IsMode(menu_last))
      InputEvents::HideMenu();

    else InputEvents::setMode(menu);

    return;
  }

}
#endif /* TOPHATMENU_HPP_ */
