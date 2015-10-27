/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include <windef.h>
#include "Dialogs/DialogSettings.hpp"
#include "Dialogs/SimulatorPromptWindow.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Message.hpp"
#include "Widget/WindowWidget.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Init.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Language/Language.hpp"
#include "Form/ActionListener.hpp"
#include "Screen/Custom/TopCanvas.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Canvas.hpp"
#include "SDCardSync.hpp"
#include "System.hpp"
#include "NetworkDialog.hpp"
#include "Event/Timer.hpp"
#include "OS/FileUtil.hpp"
#include "Form/Button.hpp"
#include "LocalPath.hpp"
#include "IO/FileLineReader.hpp"
#include "Dialogs/Settings/Panels/StartupConfigPanel.hpp"
#include "CommandLine.hpp"
#include "OS/Args.hpp"
#include "Simulator.hpp"

enum Buttons {
  LAUNCH_NICKEL = 100,
  NETWORK,
  REBOOT,
  POWEROFF
};

static DialogSettings dialog_settings;
static SingleWindow *global_main_window;
static DialogLook *global_dialog_look;
static IconLook *global_icon_look;

const DialogSettings &
UIGlobals::GetDialogSettings()
{
  return dialog_settings;
}

SingleWindow &
UIGlobals::GetMainWindow()
{
  assert(global_main_window != nullptr);

  return *global_main_window;
}

const DialogLook &
UIGlobals::GetDialogLook()
{
  assert(global_dialog_look != nullptr);

  return *global_dialog_look;
}

const IconLook &
UIGlobals::GetIconLook()
{
  assert(global_icon_look != nullptr);

  return *global_icon_look;
}

class KoboMenuWidget final : public WindowWidget, ActionListener, Timer {
  ActionListener &dialog;
  SimulatorPromptWindow w;
  Button *poweroff_button;

  /* should poweroff button behave as a reboot button ? */
  bool do_reboot;

  /** was usb storage mounted last time we checked? */
  bool usb_storage_mounted;

public:
  KoboMenuWidget(const DialogLook &_look,
                 ActionListener &_dialog)
    :dialog(_dialog),
     w(_look, _dialog, false), do_reboot(false), usb_storage_mounted(false) {}

  void CreateButtons(WidgetDialog &buttons);

  /**
   * if update file exists, change "Poweroff" button to "Reboot"
   */
  void UpdateButtons();

  /**
   * If USB Storage is just mounted, then show the SDCard Sync dialog
   */
  void CheckUSBStorage();

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;

  virtual void Unprepare() override {
    Timer::Cancel();
    WindowWidget::Unprepare();
  }

  virtual bool KeyPress(unsigned key_code) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;

  /* virtual methods from class Timer */
  virtual void OnTimer() {
    UpdateButtons();
    CheckUSBStorage();
  }

  const TCHAR *
  GetPowerOffCaption(bool do_reboot)
  {
    return do_reboot ? _T("Reboot") : _T("Poweroff");
  }
};

void
KoboMenuWidget::CreateButtons(WidgetDialog &buttons)
{
  buttons.AddButton(("PC connect"), *this, LAUNCH_NICKEL);
  buttons.AddButton(_("Wifi"), *this, NETWORK);
  poweroff_button = buttons.AddButton(GetPowerOffCaption(false), *this, POWEROFF);
  UpdateButtons();
}

void
KoboMenuWidget::CheckUSBStorage()
{
  if (IsUSBStorageConnected() && !usb_storage_mounted) {
    ShowSDCardSyncDialog();
  }
  usb_storage_mounted = IsUSBStorageConnected();
}

void
KoboMenuWidget::UpdateButtons()
{
  if (do_reboot)
    return;

  if(File::Exists(_T("/mnt/onboard/.kobo/KoboRoot.tgz"))) {
    do_reboot = true;
    poweroff_button->SetCaption(GetPowerOffCaption(do_reboot));
  }
}

void
KoboMenuWidget::Prepare(ContainerWindow &parent,
                        const PixelRect &rc)
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();

  w.Create(parent, rc, style);
  SetWindow(&w);
  Timer::Schedule(1000);
}

bool
KoboMenuWidget::KeyPress(unsigned key_code)
{
  switch (key_code) {
#ifdef USE_LINUX_INPUT
  case KEY_POWER:
    dialog.OnAction(POWEROFF);
    return true;
#endif

  default:
    return false;
  }
}

void
KoboMenuWidget::OnAction(int id)
{
  switch (id) {
  case NETWORK:
    ShowNetworkDialog();
    break;

  case POWEROFF:
    if (do_reboot)
      dialog.OnAction(REBOOT);
    else
      dialog.OnAction(POWEROFF);
    break;
  case LAUNCH_NICKEL:
    if (ShowMessageBox(_("Connect to PC with USB cable?  \n\nThis takes a minute.  "
        "Disconnect cable now.  The screen will flash. Click 'Computer Setup' and connect cable"), _("USB cable connect"),
                       MB_YESNO | MB_ICONQUESTION) == IDYES)
      if (ShowMessageBox(_("Important!\n\n"
          "Do not use the 'Wireless setup' option.  You cannot use the Kobo as an Book reader.  Doing so will break Top Hat!"), _("Warning!"),
                         MB_OKCANCEL | MB_ICONWARNING) != IDCANCEL)
        dialog.OnAction(LAUNCH_NICKEL);
  }
}

static int
Main(SingleWindow &main_window, const DialogLook &dialog_look)
{
  WidgetDialog dialog(dialog_look);
  KoboMenuWidget widget(dialog_look, dialog);
  dialog.CreateFull(main_window, _T(""), &widget);
  widget.CreateButtons(dialog);

  const int result = dialog.ShowModal();
  dialog.StealWidget();
  return result;
}

static PixelSize
GetScreenSize()
{
  TopCanvas screen;
  screen.Create(PixelSize{100, 100}, true, false);

  PixelSize size(600, 800);
  {
    Canvas canvas = screen.Lock();
    if (canvas.IsDefined()) {
      size = canvas.GetSize();
      screen.Unlock();
    }
  }

  return size;
}

static int
Main()
{
  dialog_settings.SetDefaults();

  ScreenGlobalInit screen_init;
  PixelSize screen_size = GetScreenSize();
  Layout::Initialize(screen_size);

  DialogLook dialog_look;
  dialog_look.Initialise();

  IconLook icon_look;
  icon_look.Initialise();

  TopWindowStyle main_style;
  main_style.Resizable();

  SingleWindow main_window;
  main_window.Create(_T("XCSoar/KoboMenu"), {600, 800}, main_style);
  main_window.Show();

  global_dialog_look = &dialog_look;
  global_icon_look = &icon_look;
  global_main_window = &main_window;

  int action = Main(main_window, dialog_look);

  main_window.Destroy();

  return action;
}

int main(int argc, char **argv)
{
  static TCHAR path[MAX_PATH];
  StaticString<64> line;

  InitialiseDataPath();
  LocalPath(path, _T(TOPHAT_ARGUMENTS));

  FileLineReader *file = new FileLineReader(path);
  if (file != nullptr) {
          // program name is not included in command line on CE
    line.Format(_T("%s %s"), _T("Top_Hat_Soaring"), file->ReadLine());
    Args args(line.c_str(), "");
    args.SetStopOnError(false);
    CommandLine::Parse(args);
    delete file;
    if (!CommandLine::show_dialog_setup_quick)
      KoboRunXCSoar("");
  }

  while (true) {
    switch (Main()) {
    case LAUNCH_NICKEL:
      KoboExecNickel();
      return EXIT_FAILURE;

    case SimulatorPromptWindow::FLY:
      KoboRunXCSoar("-fly");
      /* return to menu after XCSoar quits */
      break;

    case SimulatorPromptWindow::SIMULATOR:
      KoboRunXCSoar("-simulator");
      /* return to menu after XCSoar quits */
      break;

    case REBOOT:
      KoboReboot();
      return EXIT_SUCCESS;

    case POWEROFF:
      KoboPowerOff();
      return EXIT_SUCCESS;

    default:
      return EXIT_SUCCESS;
    }
  }
}
