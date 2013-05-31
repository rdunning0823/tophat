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

#include "Dialogs/Dialogs.h"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/TabBar.hpp"
#include "Form/ActionWidget.hpp"
#include "UIGlobals.hpp"
#include "Look/IconLook.hpp"
#include "StatusPanels/FlightStatusPanel.hpp"
#include "StatusPanels/RulesStatusPanel.hpp"
#include "StatusPanels/SystemStatusPanel.hpp"
#include "StatusPanels/TimesStatusPanel.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Protection.hpp"
#include "Hardware/Battery.hpp"
#include "Formatter/Units.hpp"
#include "Logger/Logger.hpp"
#include "Math/FastMath.h"
#include "LocalTime.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Compiler.h"

#include <assert.h>
#include <stdio.h>

#include <algorithm>

class WndButton;

static WndForm *wf = NULL;
static TabBarControl *wTabBar;
static int status_page = 0;

static void
SetTitle()
{
  StaticString<128> title;
  title.Format(_T("%s: %s"), _("Status"),
               wTabBar->GetButtonCaption((wTabBar->GetCurrentPage())));
  wf->SetCaption(title);
}

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgStatusShowModal(int start_page)
{
  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ?
                  _T("IDR_XML_STATUS_L") : _T("IDR_XML_STATUS"));
  assert(wf);

  wTabBar = ((TabBarControl *)wf->FindByName(_T("TabBar")));
  assert(wTabBar != NULL);
  wTabBar->SetPageFlippedCallback(SetTitle);

  const NMEAInfo &basic = CommonInterface::Basic();
  const Waypoint *nearest_waypoint = basic.location_available
    ? way_points.GetNearest(CommonInterface::Basic().location, fixed(100000))
    : NULL;

  /* setup tabs */

  const bool enable_icons =
    CommonInterface::GetUISettings().dialog.tab_style
    == DialogSettings::TabStyle::Icon;

  const DialogLook &look = UIGlobals::GetDialogLook();
  const IconLook &icons = UIGlobals::GetIconLook();
  const Bitmap *FlightIcon = enable_icons ? &icons.hBmpTabFlight : NULL;
  const Bitmap *SystemIcon = enable_icons ? &icons.hBmpTabSystem : NULL;
  const Bitmap *RulesIcon = enable_icons ? &icons.hBmpTabRules : NULL;
  const Bitmap *TimesIcon = enable_icons ? &icons.hBmpTabTimes : NULL;

  Widget *flight_panel = new FlightStatusPanel(look, nearest_waypoint);
  wTabBar->AddTab(flight_panel, _T("Flight"), FlightIcon);

  Widget *system_panel = new SystemStatusPanel(look);
  wTabBar->AddTab(system_panel, _T("System"), SystemIcon);

  Widget *rules_panel = new RulesStatusPanel(look);
  wTabBar->AddTab(rules_panel, _T("Task Start"), RulesIcon);

  Widget *times_panel = new TimesStatusPanel(look);
  wTabBar->AddTab(times_panel, _T("Times"), TimesIcon);

  Widget *wClose = new ActionWidget(*wf, mrOK);
  wTabBar->AddTab(wClose, _("Close"));


  /* restore previous page */

  if (start_page != -1) {
    status_page = start_page;
  }

  wTabBar->SetCurrentPage(status_page);

  SetTitle();

  wf->ShowModal();

  /* save page number for next time this dialog is opened */
  status_page = wTabBar->GetCurrentPage();

  delete wf;
}
