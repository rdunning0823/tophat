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
#include "Dialogs/WidgetDialog.hpp"
#include "Protection.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "ComputerSettings.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Listener.hpp"
#include "Screen/SingleWindow.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/RowFormWidget.hpp"
#include "Form/Button.hpp"
#include "Form/Form.hpp"
#include "Form/ButtonPanel.hpp"
#include "Language/Language.hpp"
#include "Operation/MessageOperationEnvironment.hpp"
#include "Compiler.h"

#include <math.h>

enum ControlIndex {
  Ballast,
  WingLoading,
  Bugs,
};

enum Actions {
  DUMP = 100,
};

class FlightSetupPanel
  : public RowFormWidget, DataFieldListener, public ActionListener
{
  WndButton *dump_button;

  PolarSettings &polar_settings;

public:
  FlightSetupPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()),
     dump_button(NULL),
     polar_settings(CommonInterface::SetComputerSettings().polar)
  {}

  void SetDumpButton(WndButton *_dump_button) {
    dump_button = _dump_button;
  }

  void SetButtons();
  void SetBallast();
  void SetBallastTimer(bool active);
  void FlipBallastTimer();

  void PublishPolarSettings() {
    if (protected_task_manager != NULL)
      protected_task_manager->SetGlidePolar(polar_settings.glide_polar_task);
  }

  void SetBallastLitres(fixed ballast_litres) {
    polar_settings.glide_polar_task.SetBallastLitres(ballast_litres);
    PublishPolarSettings();
    SetButtons();
    SetBallast();
  }

  void SetBugs(fixed bugs);

  void OnTimer();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

  virtual void OnAction(int id);

private:
  /* virtual methods from DataFieldListener */
  virtual void OnModified(DataField &df);
  virtual void OnSpecial(DataField &df);
};

static FlightSetupPanel *instance;

void
FlightSetupPanel::SetButtons()
{
  dump_button->SetVisible(polar_settings.glide_polar_task.HasBallast());

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  dump_button->SetCaption(settings.polar.ballast_timer_active
                          ? _("Stop") : _("Dump"));
}

void
FlightSetupPanel::SetBallast()
{
  const bool ballastable = polar_settings.glide_polar_task.IsBallastable();
  SetRowVisible(Ballast, ballastable);
  if (ballastable)
    LoadValue(Ballast, polar_settings.glide_polar_task.GetBallastLitres());

  const fixed wl = polar_settings.glide_polar_task.GetWingLoading();
  SetRowVisible(WingLoading, positive(wl));
  if (positive(wl))
    LoadValue(WingLoading, wl);

  if (device_blackboard != NULL) {
    const Plane &plane = CommonInterface::GetComputerSettings().plane;
    if (positive(plane.dry_mass)) {
      fixed fraction = polar_settings.glide_polar_task.GetBallast();
      fixed overload = (plane.dry_mass + fraction * plane.max_ballast) /
        plane.dry_mass;

      MessageOperationEnvironment env;
      device_blackboard->SetBallast(fraction, overload, env);
    }
  }
}

void
FlightSetupPanel::SetBallastTimer(bool active)
{
  if (!polar_settings.glide_polar_task.HasBallast())
    active = false;

  PolarSettings &settings = CommonInterface::SetComputerSettings().polar;
  if (active == settings.ballast_timer_active)
    return;

  settings.ballast_timer_active = active;
  SetButtons();
}

void
FlightSetupPanel::FlipBallastTimer()
{
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  SetBallastTimer(!settings.polar.ballast_timer_active);
}

void
FlightSetupPanel::SetBugs(fixed bugs) {
  polar_settings.SetBugs(bugs);
  PublishPolarSettings();

  if (device_blackboard != NULL) {
    MessageOperationEnvironment env;
    device_blackboard->SetBugs(bugs, env);
  }
}

void
FlightSetupPanel::OnTimer()
{
  const PolarSettings &settings = CommonInterface::GetComputerSettings().polar;

  if (settings.ballast_timer_active) {
    /* display the new values on the screen */
    SetBallast();
  }
}

void
FlightSetupPanel::OnModified(DataField &df)
{
  if (IsDataField(Ballast, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    SetBallastLitres(dff.GetAsFixed());
  } else if (IsDataField(Bugs, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    SetBugs(fixed_one - (dff.GetAsFixed() / 100));
  }
}

void
FlightSetupPanel::OnSpecial(DataField &df)
{
  if (IsDataField(Ballast, df))
    FlipBallastTimer();
}

void
FlightSetupPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  AddFloat(_("Ballast"),
           _("Ballast of the glider.  Increase this value if the pilot/cockpit load is greater than the reference pilot weight of the glide polar (typically 75kg).  Press ENTER on this field to toggle count-down of the ballast volume according to the dump rate specified in the configuration settings."),
           _T("%.0f l"), _T("%.0f"),
           fixed_zero, fixed(500), fixed(5), false,
           fixed_zero,
           this);

  AddReadOnly(_("Wing loading"), NULL, _T("%.1f kg/m2"), fixed_zero);

  AddFloat(_("Bugs"), /* xgettext:no-c-format */
           _("How clean the glider is. Set to 0% for clean, larger numbers as the wings "
               "pick up bugs or gets wet.  50% indicates the glider's sink rate is doubled."),
           _T("%.0f %%"), _T("%.0f"),
           fixed_zero, fixed(50), fixed_one, false,
           (fixed_one - polar_settings.bugs) * 100,
           this);

  OnTimer();
  SetButtons();
  SetBallast();
}

bool
FlightSetupPanel::Save(bool &changed, bool &require_restart)
{
  return true;
}

void
FlightSetupPanel::OnAction(int id)
{
  if (id == DUMP)
    FlipBallastTimer();
}

/**
 * This function is called repeatedly by the timer and updates the
 * current ballast. The ballast can change without user
 * input due to the dump function.
 */
static void
OnTimerNotify(gcc_unused WndForm &Sender)
{
  instance->OnTimer();
}

void
dlgBasicSettingsShowModal()
{
  instance = new FlightSetupPanel();
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  WidgetDialog dialog(_("Bugs & ballast"), rc, instance);
  dialog.SetTimerNotify(OnTimerNotify);
  dialog.AddButton(_("OK"), mrOK);
  instance->SetDumpButton(dialog.AddButton(_("Dump"), instance, DUMP));

  dialog.ShowModal();
}
