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

#include "MapLook.hpp"
#include "MapSettings.hpp"
#include "Screen/Layout.hpp"
#include "FontDescription.hpp"
#include "Resources.hpp"

void
MapLook::Initialise(const MapSettings &settings,
                    const Font &font, const Font &bold_font,
                    unsigned font_scale_map_waypoint_name,
                    unsigned font_scale_map_place_name)
{
  waypoint.Initialise(settings.waypoint, font_scale_map_waypoint_name);
  aircraft.Initialise();
  task.Initialise();
  trail.Initialise(settings.trail);
  wave.Initialise();
  wind.Initialise(bold_font);

#ifdef HAVE_NOAA
  noaa.Initialise();
#endif

#ifdef HAVE_HATCHED_BRUSH
  above_terrain_bitmap.Load(IDB_ABOVETERRAIN);
  above_terrain_brush.Create(above_terrain_bitmap);
#endif

  terrain_warning_icon.LoadResource(IDB_TERRAINWARNING, IDB_TERRAINWARNING_HD, IDB_TERRAINWARNING_HD2);

    compass_brush.Create(IsDithered() ? COLOR_WHITE : COLOR_GRAY);
  compass_pen.Create(Layout::ScalePenWidth(1),
                     HasColors()? COLOR_GRAY : COLOR_BLACK);
  compass_triangle_brush.Create(IsDithered()
                                ? COLOR_BLACK
                                : Color(50, 50, 50));
  compass_triangle_pen.Create(Layout::ScalePenWidth(1),
                              HasColors() ? COLOR_GRAY : COLOR_BLACK);

  traffic_safe_icon.LoadResource(IDB_TRAFFIC_SAFE, IDB_TRAFFIC_SAFE_HD, IDB_TRAFFIC_SAFE_HD2, false);
  traffic_warning_icon.LoadResource(IDB_TRAFFIC_WARNING, IDB_TRAFFIC_WARNING_HD, IDB_TRAFFIC_WARNING_HD2, false);
  traffic_alarm_icon.LoadResource(IDB_TRAFFIC_ALARM, IDB_TRAFFIC_ALARM_HD, IDB_TRAFFIC_ALARM_HD2, false);

  static constexpr Color clrSepia(0x78,0x31,0x18);
  reach_pen.Create(Pen::DASH, Layout::ScalePenWidth(2), clrSepia);
  reach_pen_thick.Create(Pen::DASH, Layout::ScalePenWidth(3), clrSepia);

  track_line_pen.Create(3, COLOR_GRAY);

  contest_pens[0].Create(Layout::ScalePenWidth(1) + 2, COLOR_RED);
  contest_pens[1].Create(Layout::ScalePenWidth(1) + 1, COLOR_ORANGE);
  contest_pens[2].Create(Layout::ScalePenWidth(1), COLOR_BLUE);

  thermal_source_icon.LoadResource(IDB_THERMALSOURCE, IDB_THERMALSOURCE_HD, IDB_THERMALSOURCE_HD2);

  traffic_safe_icon.LoadResource(IDB_TRAFFIC_SAFE, IDB_TRAFFIC_SAFE_HD, IDB_TRAFFIC_SAFE_HD2, false);
  traffic_warning_icon.LoadResource(IDB_TRAFFIC_WARNING, IDB_TRAFFIC_WARNING_HD, IDB_TRAFFIC_WARNING_HD2, false);
  traffic_alarm_icon.LoadResource(IDB_TRAFFIC_ALARM, IDB_TRAFFIC_ALARM_HD, IDB_TRAFFIC_ALARM_HD2, false);

  map_scale_left_icon.LoadResource(IDB_MAPSCALE_LEFT, IDB_MAPSCALE_LEFT_HD, IDB_MAPSCALE_LEFT_HD2, false);
  map_scale_right_icon.LoadResource(IDB_MAPSCALE_RIGHT, IDB_MAPSCALE_RIGHT_HD, IDB_MAPSCALE_RIGHT_HD2, false);

  cruise_mode_icon.LoadResource(IDB_CRUISE, IDB_CRUISE_HD, IDB_CRUISE_HD2, false);
  climb_mode_icon.LoadResource(IDB_CLIMB, IDB_CLIMB_HD, IDB_CLIMB_HD2, false);
  final_glide_mode_icon.LoadResource(IDB_FINALGLIDE, IDB_FINALGLIDE_HD, IDB_FINALGLIDE_HD2, false);
  abort_mode_icon.LoadResource(IDB_ABORT, IDB_ABORT_HD, IDB_ABORT_HD2, false);

  waiting_for_fix_icon.LoadResource(IDB_GPSSTATUS1, IDB_GPSSTATUS1_HD, IDB_GPSSTATUS1_HD2, false);
  no_gps_icon.LoadResource(IDB_GPSSTATUS2, IDB_GPSSTATUS2_HD, IDB_GPSSTATUS2_HD2, false);

  airspace.Initialise(settings.airspace, topography.important_label_font);
  Reinitialise(settings, font_scale_map_waypoint_name, font_scale_map_place_name);
}

void
MapLook::Reinitialise(const MapSettings &settings,
                      unsigned font_scale_map_waypoint_name,
                      unsigned font_scale_map_place_name)
{
  const FontDescription font_bold_d((Layout::FontScale(18) *
                                         font_scale_map_waypoint_name / 100), true);
  overlay_font.Load(font_bold_d);

  waypoint.Reinitialise(settings.waypoint, font_scale_map_waypoint_name);
  topography.Reinitialise(font_scale_map_place_name);
}
