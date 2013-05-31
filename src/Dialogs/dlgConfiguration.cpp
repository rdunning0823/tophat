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
#include "Dialogs/Message.hpp"
#include "UIGlobals.hpp"
#include "Form/TabMenu.hpp"
#include "Form/CheckBox.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/FileReader.hpp"
#include "Screen/Busy.hpp"
#include "Screen/Key.h"
#include "Screen/Layout.hpp"
#include "Profile/Profile.hpp"
#include "LogFile.hpp"
#include "Util/Macros.hpp"
#include "ConfigPanels/ConfigPanel.hpp"
#include "ConfigPanels/PagesConfigPanel.hpp"
#include "ConfigPanels/PolarConfigPanel.hpp"
#include "ConfigPanels/UnitsConfigPanel.hpp"
#include "ConfigPanels/TimeConfigPanel.hpp"
#include "ConfigPanels/LoggerConfigPanel.hpp"
#include "ConfigPanels/LoggerInfoConfigPanel.hpp"
#include "ConfigPanels/DevicesConfigPanel.hpp"
#include "ConfigPanels/AirspaceConfigPanel.hpp"
#include "ConfigPanels/SiteConfigPanel.hpp"
#include "ConfigPanels/MapDisplayConfigPanel.hpp"
#include "ConfigPanels/WaypointDisplayConfigPanel.hpp"
#include "ConfigPanels/SymbolsConfigPanel.hpp"
#include "ConfigPanels/TerrainDisplayConfigPanel.hpp"
#include "ConfigPanels/GlideComputerConfigPanel.hpp"
#include "ConfigPanels/WindConfigPanel.hpp"
#include "ConfigPanels/SafetyFactorsConfigPanel.hpp"
#include "ConfigPanels/RouteConfigPanel.hpp"
#include "ConfigPanels/InterfaceConfigPanel.hpp"
#include "ConfigPanels/LayoutConfigPanel.hpp"
#include "ConfigPanels/GaugesConfigPanel.hpp"
#include "ConfigPanels/VarioConfigPanel.hpp"
#include "ConfigPanels/TaskRulesConfigPanel.hpp"
#include "ConfigPanels/InfoBoxesConfigPanel.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Audio/Features.hpp"

#ifdef HAVE_PCM_PLAYER
#include "ConfigPanels/AudioVarioConfigPanel.hpp"
#endif

#ifdef HAVE_TRACKING
#include "ConfigPanels/TrackingConfigPanel.hpp"
#endif

#ifdef HAVE_MODEL_TYPE
#include "ConfigPanels/ExperimentalConfigPanel.hpp"
#endif

#include <assert.h>

static unsigned current_page;
static WndForm *dialog = NULL;
static bool single_page;

static TabMenuControl* tab_menu = NULL;

const TCHAR *main_menu_captions[] = {
  N_("Site Files"),
  N_("Map Display"),
  N_("Glide Computer"),
  N_("Gauges"),
  N_("Contest"),
  N_("Look"),
  N_("Setup"),
};

static const TabMenuControl::PageItem pages[] = {
  {N_("Site Files"), 0, CreateSiteConfigPanel },
  {N_("Orientation"), 1, CreateMapDisplayConfigPanel },
  {N_("Elements"), 1, CreateSymbolsConfigPanel },
  {N_("Waypoints"), 1, CreateWaypointDisplayConfigPanel },
  {N_("Terrain"), 1, CreateTerrainDisplayConfigPanel },
  {N_("Airspace"), 1, CreateAirspaceConfigPanel },
  {N_("Safety Factors"), 2, CreateSafetyFactorsConfigPanel },
  {N_("Glide Computer"), 2, CreateGlideComputerConfigPanel },
  {N_("Wind"), 2, CreateWindConfigPanel },
  {N_("FLARM, Other"), 3, CreateGaugesConfigPanel },
#ifdef HAVE_PCM_PLAYER
  {N_("Audio Vario"), 3, CreateAudioVarioConfigPanel },
#endif
  {N_("Contest"), 4, CreateTaskRulesConfigPanel },
  {N_("Language, Input"), 5, CreateInterfaceConfigPanel },
  {N_("Screen Layout"), 5, CreateLayoutConfigPanel },
  {N_("InfoBox Pages"), 5, CreatePagesConfigPanel },
  {N_("InfoBox Modes"), 5, CreateInfoBoxesConfigPanel },
  {N_("Logger"), 6, CreateLoggerConfigPanel },
  {N_("Units"), 6, CreateUnitsConfigPanel },
  // Important: all pages after Units in this list must not have data fields that are
  // unit-dependent because they will be saved after their units may have changed.
  // ToDo: implement API that controls order in which pages are saved
  {N_("Time"), 6, CreateTimeConfigPanel },
#ifdef HAVE_TRACKING
  {N_("Tracking"), 6, CreateTrackingConfigPanel },
#endif
#ifdef HAVE_MODEL_TYPE
  {N_("Experimental Features"), 6, CreateExperimentalConfigPanel, },
#endif
};

WndForm &
ConfigPanel::GetForm()
{
  assert(dialog != NULL);

  return *dialog;
}

WndButton *
ConfigPanel::GetExtraButton(unsigned number)
{
  assert(number >= 1 && number <= 2);

  WndButton *extra_button = NULL;

  switch (number) {
  case 1:
    extra_button = (WndButton *)dialog->FindByName(_T("cmdExtra1"));
    break;
  case 2:
    extra_button = (WndButton *)dialog->FindByName(_T("cmdExtra2"));
    break;
  }

  return extra_button;
}

static void
OnUserLevel(CheckBoxControl &control)
{
  const bool expert = control.GetState();
  CommonInterface::SetUISettings().dialog.expert = expert;
  Profile::Set(ProfileKeys::UserLevel, expert);
  dialog->FilterAdvanced(expert);
  tab_menu->UpdateLayout();
}

static void
OnNextClicked(gcc_unused WndButton &button)
{
  tab_menu->NextPage();
}

static void
OnPrevClicked(gcc_unused WndButton &button)
{
  tab_menu->PreviousPage();
}

/**
 * close dialog from menu page.  from content, goes to menu page
 */
static void
OnCloseClicked(gcc_unused WndButton &button)
{
  if (tab_menu->IsCurrentPageTheMenu() || single_page)
    dialog->SetModalResult(mrOK);
  else
    tab_menu->GotoMenuPage();
}

static bool
FormKeyDown(gcc_unused WndForm &Sender, unsigned key_code)
{
  if (single_page)
    return false;

  switch (key_code) {
  case VK_LEFT:
#ifdef GNAV
  case '6':
#endif
    if (tab_menu->IsCurrentPageTheMenu()) {
      tab_menu->FocusMenuPage();
      tab_menu->HighlightPreviousMenuItem();
    } else {
      tab_menu->PreviousPage();
      ((WndButton *)dialog->FindByName(_T("cmdPrev")))->SetFocus();
    }
    return true;

  case VK_RIGHT:
#ifdef GNAV
  case '7':
#endif
    if (tab_menu->IsCurrentPageTheMenu()) {
      tab_menu->FocusMenuPage();
      tab_menu->HighlightNextMenuItem();
    } else {
      tab_menu->NextPage();
      ((WndButton *)dialog->FindByName(_T("cmdNext")))->SetFocus();
    }
    return true;

  default:
    return false;
  }
}

static void
PrepareConfigurationMenu()
{
  assert (dialog != NULL);

  tab_menu = (TabMenuControl*)dialog->FindByName(_T("TabMenu"));
  assert(tab_menu != NULL);
  tab_menu->InitMenu(pages,
                     ARRAY_SIZE(pages),
                     main_menu_captions,
                     ARRAY_SIZE(main_menu_captions));
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnUserLevel),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

static void
PrepareConfigurationDialog(const TCHAR *page_name)
{
  gcc_unused ScopeBusyIndicator busy;

  dialog = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ? _T("IDR_XML_CONFIGURATION_L") :
                                      _T("IDR_XML_CONFIGURATION"));
  if (dialog == NULL)
    return;

  dialog->SetKeyDownNotify(FormKeyDown);

  CommonInterface::SetUISettings().dialog.expert = false;
  bool expert_mode = CommonInterface::GetUISettings().dialog.expert;
  CheckBox *cb = (CheckBox *)dialog->FindByName(_T("Expert"));
  cb->SetState(expert_mode);
  dialog->FilterAdvanced(expert_mode);

  PrepareConfigurationMenu();
  int page = tab_menu->FindPage(page_name);
  single_page = (page >= 0);
  cb->SetVisible(!single_page);
  if (single_page) {
    assert((unsigned)page < tab_menu->GetNumPages());
    WndButton *b = (WndButton*)dialog->FindByName(_T("cmdPrev"));
    b->SetVisible(false);
    b = (WndButton*)dialog->FindByName(_T("cmdNext"));
    b->SetVisible(false);
    current_page = (unsigned)page;
    tab_menu->SetCurrentPage(current_page);
  } else
    tab_menu->GotoMenuPage();
  /* restore last selected menu item */
  static bool Initialized = false;
  if (!Initialized)
    Initialized = true;
  else
    tab_menu->SetLastContentPage(current_page);
}

static void
Save()
{
  /* save page number for next time this dialog is opened */
  current_page = tab_menu->GetLastContentPage();

  // TODO enhancement: implement a cancel button that skips all this
  // below after exit.
  bool changed = false;
  bool requirerestart = false;
  tab_menu->Save(changed, requirerestart);

  if (changed) {
    Profile::Save();
    LogDebug(_T("Configuration: Changes saved"));
    if (requirerestart)
      ShowMessageBox(_("Changes to configuration saved.  Restart Top Hat to apply changes."),
                  _T(""), MB_OK);
  }
}

void dlgConfigurationShowModal(const TCHAR *page_name)
{
  PrepareConfigurationDialog(page_name);

  dialog->ShowModal();

  if (dialog->IsDefined()) {
    /* hide the dialog, to avoid redraws inside Save() on a dialog
       that has already been deregistered from the SingleWindow */
    dialog->Hide();

    Save();
  }

  /* destroy the TabMenuControl first, to have a well-defined
     destruction order; this is necessary because some config panels
     refer to buttons belonging to the dialog */
  tab_menu->reset();

  delete dialog;
  dialog = NULL;
}
