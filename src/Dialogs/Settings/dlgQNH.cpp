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

#include "Dialogs/Settings/dlgQNH.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Protection.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Interface.hpp"
#include "Components.hpp"
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
  ShowDoubleUnits,
};

static QNHPanel *instance;

void
QNHPanel::ShowAltitude(fixed altitude)
{
  if (fabs(altitude - last_altitude) >= fixed(1)) {
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
  } else {
    if (IsDataField(ShowDoubleUnits, df)) {
      InfoBoxSettings &settings_info_boxes = CommonInterface::SetUISettings().info_boxes;
      const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
      settings_info_boxes.show_alternative_altitude_units = dfb.GetAsBoolean();
    }
  }
}

void
QNHPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;

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
              UnitGroup::ALTITUDE, fixed(0));
  if (show_double_units_checkbox) {
    AddBoolean(_("Show feet and meters"),
               _("Display second set of units in the bottom of altitude Infoboxes"),
               settings_info_boxes.show_alternative_altitude_units, this);
  } else {
    AddDummy();
  }
}

bool
QNHPanel::Save(bool &changed)
{
  return true;
}

void
dlgQNHShowModal()
{
  instance = new QNHPanel(false);

  StaticString<128> caption(_("QNH"));
  WidgetDialog dialog(UIGlobals::GetDialogLook());
  dialog.CreateFull(UIGlobals::GetMainWindow(), caption, instance);


  dialog.AddSymbolButton(_T("_X"), mrOK);

  dialog.ShowModal();
}
