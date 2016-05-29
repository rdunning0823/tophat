/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#if !defined(XCSOAR_DIALOGS_H)
#define XCSOAR_DIALOGS_H

#include <tchar.h>


class SingleWindow;
struct Waypoint;
class Widget;

void dlgBasicSettingsShowModal();

/**
 * Allows setting up of the critical items for Top Hat
 */
void ShowDialogSetupQuick();

/**
 * Allows setting up of the Navbar for Top Hat
 */
void ShowDialogSetupNavBar();

/**
 * displays a popup describing the "what if" task stats if the
 * wp is appended before the finish of the current MAT task
 * Allows user to either append it or cancel
 * @param wp. the waypoint to be added.
 * return true if the "More options" button was clicked
 */
bool dlgMatItemClickShowModal(const Waypoint &wp);
void dlgBrightnessShowModal();

void dlgChecklistShowModal();
void dlgConfigurationShowModal();
void dlgConfigFontsShowModal();

/**
 * shows a single configuration panel
 * Does not do preparation or leaving for settings.
 * Should be called from SystemConfiguration() to do prep work
 */
void dlgConfigurationSingle(Widget &widget, const TCHAR *caption);

/**
 * show the help screen.
 * @param conditional: if true, does not display if user has opted to not show
 * on startup
 */
void dlgStartupAssistantShowModal(bool conditional);

/**
 * shows dialog with task status info
 */
void ShowTaskStatusDialog();

void ShowWindSettingsDialog();

void dlgStatusShowModal(int page);

void dlgCreditsShowModal(SingleWindow &parent);

void dlgQuickMenuShowModal(SingleWindow &parent);

#endif
