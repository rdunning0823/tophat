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

#include "WaypointLook.hpp"
#include "Renderer/WaypointRendererSettings.hpp"
#include "Screen/Layout.hpp"
#include "FontDescription.hpp"
#include "AutoFont.hpp"
#include "Resources.hpp"

void
WaypointLook::Initialise(const WaypointRendererSettings &settings,
                         unsigned font_scale_map_waypoint_name)
{
  small_icon.LoadResource(IDB_SMALL, IDB_SMALL_HD, IDB_SMALL_HD2);
  turn_point_icon.LoadResource(IDB_TURNPOINT, IDB_TURNPOINT_HD, IDB_TURNPOINT_HD2);
  task_turn_point_icon.LoadResource(IDB_TASKTURNPOINT, IDB_TASKTURNPOINT_HD, IDB_TASKTURNPOINT_HD2);
  mountain_top_icon.LoadResource(IDB_MOUNTAIN_TOP, IDB_MOUNTAIN_TOP_HD, IDB_MOUNTAIN_TOP_HD2);
  mountain_pass_icon.LoadResource(IDB_MOUNTAIN_PASS, IDB_MOUNTAIN_PASS_HD, IDB_MOUNTAIN_PASS_HD2);
  bridge_icon.LoadResource(IDB_BRIDGE, IDB_BRIDGE_HD, IDB_BRIDGE_HD2);
  tunnel_icon.LoadResource(IDB_TUNNEL, IDB_TUNNEL_HD, IDB_TUNNEL_HD2);
  tower_icon.LoadResource(IDB_TOWER, IDB_TOWER_HD, IDB_TOWER_HD2);
  power_plant_icon.LoadResource(IDB_POWER_PLANT, IDB_POWER_PLANT_HD, IDB_POWER_PLANT_HD2);
  obstacle_icon.LoadResource(IDB_OBSTACLE, IDB_OBSTACLE_HD, IDB_OBSTACLE_HD2);
  thermal_hotspot_icon.LoadResource(IDB_THERMAL_HOTSPOT, IDB_THERMAL_HOTSPOT_HD, IDB_THERMAL_HOTSPOT_HD2);
  marker_icon.LoadResource(IDB_MARK, IDB_MARK_HD, IDB_MARK_HD2);
  teammate_icon.LoadResource(IDB_TEAMMATE_POS, IDB_TEAMMATE_POS_HD, IDB_TEAMMATE_POS_HD2);
  reachable_brush.Create(COLOR_GREEN);
  terrain_unreachable_brush.Create(LightColor(COLOR_RED));
  unreachable_brush.Create(COLOR_RED);
  white_brush.Create(COLOR_WHITE);
  light_gray_brush.Create(COLOR_LIGHT_GRAY);
  magenta_brush.Create(COLOR_MAGENTA);
  orange_brush.Create(COLOR_ORANGE);

  Reinitialise(settings, font_scale_map_waypoint_name);
}

void
WaypointLook::Reinitialise(const WaypointRendererSettings &settings,
                           unsigned font_scale_map_waypoint_name)
{
  switch (settings.landable_style) {
  case WaypointRendererSettings::LandableStyle::PURPLE_CIRCLE:
    airport_reachable_icon.LoadResource(IDB_REACHABLE, IDB_REACHABLE_HD, IDB_REACHABLE_HD2);
    airport_marginal_icon.LoadResource(IDB_MARGINAL, IDB_MARGINAL_HD, IDB_MARGINAL_HD2);
    airport_unreachable_icon.LoadResource(IDB_LANDABLE, IDB_LANDABLE_HD, IDB_LANDABLE_HD2);
    field_reachable_icon.LoadResource(IDB_REACHABLE, IDB_REACHABLE_HD, IDB_REACHABLE_HD2);
    field_marginal_icon.LoadResource(IDB_MARGINAL, IDB_MARGINAL_HD, IDB_MARGINAL_HD2);
    field_unreachable_icon.LoadResource(IDB_LANDABLE, IDB_LANDABLE_HD, IDB_LANDABLE_HD2);
    break;

  case WaypointRendererSettings::LandableStyle::BW:
    airport_reachable_icon.LoadResource(IDB_AIRPORT_REACHABLE,
                                    IDB_AIRPORT_REACHABLE_HD, IDB_AIRPORT_REACHABLE_HD2);
    airport_marginal_icon.LoadResource(IDB_AIRPORT_MARGINAL,
                                   IDB_AIRPORT_MARGINAL_HD, IDB_AIRPORT_MARGINAL_HD2);
    airport_unreachable_icon.LoadResource(IDB_AIRPORT_UNREACHABLE,
                                      IDB_AIRPORT_UNREACHABLE_HD, IDB_AIRPORT_UNREACHABLE_HD2);
    field_reachable_icon.LoadResource(IDB_OUTFIELD_REACHABLE,
                                  IDB_OUTFIELD_REACHABLE_HD, IDB_OUTFIELD_REACHABLE_HD2);
    field_marginal_icon.LoadResource(IDB_OUTFIELD_MARGINAL,
                                 IDB_OUTFIELD_MARGINAL_HD, IDB_OUTFIELD_MARGINAL_HD2);
    field_unreachable_icon.LoadResource(IDB_OUTFIELD_UNREACHABLE,
                                    IDB_OUTFIELD_UNREACHABLE_HD, IDB_OUTFIELD_UNREACHABLE_HD2);
    break;

  case WaypointRendererSettings::LandableStyle::TRAFFIC_LIGHTS:
    airport_reachable_icon.LoadResource(IDB_AIRPORT_REACHABLE,
                                    IDB_AIRPORT_REACHABLE_HD, IDB_AIRPORT_REACHABLE_HD2);
    airport_marginal_icon.LoadResource(IDB_AIRPORT_MARGINAL2,
                                   IDB_AIRPORT_MARGINAL2_HD, IDB_AIRPORT_MARGINAL2_HD2);
    airport_unreachable_icon.LoadResource(IDB_AIRPORT_UNREACHABLE2,
                                      IDB_AIRPORT_UNREACHABLE2_HD, IDB_AIRPORT_UNREACHABLE2_HD2);
    field_reachable_icon.LoadResource(IDB_OUTFIELD_REACHABLE,
                                  IDB_OUTFIELD_REACHABLE_HD, IDB_OUTFIELD_REACHABLE_HD2);
    field_marginal_icon.LoadResource(IDB_OUTFIELD_MARGINAL2,
                                 IDB_OUTFIELD_MARGINAL2_HD, IDB_OUTFIELD_MARGINAL2_HD2);
    field_unreachable_icon.LoadResource(IDB_OUTFIELD_UNREACHABLE2,
                                    IDB_OUTFIELD_UNREACHABLE2_HD, IDB_OUTFIELD_UNREACHABLE2_HD2);
    break;
  }
  const FontDescription font_d((Layout::FontScale(18) *
                                         font_scale_map_waypoint_name) / 100);
  const FontDescription font_bold_d((Layout::FontScale(18) *
                                         font_scale_map_waypoint_name / 100), true);
  font_instance.Load(font_d);
  bold_font_instance.Load(font_bold_d);
  font = & font_instance;
  bold_font = &bold_font_instance;
}
