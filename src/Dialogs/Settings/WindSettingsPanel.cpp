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

#include "WindSettingsPanel.hpp"
#include "Computer/Wind/Settings.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Angle.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Units/Units.hpp"

WindSettingsPanel::WindSettingsPanel(bool _edit_manual_wind,
                                     bool _clear_manual_button,
                                     bool _edit_trail_drift)
  :RowFormWidget(UIGlobals::GetDialogLook()),
   edit_manual_wind(_edit_manual_wind),
   clear_manual_button(_clear_manual_button),
   edit_trail_drift(_edit_trail_drift),
   form(nullptr) {}

void
WindSettingsPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  const WindSettings &settings = CommonInterface::GetComputerSettings().wind;
  const MapSettings &map_settings = CommonInterface::GetMapSettings();

  static constexpr StaticEnumChoice wind_source_list[] = {
    { MANUAL_WIND, N_("Manual"),
      N_("When the algorithm is switched off, the pilot is responsible for setting the wind estimate.") },
    { INTERNAL_WIND, N_("Internal calculation"),
      N_("Requires only a GPS source.") },
    { EXTERNAL_WIND_IF_AVAILABLE, N_("Prefer external wind"),
      N_("If enabled, then the wind vector received from external devices overrides "
          "Top Hat's internal wind calculation.") },
    { 0 }
  };

  user_wind_source = AddEnum(_("Wind source"),
                             _("Select the source of the wind calculation."),
                             wind_source_list,
                             (unsigned)settings.GetUserWindSource(),  this);

  if (edit_trail_drift)
    AddBoolean(_("Trail drift"),
               _("Determines whether the snail trail is drifted with the wind "
                 "when displayed in circling mode. Switched Off, "
                 "the snail trail stays uncompensated for wind drift."),
               map_settings.trail.wind_drift_enabled);
  else
    AddDummy();

  if (edit_manual_wind) {
    SpeedVector manual_wind = CommonInterface::Calculated().GetWindOrZero();

    AddReadOnly(_("Source"));

    WndProperty *wp =
      AddFloat(_("Speed"), _("Manual adjustment of wind speed."),
               _T("%.0f %s"), _T("%.0f"),
               fixed(0),
               Units::ToUserWindSpeed(Units::ToSysUnit(fixed(200),
                                                       Unit::KILOMETER_PER_HOUR)),
               fixed(1), false,
               Units::ToUserWindSpeed(manual_wind.norm),
               this);
    DataFieldFloat &df = *(DataFieldFloat *)wp->GetDataField();
    df.SetUnits(Units::GetWindSpeedName());
    wp->RefreshDisplay();

    wp = AddAngle(_("Direction"), _("Manual adjustment of wind direction."),
                  manual_wind.bearing, 5u, false,
                  this);

    manual_modified = false;
  }

  if (clear_manual_button)
    AddButton(_("Clear"), *this, CLEAR_MANUAL);

  UpdateVector();
  UpdatetManualVisibility();
}

void
WindSettingsPanel::UpdatetManualVisibility()
{
  if (!edit_manual_wind)
    return;

  assert(user_wind_source != nullptr);

  DataFieldEnum *wind_source_enum = (DataFieldEnum *)
      user_wind_source->GetDataField();
  bool manual_mode = wind_source_enum->GetAsInteger()
      == (unsigned)MANUAL_WIND;

  this->GetControl(Speed).SetVisible(manual_mode);
  this->GetControl(Direction).SetVisible(manual_mode);
}

void
WindSettingsPanel::Show(const PixelRect &rc)
{
  if (edit_manual_wind) {
    UpdateVector();
    CommonInterface::GetLiveBlackboard().AddListener(*this);
  }

  RowFormWidget::Show(rc);
  UpdatetManualVisibility();
}

void
WindSettingsPanel::Hide()
{
  RowFormWidget::Hide();

  if (edit_manual_wind)
    CommonInterface::GetLiveBlackboard().RemoveListener(*this);
}

bool
WindSettingsPanel::Save(bool &_changed)
{
  WindSettings &settings = CommonInterface::SetComputerSettings().wind;
  MapSettings &map_settings = CommonInterface::SetMapSettings();

  bool changed = false;

  unsigned source = (unsigned)settings.GetUserWindSource();
  if (SaveValueEnum(WIND_SOURCE, ProfileKeys::UserWindSource, source)) {
    settings.SetUserWindSource((UserWindSource)source);
    changed = true;
  }

  if (edit_trail_drift)
    changed |= SaveValue(TrailDrift, ProfileKeys::TrailDrift,
                         map_settings.trail.wind_drift_enabled);

  _changed |= changed;
  return true;
}

void
WindSettingsPanel::OnAction(int id)
{
  switch (id) {
  case CLEAR_MANUAL:
    CommonInterface::SetComputerSettings().wind.manual_wind_available.Clear();
    manual_modified = false;
    UpdateVector();
    break;
  }
}

void
WindSettingsPanel::OnModified(DataField &df)
{
  UpdatetManualVisibility();

  if (&df == &GetDataField(WIND_SOURCE)) {
    DataFieldEnum *wind_source_enum = (DataFieldEnum *)
        user_wind_source->GetDataField();
    if (wind_source_enum->GetAsInteger() != (unsigned)MANUAL_WIND &&
        form != nullptr)
      form->SetModalResult(mrOK);
  }

  if (!edit_manual_wind)
    return;

  const NMEAInfo &basic = CommonInterface::Basic();
  WindSettings &settings = CommonInterface::SetComputerSettings().wind;

  if (&df == &GetDataField(Speed) || &df == &GetDataField(Direction)) {
    settings.manual_wind.norm = Units::ToSysWindSpeed(GetValueFloat(Speed));
    settings.manual_wind.bearing = GetValueAngle(Direction);
    settings.manual_wind_available.Update(basic.clock);
    manual_modified = true;
  }

  UpdateVector();
}

void
WindSettingsPanel::UpdateVector()
{
  if (!edit_manual_wind)
    return;

  const DerivedInfo &calculated = CommonInterface::Calculated();
  const WindSettings &settings = CommonInterface::SetComputerSettings().wind;

  const TCHAR *source = nullptr;
  switch (manual_modified
          ? DerivedInfo::WindSource::MANUAL
          : calculated.wind_source) {
  case DerivedInfo::WindSource::NONE:
    source = _("None");
    break;

  case DerivedInfo::WindSource::MANUAL:
    source = _("Manual");
    break;

  case DerivedInfo::WindSource::AUTO:
    source = _("Internal");
    break;

  case DerivedInfo::WindSource::EXTERNAL:
    source = _("External");
    break;
  }

  SetText(SOURCE, source);

  if (!manual_modified && !settings.manual_wind_available) {
    SpeedVector wind = CommonInterface::Calculated().GetWindOrZero();
    LoadValue(Speed, Units::ToUserWindSpeed(wind.norm));
    LoadValue(Direction, wind.bearing);
  }
}

void
WindSettingsPanel::OnCalculatedUpdate(const MoreData &basic,
                                      const DerivedInfo &calculated)
{
  UpdateVector();
}

