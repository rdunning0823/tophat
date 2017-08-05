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

#include "dlgSetupQuick.hpp"
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
#include "Renderer/SymbolButtonRenderer.hpp"
#include "Form/Draw.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Widget/Widget.hpp"
#include "Widget/ManagedWidget.hpp"
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
#include "Profile/Map.hpp"
#include "Profile/Current.hpp"
#include "Device/Register.hpp"
#include "Device/MultipleDevices.hpp"
#include "Formatter/UserUnits.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Units/UnitsStore.hpp"
#include "Dialogs/Settings/Panels/NationalityConfigPanel.hpp"
#include "Dialogs/Settings/Panels/SiteConfigPanel.hpp"
#include "Dialogs/Settings/Panels/SafetyFactorsConfigPanel.hpp"
#include "Dialogs/Settings/Panels/LayoutConfigPanel.hpp"
#include "UtilsSettings.hpp"
#include "Simulator.hpp"
#include "Event/Timer.hpp"
#include "Dialogs/TextEntry.hpp"

#include <math.h>
#include <assert.h>

// TODO: not sure i needed to change the order here
  enum ControlIndex {
    NATIONALITY,
    DEVICE,
    SITE_FILES,
    SAFETY,
    PLANE,
    PILOT,
    OK,
    SCREENS,
    ADVANCED,
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

class SetupQuick : public NullWidget, public WndForm, private Timer
{
private:
  PixelRect rc_site_files_text, rc_plane_text, rc_device_text;
  PixelRect rc_site_files_button, rc_plane_button, rc_device_button;
  PixelRect rc_safety_text, rc_nationality_text, rc_pilot_text;
  PixelRect rc_safety_button, rc_nationality_button, rc_pilot_button;
  PixelRect rc_screens_button;
  PixelRect rc_ok, rc_advanced;

  WndFrame *site_files_text, *plane_text, *device_text;
  WndFrame *safety_text, *nationality_text, *pilot_text;
  Button *site_files_button, *plane_button, *device_button;
  Button *safety_button, *nationality_button, *pilot_button;
  Button *screens_button;
  Button *ok, *advanced;

public:
  SetupQuick()
    : WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
              UIGlobals::GetMainWindow().GetClientRect(),
              _("Set up Top Hat"),
              GetDialogStyle()) {}

  void RefreshForm();
  void RefreshDeviceStatus();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Show(const PixelRect &rc) override {};
  virtual void Hide() override {};

  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);

#ifndef _WIN32_WCE
  /* This crashes Prepare on PPC2003 */
  virtual void OnResize(PixelSize new_size) override;
#endif

  virtual void ReinitialiseLayout(const PixelRect &parent_rc) override;

  /**
   * from ActionListener
   */
  virtual void OnAction(int id) override;

  /**
   * Reads relevant devices and puts them into a string
   * buffer_out the buffer
   */
  void CreateDeviceList(TCHAR *buffer_out, size_t buffer_size);

private:
  using Window::OnTimer;
  virtual void OnTimer() override;
};

static SetupQuick *instance;

void
SetupQuick::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc = WndForm::GetClientRect();
  rc.bottom -= WndForm::GetTitleHeight();
  const PixelScalar bottom_row_height = Layout::GetMinimumControlHeight();
  unsigned max_control_height =
      (rc.GetSize().cy - bottom_row_height) / 7;

  PixelScalar height = std::min(max_control_height, Layout::GetMaximumControlHeight());

  PixelRect rc_left = rc;
  PixelRect rc_right = rc;

  rc_left.left += Layout::Scale(2);
  rc_right.right -= Layout::Scale(2);
  rc_left.right = PixelScalar (fixed(rc.right + rc.left) / fixed(2.5));
  rc_right.left = rc_left.right + Layout::Scale(1);

  rc_site_files_text = rc_plane_text = rc_device_text = rc_safety_text
      = rc_nationality_text = rc_pilot_text = rc_right;
  rc_site_files_button = rc_plane_button = rc_device_button
      = rc_safety_button = rc_nationality_button = rc_pilot_button
      = rc_left;

  PixelScalar top1, top2, top3, top4, top5, top6;
  top1 = rc.top;
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
  rc_ok.top = rc_ok.bottom - bottom_row_height;
  rc_advanced.top = rc_ok.top;
  rc_advanced.bottom = rc.bottom;

  rc_advanced.right = rc_right.right;
  rc_advanced.left = rc_advanced.right - Layout::Scale(80);
  rc_screens_button = rc_advanced;
  rc_screens_button.Offset(-rc_advanced.GetSize().cx, 0);

  rc_ok.right = rc_screens_button.left;
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
  case SCREENS:
    widget = CreateLayoutConfigPanel(true);
    title = _("Set up screen");
    break;
  case DEVICE:
  case PLANE:
  case ADVANCED:
  case PILOT:
  case OK:
    gcc_unreachable();
    return;
  }

  assert(widget != nullptr);
  SystemConfiguration(*widget, title.c_str());
}

static
void
dlgPilotTextEntry()
{
  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  LoggerSettings &logger = settings_computer.logger;

  StaticString<64>name(logger.pilot_name.c_str());

  if (TextEntryDialog(name.buffer(), name.CAPACITY, _("Pilot name"))) {
    logger.pilot_name = name;
    Profile::Set(ProfileKeys::PilotName, name.c_str());
    Profile::map.SetModified(true);
  }
}

void
SetupQuick::OnAction(int id)
{
  switch(id) {
  case NATIONALITY:
  case SITE_FILES:
  case SAFETY:
  case SCREENS:
    ShowPanel(id);
    break;

  case PILOT:
    dlgPilotTextEntry();
    break;

  case PLANE:
    dlgPlanesShowModal();
    break;

  case DEVICE:
    ShowDeviceList();
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
SetupQuick::CreateDeviceList(TCHAR *buffer_out, size_t buffer_size)
{
  bool first = true;

  StaticString<256> buffer(_T(""));
  for (unsigned idx = 0; idx < NUMDEV; idx++) {
    Item item;
    item.Set(CommonInterface::GetSystemSettings().devices[idx],
             (*devices)[idx], device_blackboard->RealState(idx));

    const DeviceConfig &config =
      CommonInterface::SetSystemSettings().devices[idx];
    const Flags flags(*item);

    StaticString<256> name(_T(""));

    if (config.UsesDriver()) {
      name = config.driver_name;
    } else {
      TCHAR port_name_buffer[128];
      name = config.GetPortName(port_name_buffer, ARRAY_SIZE(port_name_buffer));
    }

    bool show;
    show = false;
    StaticString<256> flag_text;
    const TCHAR *status;
    status = _T("");
    if (flags.alive) {
      show = true;
      if (flags.location) {
        flag_text = _("GPS fix");
      } else if (flags.gps) {
        /* device sends GPGGA, but no valid location */
        flag_text = _("Bad GPS");
      } else {
        flag_text = _("Connected");
      }

      status = flag_text;
    } else if (config.IsDisabled()) {
      flag_text = _("Disabled");
    } else if (is_simulator() || !config.IsAvailable()) {
      status = _("");
      show = true;
    } else if (flags.open) {
      status = _("No data");
      show = true;
    } else if (flags.duplicate) {
      status = _("Duplicate");
    } else if (flags.error) {
      status = _("Error");
      show = true;
    } else {
      status = _("Not connected");
      show = true;
    }

    if (show) {
      if (!first)
        buffer.append(_T(" \n"));

      buffer.append(name.c_str());
      if (StringLength(status) > 0)
        buffer.AppendFormat(_T(": %s"), status);

      first = false;
    }
  }
  CopyString(buffer_out, buffer.c_str(), buffer_size);
}

void
SetupQuick::RefreshDeviceStatus()
{
  const TCHAR unconfigured[] = N_("*** Not configured ***");
  StaticString<255> text;
  text.clear();
  CreateDeviceList(text.buffer(), text.CAPACITY);

  if (text.empty())
    text = gettext(unconfigured);
  device_text->SetCaption(text.c_str());
}

void
SetupQuick::RefreshForm()
{
  StaticString<255> text;
  StaticString<255> text_filename;
  const TCHAR unconfigured[] = N_("*** Not configured ***");

  RefreshDeviceStatus();

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
    text_filename = gettext(unconfigured);

  site_files_text->SetCaption(text_filename.c_str());

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  text = settings.plane.registration;
  if (!settings.plane.competition_id.empty()) {
    text.AppendFormat(_T(" - %s"), settings.plane.competition_id.c_str());
  }
  if (text.empty())
      text = gettext(unconfigured);
  plane_text->SetCaption(text.c_str());

  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;
  StaticString<25> terrain_height;
  FormatRelativeUserAltitude(
      task_behaviour.route_planner.safety_height_terrain, terrain_height.buffer(), true);
  FormatRelativeUserAltitude(task_behaviour.safety_height_arrival, text.buffer(),
                             true);

  text.AppendFormat(_T(" / %s"), terrain_height.c_str());
  safety_text->SetCaption(text.c_str());

  const TCHAR *active_language = GetActiveLanguageName();
  text.Format(_T("%s / %s"), ( active_language== nullptr) ? _("System") : active_language,
              (task_behaviour.contest_nationality == ContestNationalities::AMERICAN) ?
                  _("US task rules") : _("FAI task rules"));
  nationality_text->SetCaption(text);

  const LoggerSettings &logger = settings_computer.logger;
  text = logger.pilot_name;
  if (text.empty())
    text = gettext(unconfigured);
  pilot_text->SetCaption(text.c_str());

  nationality_button->SetCaption(_("Nationality"));
  device_button->SetCaption(_("Device"));
  site_files_button->SetCaption(_("Site files"));
  safety_button->SetCaption(_("Safety heights"));
  pilot_button->SetCaption(_("Pilot"));
  plane_button->SetCaption(_("Plane"));
  screens_button->SetCaption(_("Screen"));
  advanced->SetCaption(_("Advanced"));
  WndForm::SetCaption(_("Set up Top Hat"));
}

// TODO : needs to depend on landscape and maybe device to get order right
void
SetupQuick::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  PixelRect rc_form = rc;
  NullWidget::Prepare(parent, rc_form);

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

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  WindowStyle button_style;
  button_style.TabStop();

  nationality_button = new Button(GetClientAreaWindow(), button_look,
                                _("Nationality"),
                                rc_nationality_button,
                                button_style, *this, NATIONALITY);

  device_button = new Button(GetClientAreaWindow(), button_look,
                                _("Device"),
                                rc_device_button,
                                button_style, *this, DEVICE);

  site_files_button = new Button(GetClientAreaWindow(), button_look,
                                    _("Site files"),
                                    rc_site_files_button,
                                    button_style, *this, SITE_FILES);

  safety_button = new Button(GetClientAreaWindow(), button_look,
                                _("Safety heights"),
                                rc_safety_button,
                                button_style, *this, SAFETY);

  plane_button = new Button(GetClientAreaWindow(), button_look, _("Plane"),
                               rc_plane_button,
                               button_style, *this, PLANE);

  pilot_button = new Button(GetClientAreaWindow(), button_look,
                               _("Pilot"),
                               rc_pilot_button,
                               button_style, *this, PILOT);

  ok = new WndSymbolButton(GetClientAreaWindow(), button_look, _T("_X"),
                           rc_ok,
                           button_style, *this, OK);

  screens_button = new Button(GetClientAreaWindow(), button_look,
                                 _("Screen"),
                                 rc_screens_button,
                                 button_style, *this, SCREENS);

  advanced = new Button(GetClientAreaWindow(), button_look, _("Advanced"),
                           rc_advanced,
                           button_style, *this, ADVANCED);

  WndForm::Move(rc_form);
  RefreshForm();
  Timer::Schedule(1000);
}

void
SetupQuick::OnTimer()
{
  RefreshDeviceStatus();
}

void
SetupQuick::ReinitialiseLayout(const PixelRect &parent_rc)
{
  WndForm::Move(parent_rc);
}

#ifndef _WIN32_WCE
void
SetupQuick::OnResize(PixelSize new_size)
{
  WndForm::OnResize(new_size);
  SetRectangles(GetClientRect());

  nationality_text->Move(rc_nationality_text);
  site_files_text->Move(rc_site_files_text);
  plane_text->Move(rc_plane_text);
  device_text->Move(rc_device_text);
  safety_text->Move(rc_safety_text);
  pilot_text->Move(rc_pilot_text);
  nationality_button->Move(rc_nationality_button);
  site_files_button->Move(rc_site_files_button);
  plane_button->Move(rc_plane_button);
  device_button->Move(rc_device_button);
  safety_button->Move(rc_safety_button);
  pilot_button->Move(rc_pilot_button);
  screens_button->Move(rc_screens_button);
  ok->Move(rc_ok);
  advanced->Move(rc_advanced);
}
#endif

void
SetupQuick::Unprepare()
{
  Timer::Cancel();
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
  delete screens_button;
  delete ok;
  delete advanced;
}

void
ShowDialogSetupQuick()
{
  // add point to task
  ContainerWindow &w = UIGlobals::GetMainWindow();
  instance = new SetupQuick();
  ManagedWidget managed_widget(w, instance);
  managed_widget.Move(w.GetClientRect());
  managed_widget.Show();
  instance->ShowModal();

  Profile::Save();
}
