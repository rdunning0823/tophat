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
#include "Atmosphere/Temperature.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Listener.hpp"
#include "Screen/SingleWindow.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "Form/RowFormWidget.hpp"
#include "Form/Button.hpp"
#include "Form/Form.hpp"
#include "Form/ButtonPanel.hpp"
#include "Language/Language.hpp"
#include "Operation/MessageOperationEnvironment.hpp"
#include "Compiler.h"

#include <math.h>

enum ControlIndex {
  QNH,
  Altitude,
};

enum Actions {
  DUMP = 100,
};

class QNHPanel
  : public RowFormWidget, DataFieldListener
{
  fixed last_altitude;

public:
  QNHPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()),
     last_altitude(-2)
  {}

  void ShowAltitude(fixed altitude);
  void RefreshAltitudeControl();
  void SetQNH(AtmosphericPressure qnh);

  void OnTimer();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* virtual methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

static QNHPanel *instance;

void
QNHPanel::ShowAltitude(fixed altitude)
{
  if (fabs(altitude - last_altitude) >= fixed_one) {
    last_altitude = altitude;
    LoadValue(Altitude, altitude, UnitGroup::ALTITUDE);
  }

  ShowRow(Altitude);
}

void
QNHPanel::RefreshAltitudeControl()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();

  if (basic.pressure_altitude_available && settings_computer.pressure_available)
    ShowAltitude(settings_computer.pressure.PressureAltitudeToQNHAltitude(
                 basic.pressure_altitude));
  else if (basic.baro_altitude_available)
    ShowAltitude(basic.baro_altitude);
  else
    HideRow(Altitude);
}

void
QNHPanel::SetQNH(AtmosphericPressure qnh)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();

  settings_computer.pressure = qnh;
  settings_computer.pressure_available.Update(basic.clock);

  if (device_blackboard != NULL) {
    MessageOperationEnvironment env;
    device_blackboard->SetQNH(qnh, env);
  }

  RefreshAltitudeControl();
}

void
QNHPanel::OnTimer()
{
  RefreshAltitudeControl();
}

void
QNHPanel::OnModified(DataField &df)
{
  if (IsDataField(QNH, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    SetQNH(Units::FromUserPressure(dff.GetAsFixed()));
  }
}

void
QNHPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  WndProperty *wp;
  wp = AddFloat(_("QNH"),
                _("Area pressure for barometric altimeter calibration.  This is set automatically if Vega connected."),
                GetUserPressureFormat(), GetUserPressureFormat(),
                Units::ToUserPressure(Units::ToSysUnit(fixed(850), Unit::HECTOPASCAL)),
                Units::ToUserPressure(Units::ToSysUnit(fixed(1300), Unit::HECTOPASCAL)),
                GetUserPressureStep(), false,
                Units::ToUserPressure(settings.pressure), this);
  {
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
    df.SetUnits(Units::GetPressureName());
    wp->RefreshDisplay();
  }

  AddReadOnly(_("Altitude"), NULL, _T("%.0f %s"),
              UnitGroup::ALTITUDE, fixed_zero);

  OnTimer();
}

bool
QNHPanel::Save(bool &changed, bool &require_restart)
{
  return true;
}

/**
 * This function is called repeatedly by the timer and updates the
 * current altitude.
 */
static void
OnTimerNotify(gcc_unused WndForm &Sender)
{
  instance->OnTimer();
}

void
dlgQNHShowModal()
{
  instance = new QNHPanel();
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  WidgetDialog dialog(_("QNH"), rc, instance);
  dialog.SetTimerNotify(OnTimerNotify);
  dialog.AddButton(_("OK"), mrOK);

  dialog.ShowModal();
}
