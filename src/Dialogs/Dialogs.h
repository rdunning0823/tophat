/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
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
class Waypoint;

void dlgAlternatesListShowModal(SingleWindow &parent);

void dlgBasicSettingsShowModal();

/**
 * show the help screen.
 * @param conditional: if true, does not display if user has opted to not show
 * on startup.  if false, skips the configuration screen
 */
void dlgStartupAssistantShowModal(bool conditional);

/**
 * Allows setting up of the critical items for Top Hat
 * @param auto_prompt.  do we explain that there is missing data?
 */
void ShowDialogSetupQuick(bool auto_prompt);

/**
 * displays a popup describing the "what if" task stats if the
 * wp is appended before the finish of the current MAT task
 * Allows user to either append it or cancel
 * @param wp. the waypoint to be added.
 * return true if the "More options" button was clicked
 */
bool dlgMatItemClickShowModal(const Waypoint &wp);
void dlgQNHShowModal();
void dlgBrightnessShowModal();

void dlgChecklistShowModal();
void dlgConfigurationShowModal(const TCHAR *page_name = _T(""));
void dlgConfigFontsShowModal();

/**
 * @return true on success, false if the user has pressed the "Quit"
 * button
 */
bool
dlgStartupShowModal();

/**
 * shows dialog with task status info
 */
void ShowTaskStatusDialog();

void ShowWindSettingsDialog();

void dlgStatusShowModal(int page);

void dlgSwitchesShowModal();

void dlgCreditsShowModal(SingleWindow &parent);

void dlgQuickMenuShowModal(SingleWindow &parent);

#endif
