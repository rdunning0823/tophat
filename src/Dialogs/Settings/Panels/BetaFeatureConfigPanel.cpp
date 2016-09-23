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

#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "BetaFeatureConfigPanel.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "UtilsSettings.hpp"
#include "Computer/Settings.hpp"

enum ControlIndex {
  CirclingMinTurnRate,
  CirclingCruiseClimbSwitch,
  CirclingClimbCruiseSwitch,
};

class BetaFeatureConfigPanel final : public DataFieldListener, public RowFormWidget {
public:
  BetaFeatureConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  virtual void OnModified(DataField &df) override {};
};

void
BetaFeatureConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  const CirclingSettings &settings = CommonInterface::GetComputerSettings().circling;
  AddFloat(_("Circling min turn rate"),
           _("Select the minimum rate of turn (degrees / second) when the system detects turning.  Default=4"),
           _T("%.0f %s"), _T("%.0f"),
           fixed(1), fixed(45), fixed(1), true,
           settings.min_turn_rate.AbsoluteDegrees(),
           this);

  AddFloat(_("Cruise to climb switch"),
           _("Select the number seconds of detected turning before Climb mode is entered. Default=15"),
           _T("%.0f %s"), _T("%.0f"),
           fixed(1), fixed(25), fixed(1), true,
           settings.cruise_climb_switch,
           this);

  AddFloat(_("Climb to cruise switch"),
           _("Select the number seconds of not detecting turning before Cruise mode is entered. Default=10"),
           _T("%.0f %s"), _T("%.0f"),
           fixed(1), fixed(25), fixed(1), true,
           settings.climb_cruise_switch,
           this);

}

bool
BetaFeatureConfigPanel::Save(bool &_changed)
{
  bool changed = false;

/*  fixed min_turn_rate;
  Get(ProfileKeys::CirclingMinTurnRate, min_turn_rate);
  settings.min_turn_rate = settings.min_turn_rate.Degrees(min_turn_rate);
  Get(ProfileKeys::CirclingCruiseClimbSwitch, settings.cruise_climb_switch);
  Get(ProfileKeys::CirclingClimbCruiseSwitch, settings.climb_cruise_switch);*/

  CirclingSettings &settings = CommonInterface::SetComputerSettings().circling;
  fixed min_turn_rate = settings.min_turn_rate.AbsoluteDegrees();
  if (SaveValue(CirclingMinTurnRate, ProfileKeys::CirclingMinTurnRate,
                       min_turn_rate)) {
    changed = true;
    settings.min_turn_rate = Angle::Degrees(min_turn_rate);
  }

  changed |= SaveValue(CirclingCruiseClimbSwitch, ProfileKeys::CirclingCruiseClimbSwitch,
                       settings.cruise_climb_switch);

  changed |= SaveValue(CirclingClimbCruiseSwitch, ProfileKeys::CirclingClimbCruiseSwitch,
                       settings.climb_cruise_switch);

  _changed |= changed;

  return true;
}

Widget *
CreateBetaFeatureConfigPanel()
{
  return new BetaFeatureConfigPanel();
}
