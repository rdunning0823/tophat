/*
 * TophatMenu.hpp
 *
 *  Created on: Jun 13, 2013
 *      Author: rob
 */

#ifndef TOPHATMENU_HPP
#define TOPHATMENU_HPP

#include <tchar.h>

namespace TophatMenu {

  const TCHAR menu[] = _T("Menu");
  const TCHAR menu_alt_with_screens_button[] = _T("MenuWithScreens");
  const TCHAR menu_1[] = _T("Menu1");
  const TCHAR menu_2[] = _T("Menu2");
  const TCHAR menu_last[] = _T("MenuLast");

  void ShowMenu(const TCHAR *menu_name);
  void RotateMenu();
}
#endif /* TOPHATMENU_HPP_ */
