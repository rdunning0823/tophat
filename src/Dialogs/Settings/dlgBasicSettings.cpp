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

#include "Dialogs/Dialogs.h"
#include "Dialogs/WidgetDialog.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Computer/Settings.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "Formatter/UserUnits.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/Button.hpp"
#include "Language/Language.hpp"
#include "Operation/MessageOperationEnvironment.hpp"
#include "Event/Timer.hpp"
#include "Compiler.h"

#include <math.h>

enum ControlIndex {
  Ballast,
  WingLoading,
  Bugs,
  MaxBallast,
};

enum Actions {
  DUMP = 100,
};

class FlightSetupPanel final
  : public RowFormWidget, DataFieldListener,
    private Timer,
    public ActionListener {
  Button *dump_button;

  PolarSettings &polar_settings;

public:
  FlightSetupPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()),
     dump_button(NULL),
     polar_settings(CommonInterface::SetComputerSettings().polar)
  {}

  void SetDumpButton(Button *_dump_button) {
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

  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

  virtual void Show(const PixelRect &rc) override {
    RowFormWidget::Show(rc);
    Timer::Schedule(500);

    OnTimer();
    SetButtons();
    SetBallast();
  }

  virtual void Hide() override {
    Timer::Cancel();
    RowFormWidget::Hide();
  }

  /* virtual methods from ActionListener */
  virtual void OnAction(int id) override;

private:
  /* virtual methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;

  /* virtual methods from Timer */
  virtual void OnTimer() override;
};

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
    LoadValue(Ballast, polar_settings.glide_polar_task.GetBallastLitres(),
              UnitGroup::VOLUME);

  const fixed wl = polar_settings.glide_polar_task.GetWingLoading();
  SetRowVisible(WingLoading, positive(wl));
  if (positive(wl))
    LoadValue(WingLoading, wl, UnitGroup::WING_LOADING);

  if (device_blackboard != NULL) {
    const Plane &plane = CommonInterface::GetComputerSettings().plane;
    if (positive(plane.dry_mass)) {
      fixed fraction = polar_settings.glide_polar_task.GetBallast();
      fixed overload = (plane.dry_mass + fraction * plane.max_ballast) /
        plane.dry_mass;

      MessageOperationEnvironment env;
      device_blackboard->SetBallast(fraction, overload, env);
    }

    SetRowVisible(MaxBallast, ballastable);
    if (ballastable) {
      LoadValue(MaxBallast, plane.max_ballast, UnitGroup::VOLUME);
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
    fixed ballast = fixed(0);
    if (SaveValue(Ballast, UnitGroup::VOLUME, ballast))
      SetBallastLitres(ballast);
  } else if (IsDataField(Bugs, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    SetBugs(fixed(1) - (dff.GetAsFixed() / 100));
  }
}

void
FlightSetupPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  AddFloat(_("Ballast"),
           _("The amount of water ballast currently in the glider.  Affects the glide polar.  A 'Dump' button shows on the main menu if ballast > 0.  See 'Setup Plane.'  If flying with a smart vario, Max Ballast must also be configured in Tophat > Plane > Setup"),
           _T("%.0f %s"), _T("%.0f"),
           fixed(0), fixed(500), fixed(5), false,
           UnitGroup::VOLUME, fixed(0), this);

  WndProperty *wing_loading = AddFloat(_("Wing loading"), _("The current wing loading with the current ballast."),
                                       _T("%.1f %s"), _T("%.0f"), fixed(0),
                                       fixed(300), fixed(5),
                                       false, UnitGroup::WING_LOADING,
                                       fixed(0));
  wing_loading->SetReadOnly(true);

  AddFloat(_("Bugs"), /* xgettext:no-c-format */
           _("How clean the glider is. Set to 0% for clean, larger numbers as the wings "
               "pick up bugs or gets wet.  50% indicates the glider's sink rate is doubled."),
           _T("%.0f %%"), _T("%.0f"),
           fixed(0), fixed(50), fixed(1), false,
           (fixed(1) - polar_settings.bugs) * 100,
           this);

  WndProperty *max_ballast =  AddFloat(_("Max ballast"),
                                       _("The maximum amount of water ballast allowed for your glider.  If flying with a smart vario, Max Ballast must be configured in both Tophat > Plane > Setup and separately in your smart vario."),
                                       _T("%.0f %s"), _T("%.0f"),
                                       fixed(0), fixed(500), fixed(5), false,
                                       UnitGroup::VOLUME, fixed(0));

  max_ballast->SetReadOnly(true);
}

bool
FlightSetupPanel::Save(bool &changed)
{
  return true;
}

void
FlightSetupPanel::OnAction(int id)
{
  if (id == DUMP)
    FlipBallastTimer();
}

void
dlgBasicSettingsShowModal()
{
  FlightSetupPanel *instance = new FlightSetupPanel();

  const Plane &plane = CommonInterface::GetComputerSettings().plane;
  StaticString<128> caption(_("Bugs & ballast"));
  caption.append(_T(" - "));
  caption.append(plane.polar_name);

  WidgetDialog dialog(UIGlobals::GetDialogLook());
  dialog.CreateFull(UIGlobals::GetMainWindow(), caption, instance);
  dialog.AddSymbolButton(_T("_X"), mrOK);
  instance->SetDumpButton(dialog.AddButton(_("Dump"), *instance, DUMP));

  dialog.ShowModal();
}
