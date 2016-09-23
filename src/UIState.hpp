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

#ifndef XCSOAR_UI_STATE_HPP
#define XCSOAR_UI_STATE_HPP

#include "DisplayMode.hpp"
#include "Util/StaticString.hxx"
#include "PageState.hpp"
#include "Weather/WeatherUIState.hpp"

/**
 * The state of the user interface.
 */
struct UIState {
  /**
   * Is the display currently blanked?
   */
  bool screen_blanked;

  /**
   * The display mode forced by the user.  If not NONE, it overrides
   * the automatic display mode.
   */
  DisplayMode force_display_mode;

  /**
   * The effective display mode.
   */
  DisplayMode display_mode;

  /**
   * Are the info boxes showing an "auxiliary" set?
   */
  bool auxiliary_enabled;

  /**
   * Which "auxiliary" set is visible if #auxiliary_enabled is true?
   */
  unsigned auxiliary_index;

  /**
   * The index of the InfoBox panel currently being displayed.  If
   * #auxiliary_enabled is true, then this is the same as
   * #auxiliary_index.
   */
  unsigned panel_index;

  /**
   * A copy of the current InfoBox panel name.  This copy is necessary
   * because the original name is in InfoBoxSettings, but MapWindow
   * does not know InfoBoxSettings or UISettings, and needs to know
   * the name for rendering the overlay.
   */
  StaticString<32u> panel_name;

  PagesState pages;

  /**
   * Index, 0-4 of Top Hat main menu button index.  0 means hidden
   */
  unsigned main_menu_index;
  WeatherUIState weather;

  void Clear();
};

#endif
