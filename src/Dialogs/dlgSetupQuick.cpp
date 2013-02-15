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
#include "ComputerSettings.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/DeviceListDialog.hpp"
#include "Dialogs/Planes.hpp"
#include "UtilsSettings.hpp"
#include "Util/StringUtil.hpp"
#include "Form/Button.hpp"
#include "Form/Draw.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Form/Widget.hpp"
#include "Interface.hpp"
#include "SystemSettings.hpp"
#include "Language/Language.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "Look/MapLook.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Profile/Profile.hpp"
#include "Device/Register.hpp"

#include <math.h>
#include <assert.h>

  enum ControlIndex {
    SITE_FILES,
    DEVICE,
    PLANE,
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
  PixelRect rc_ok, rc_advanced;

  WndFrame *prompt;
  WndFrame *site_files_text, *plane_text, *device_text;
  WndButton *site_files_button, *plane_button, *device_button;
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
  PixelScalar height = Layout::Scale(35);

  PixelRect rc_left = rc;
  PixelRect rc_right = rc;

  rc_prompt = rc;
  rc_prompt.bottom = auto_prompt ? rc_prompt.top + height + Layout::Scale(5) : rc.top;

  rc_left.left += Layout::Scale(2);
  rc_right.right -= Layout::Scale(2);
  rc_left.right = PixelScalar (fixed(rc.right + rc.left) / fixed(2.5));
  rc_right.left = rc_left.right + Layout::Scale(1);

  rc_site_files_text = rc_plane_text = rc_device_text = rc_right;
  rc_site_files_button = rc_plane_button = rc_device_button = rc_left;

  rc_site_files_text.top = rc_site_files_button.top = height / 2 + rc_prompt.bottom;
  rc_site_files_text.top = rc_site_files_button.top = height / 2 + rc_prompt.bottom;
  rc_plane_text.top = rc_plane_button.top = rc_site_files_text.top + height;
  rc_device_text.top = rc_device_button.top = rc_plane_text.top + height;

  rc_site_files_text.bottom = rc_site_files_button.bottom =
      rc_site_files_text.top + height;
  rc_plane_text.bottom = rc_plane_button.bottom =
      rc_plane_button.top + height;
  rc_device_text.bottom = rc_device_button.bottom =
      rc_device_button.top + height;

  rc_ok = rc;
  rc_ok.top = rc_ok.bottom - height;

  rc_advanced.right = rc_right.right;
  rc_advanced.left = rc_advanced.right - Layout::Scale(80);
  rc_advanced.bottom = rc_ok.top - 1;
  rc_advanced.top = rc_advanced.bottom - height;
}

void
SetupQuick::OnAction(int id)
{
  if (id == SITE_FILES) {
    SystemConfiguration(0);

  } else if (id == PLANE) {
    dlgPlanesShowModal(UIGlobals::GetMainWindow());

  } else if (id == DEVICE) {
    ShowDeviceList(UIGlobals::GetMainWindow(),
                   UIGlobals::GetDialogLook(),
                   UIGlobals::GetLook().terminal);

  } else if (id == ADVANCED) {
    SystemConfiguration();

  } else if (id == OK) {
      SetModalResult(mrOK);
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
  if (Profile::Get(ProfileKeys::MapFile) != nullptr)
    text = Profile::Get(ProfileKeys::MapFile);

  if (text.empty() && Profile::Get(ProfileKeys::WaypointFile) != nullptr)
    text = Profile::Get(ProfileKeys::WaypointFile);

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

  if (auto_prompt) {
    prompt->SetCaption(_("Please configure Top Hat using the three buttons below."));
  }
}

void
SetupQuick::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  PixelRect rc_form = rc;
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetRectangles(rc_form);
  StaticString<20> label_text;

  WindowStyle style_frame;
  style_frame.Border();

  site_files_text = new WndFrame(GetClientAreaWindow(), look,
      rc_site_files_text.left, rc_site_files_text.top,
      rc_site_files_text.right - rc_site_files_text.left,
      rc_site_files_text.bottom - rc_site_files_text.top,
      style_frame);
  site_files_text->SetVAlignCenter();

  plane_text = new WndFrame(GetClientAreaWindow(), look,
      rc_plane_text.left, rc_plane_text.top,
      rc_plane_text.right - rc_plane_text.left,
      rc_plane_text.bottom - rc_plane_text.top,
      style_frame);
  plane_text->SetVAlignCenter();

  device_text = new WndFrame(GetClientAreaWindow(), look,
      rc_device_text.left, rc_device_text.top,
      rc_device_text.right - rc_device_text.left,
      rc_device_text.bottom - rc_device_text.top,
      style_frame);
  device_text->SetVAlignCenter();

  if (auto_prompt) {
    prompt = new WndFrame(GetClientAreaWindow(), look,
        rc_prompt.left, rc_prompt.top,
        rc_prompt.right - rc_prompt.left,
        rc_prompt.bottom - rc_prompt.top,
        style_frame);
    prompt->SetVAlignCenter();
  }

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  site_files_button = new WndButton(GetClientAreaWindow(), dialog_look,
                                    _T("Site files"),
                                    rc_site_files_button,
                                    button_style, this, SITE_FILES);

  plane_button = new WndButton(GetClientAreaWindow(), dialog_look, _T("Plane"),
                               rc_plane_button,
                               button_style, this, PLANE);

  device_button = new WndButton(GetClientAreaWindow(), dialog_look,
                                _T("Device"),
                                rc_device_button,
                                button_style, this, DEVICE);

  ok = new WndButton(GetClientAreaWindow(), dialog_look, _("Close"),
                     rc_ok,
                     button_style, this, OK);

  advanced = new WndButton(GetClientAreaWindow(), dialog_look, _T("Advanced"),
                           rc_advanced,
                           button_style, this, ADVANCED);

  RefreshForm();
}

void
SetupQuick::Unprepare()
{
  delete site_files_text;
  delete plane_text;
  delete device_text;
  delete site_files_button;
  delete plane_button;
  delete device_button;
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
