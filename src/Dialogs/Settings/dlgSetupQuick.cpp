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

#include "Blackboard/DeviceBlackboard.hpp"
#include "Compiler.h"
#include "Components.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/Device/DeviceListDialog.hpp"
#include "Dialogs/Plane/PlaneDialogs.hpp"
#include "UtilsSettings.hpp"
#include "Util/StringUtil.hpp"
#include "Util/ConvertString.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/ButtonPanel.hpp"
#include "Form/Button.hpp"
#include "Form/Draw.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Widget/Widget.hpp"
#include "Interface.hpp"
#include "SystemSettings.hpp"
#include "Language/Language.hpp"
#include "Language/LanguageGlue.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "Look/MapLook.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Profile/Profile.hpp"
#include "Device/Register.hpp"
#include "Formatter/UserUnits.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Units/UnitsStore.hpp"
#include "Dialogs/Settings/Panels/NationalityConfigPanel.hpp"
#include "Dialogs/Settings/Panels/SiteConfigPanel.hpp"
#include "Dialogs/Settings/Panels/LoggerConfigPanel.hpp"
#include "Dialogs/Settings/Panels/SafetyFactorsConfigPanel.hpp"
#include "UtilsSettings.hpp"

#include <math.h>
#include <assert.h>

  enum ControlIndex {
    NATIONALITY,
    SITE_FILES,
    DEVICE,
    PLANE,
    SAFETY,
    PILOT,
    ADVANCED,
    OK,
  };

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

class SetupQuick : public NullWidget, public WndForm
{
private:
  PixelRect rc_prompt;
  PixelRect rc_site_files_text, rc_plane_text, rc_device_text;
  PixelRect rc_site_files_button, rc_plane_button, rc_device_button;
  PixelRect rc_safety_text, rc_nationality_text, rc_pilot_text;
  PixelRect rc_safety_button, rc_nationality_button, rc_pilot_button;
  PixelRect rc_ok, rc_advanced;

  WndFrame *prompt;
  WndFrame *site_files_text, *plane_text, *device_text;
  WndFrame *safety_text, *nationality_text, *pilot_text;
  WndButton *site_files_button, *plane_button, *device_button;
  WndButton *safety_button, *nationality_button, *pilot_button;
  WndButton *ok, *advanced;

  /**
   *   do we explain that there is missing data?
   */
  bool auto_prompt;

public:
  SetupQuick(bool _auto_prompt)
    : WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
              UIGlobals::GetMainWindow().GetClientRect(),
              _("Set up Top Hat"),
              GetDialogStyle()), auto_prompt(_auto_prompt) {}

  void RefreshForm();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Show(const PixelRect &rc) {};
  virtual void Hide() {};
  virtual void Move(const PixelRect &rc) {};

  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);

  /**
   * from ActionListener
   */
  virtual void OnAction(int id);
};

static SetupQuick *instance;

void
SetupQuick::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc = WndForm::GetClientRect();
  rc.bottom -= WndForm::GetTitleHeight();
  PixelScalar height = Layout::Scale(25);

  PixelRect rc_left = rc;
  PixelRect rc_right = rc;

  rc_prompt = rc;
  rc_prompt.bottom = auto_prompt ? rc_prompt.top + height + Layout::Scale(5) : rc.top;

  rc_left.left += Layout::Scale(2);
  rc_right.right -= Layout::Scale(2);
  rc_left.right = PixelScalar (fixed(rc.right + rc.left) / fixed(2.5));
  rc_right.left = rc_left.right + Layout::Scale(1);

  rc_site_files_text = rc_plane_text = rc_device_text = rc_safety_text
      = rc_nationality_text = rc_pilot_text = rc_right;
  rc_site_files_button = rc_plane_button = rc_device_button
      = rc_safety_button = rc_nationality_button = rc_pilot_button = rc_left;

  PixelScalar top1, top2, top3, top4, top5, top6;
  top1 = rc_prompt.bottom;
  top2 = top1 + height;
  top3 = top1 + height * 2;
  top4 = top1 + height * 3;
  top5 = top1 + height * 4;
  top6 = top1 + height * 5;

  rc_nationality_button.top = rc_nationality_text.top = top1;
  rc_device_button.top = rc_device_text.top = top2;
  rc_site_files_button.top = rc_site_files_text.top = top3;
  rc_safety_button.top = rc_safety_text.top = top4;
  rc_plane_button.top = rc_plane_text.top = top5;
  rc_pilot_button.top = rc_pilot_text.top = top6;

  rc_nationality_button.bottom = rc_nationality_text.bottom =
      rc_nationality_text.top + height;
  rc_device_button.bottom = rc_device_text.bottom =
      rc_device_text.top + height;
  rc_site_files_button.bottom = rc_site_files_text.bottom =
      rc_site_files_text.top + height;
  rc_safety_button.bottom = rc_safety_text.bottom =
      rc_safety_text.top + height;
  rc_plane_button.bottom = rc_plane_text.bottom = rc_plane_text.top + height;
  rc_pilot_button.bottom = rc_pilot_text.bottom = rc_pilot_text.top + height;

  rc_ok = rc;
  rc_ok.top = rc_ok.bottom - height;

  rc_advanced.right = rc_right.right;
  rc_advanced.left = rc_advanced.right - Layout::Scale(80);
  rc_advanced.bottom = rc_ok.top - 1;
  rc_advanced.top = rc_advanced.bottom - height;
}

static void
ShowPanel(unsigned page)
{
  Widget *widget = nullptr;
  StaticString<120> title;

  switch (page) {
  case NATIONALITY:
    widget = CreateNationalityConfigPanel();
    title = _("Nationality");
    break;
  case SITE_FILES:
    widget = CreateSiteConfigPanel(true);
    title = _("Site files");
    break;
  case SAFETY:
    widget = CreateSafetyFactorsConfigPanel();
    title = _("Safety factors");
    break;
  case PILOT:
    widget = CreateLoggerConfigPanel();
    title = _("Pilot");
    break;
  case DEVICE:
  case PLANE:
  case ADVANCED:
  case OK:
    gcc_unreachable();
    return;
  }

  assert(widget != nullptr);
  SystemConfiguration(*widget, title.get());
}

void
SetupQuick::OnAction(int id)
{
  switch(id) {
  case NATIONALITY:
  case SITE_FILES:
  case SAFETY:
  case PILOT:
    ShowPanel(id);
    break;

  case PLANE:
    dlgPlanesShowModal();
    break;

  case DEVICE:
    ShowDeviceList(UIGlobals::GetLook().terminal);
    break;

  case ADVANCED:
    SystemConfiguration();
    break;

  case OK:
      SetModalResult(mrOK);
      break;
  }

  RefreshForm();
}

void
SetupQuick::RefreshForm()
{
  StaticString<255> text;
  StaticString<255> text_filename;
  const TCHAR unconfigured[] = _T("*** Not configured ***");

  const DeviceConfig &config =
    CommonInterface::SetSystemSettings().devices[0];
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  TCHAR port_name_buffer[128];
  const TCHAR *port_name =
    config.GetPortName(port_name_buffer, ARRAY_SIZE(port_name_buffer));

  text.clear();
  if (config.UsesDriver()) {
    const TCHAR *driver_name = FindDriverDisplayName(config.driver_name);

    text.AppendFormat(_("%s on %s"), driver_name, port_name);
  } else {
    text.append(port_name);
  }
  if (text.empty())
    text = unconfigured;
  device_text->SetCaption(text.c_str());

  text.clear();
  text_filename.clear();
  if (Profile::Get(ProfileKeys::MapFile) != nullptr) {
    UTF8ToWideConverter text2(Profile::Get(ProfileKeys::MapFile));
    if (text2.IsValid())
      text = text2;
  }

  if (text.empty() && Profile::Get(ProfileKeys::WaypointFile) != nullptr) {
    UTF8ToWideConverter text2(Profile::Get(ProfileKeys::WaypointFile));
    if (text2.IsValid())
      text = text2;
  }

  StaticString<15> local_path;
  local_path = _T("%LOCAL_PATH%\\");
  const TCHAR* name = StringAfterPrefix(text.c_str(), local_path.c_str());
  if (name != nullptr)
    text_filename = name;
  if (text_filename.empty())
    text_filename = unconfigured;

  site_files_text->SetCaption(text_filename.c_str());

  text = settings.plane.registration;
  if (text.empty())
      text = unconfigured;
  plane_text->SetCaption(text.c_str());

  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;
  FormatRelativeUserAltitude(task_behaviour.safety_height_arrival, text.buffer(), true);
  safety_text->SetCaption(text.c_str());

  text.Format(_T("%s / %s"), (GetActiveLanguageName() == nullptr) ? _T("System") : GetActiveLanguageName(),
              (task_behaviour.contest_nationality == ContestNationalities::AMERICAN) ?
                  N_("US task rules") : N_("FAI task rules"));
  nationality_text->SetCaption(text);

  const LoggerSettings &logger = settings_computer.logger;
  text = logger.pilot_name;
  if (text.empty())
    text = unconfigured;
  pilot_text->SetCaption(text.c_str());

  if (auto_prompt) {
    prompt->SetCaption(_("Please configure Top Hat using the buttons below."));
  }
}

void
SetupQuick::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  PixelRect rc_form = rc;
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetRectangles(rc_form);

  WindowStyle style_frame;

  nationality_text = new WndFrame(GetClientAreaWindow(), look,
                                  rc_nationality_text,
                                  style_frame);
                                  nationality_text->SetVAlignCenter();

  site_files_text = new WndFrame(GetClientAreaWindow(), look,
                                 rc_site_files_text,
                                 style_frame);
                                 site_files_text->SetVAlignCenter();

  plane_text = new WndFrame(GetClientAreaWindow(), look,
                            rc_plane_text,
                            style_frame);
                            plane_text->SetVAlignCenter();

  device_text = new WndFrame(GetClientAreaWindow(), look,
                             rc_device_text,
                             style_frame);
                             device_text->SetVAlignCenter();

  safety_text = new WndFrame(GetClientAreaWindow(), look,
                             rc_safety_text,
                             style_frame);
                             safety_text->SetVAlignCenter();

  pilot_text = new WndFrame(GetClientAreaWindow(), look,
                            rc_pilot_text,
                            style_frame);
                            pilot_text->SetVAlignCenter();

  if (auto_prompt) {
    prompt = new WndFrame(GetClientAreaWindow(), look,
                          rc_prompt,
                          style_frame);
                          prompt->SetVAlignCenter();
  }

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  nationality_button = new WndButton(GetClientAreaWindow(), button_look,
                                _T("Nationality"),
                                rc_nationality_button,
                                button_style, *this, NATIONALITY);

  site_files_button = new WndButton(GetClientAreaWindow(), button_look,
                                    _T("Site files"),
                                    rc_site_files_button,
                                    button_style, *this, SITE_FILES);

  plane_button = new WndButton(GetClientAreaWindow(), button_look, _T("Plane"),
                               rc_plane_button,
                               button_style, *this, PLANE);

  device_button = new WndButton(GetClientAreaWindow(), button_look,
                                _T("Device"),
                                rc_device_button,
                                button_style, *this, DEVICE);

  safety_button = new WndButton(GetClientAreaWindow(), button_look,
                                _T("Safety heights"),
                                rc_safety_button,
                                button_style, *this, SAFETY);

  pilot_button = new WndButton(GetClientAreaWindow(), button_look,
                               _T("Pilot"),
                               rc_pilot_button,
                               button_style, *this, PILOT);
  ok = new WndButton(GetClientAreaWindow(), button_look, _("Close"),
                     rc_ok,
                     button_style, *this, OK);

  advanced = new WndButton(GetClientAreaWindow(), button_look, _T("Advanced"),
                           rc_advanced,
                           button_style, *this, ADVANCED);

  RefreshForm();
}

void
SetupQuick::Unprepare()
{
  delete nationality_text;
  delete site_files_text;
  delete plane_text;
  delete device_text;
  delete safety_text;
  delete pilot_text;
  delete nationality_button;
  delete site_files_button;
  delete plane_button;
  delete device_button;
  delete safety_button;
  delete pilot_button;
  delete ok;
  delete advanced;
  if (auto_prompt)
     delete prompt;
}

void
ShowDialogSetupQuick(bool auto_prompt)
{
  // add point to task
  ContainerWindow &w = UIGlobals::GetMainWindow();
  instance = new SetupQuick(auto_prompt);
  instance->Initialise(w, w.GetClientRect());
  instance->Prepare(w, w.GetClientRect());

  instance->ShowModal();

  instance->Hide();
  instance->Unprepare();
  delete instance;

}
