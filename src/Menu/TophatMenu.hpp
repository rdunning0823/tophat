/*
 * TophatMenu.hpp
 *
 *  Created on: Jun 13, 2013
 *      Author: rob
 */

#ifndef TOPHATMENU_HPP
#define TOPHATMENU_HPP

#include "Input/InputEvents.hpp"
#include "Util/StaticString.hpp"

namespace TophatMenu {
  void RotateMenu();
  void RotateMenu()
  {
    StaticString<20> menu;
    StaticString<20> menu_1;
    StaticString<20> menu_2;
    StaticString<20> menu_last;

    menu = _T("Menu");
    menu_1 = _T("Menu1");
    menu_2 = _T("Menu2");
    menu_last = _T("MenuLast");

    if (InputEvents::IsMode(menu.buffer()))
      InputEvents::setMode(menu_1.buffer());

    else if (InputEvents::IsMode(menu_1.buffer()))
      InputEvents::setMode(menu_2.buffer());

    else if (InputEvents::IsMode(menu_2.buffer()))
      InputEvents::setMode(menu_last.buffer());

    else if (InputEvents::IsMode(menu_last.buffer()))
      InputEvents::HideMenu();

    else InputEvents::setMode(menu.buffer());

    return;
  }

}
#endif /* TOPHATMENU_HPP_ */
