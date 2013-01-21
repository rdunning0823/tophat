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

#include "RouteConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/DataField/Base.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  TurningReach,
  FinalGlideTerrain,
};

class RouteConfigPanel
  : public RowFormWidget, DataFieldListener {
public:
  RouteConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  void ShowReachControls(bool show);

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

void
RouteConfigPanel::ShowReachControls(bool show)
{
  SetRowVisible(FinalGlideTerrain, show);
}

void
RouteConfigPanel::OnModified(DataField &df)
{
if (IsDataField(TurningReach, df)) {
    RoutePlannerConfig::ReachMode mode =
      (RoutePlannerConfig::ReachMode)df.GetAsInteger();
    ShowReachControls(mode != RoutePlannerConfig::ReachMode::OFF);
  }
}

void
RouteConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = XCSoarInterface::GetComputerSettings();
  const RoutePlannerConfig &route_planner = settings_computer.task.route_planner;

  RowFormWidget::Prepare(parent, rc);

  static constexpr StaticEnumChoice turning_reach_list[] = {
    { (unsigned)RoutePlannerConfig::ReachMode::OFF, N_("Off"),
      N_("Reach calculations disabled.") },
    { (unsigned)RoutePlannerConfig::ReachMode::STRAIGHT, N_("Straight"),
      N_("The reach is from straight line paths from the glider.") },
    { (unsigned)RoutePlannerConfig::ReachMode::TURNING, N_("Turning"),
      N_("The reach is calculated allowing turns around terrain obstacles.") },
    { 0 }
  };

  AddEnum(_("Reach mode"),
          _("How calculations are performed of the reach of the glider with respect to terrain."),
          turning_reach_list, (unsigned)route_planner.reach_calc_mode,
          this);

  static constexpr StaticEnumChoice final_glide_terrain_list[] = {
    { FeaturesSettings::FGT_OFF, N_("Off"), N_("Disables the reach display.") },
    { FeaturesSettings::FGT_LINE, N_("Line"), N_("Draws a dashed line at the glide reach.") },
    { FeaturesSettings::FGT_SHADE, N_("Shade"), N_("Shades terrain outside glide reach.") },
    { 0 }
  };

  AddEnum(_("Reach display"), NULL, final_glide_terrain_list,
          settings_computer.features.final_glide_terrain);

  ShowReachControls(route_planner.reach_calc_mode != RoutePlannerConfig::ReachMode::OFF);
}

bool
RouteConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;
  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  RoutePlannerConfig &route_planner = settings_computer.task.route_planner;

  changed |= SaveValueEnum(FinalGlideTerrain, ProfileKeys::FinalGlideTerrain,
                           settings_computer.features.final_glide_terrain);

  changed |= SaveValueEnum(TurningReach, ProfileKeys::TurningReach,
                           route_planner.reach_calc_mode);
  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateRouteConfigPanel()
{
  return new RouteConfigPanel();
}
