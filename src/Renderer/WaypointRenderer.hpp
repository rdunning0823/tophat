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

#ifndef XCSOAR_WAY_POINT_RENDERER_HPP
#define XCSOAR_WAY_POINT_RENDERER_HPP

#include "Util/NonCopyable.hpp"
#include "Renderer/WaypointLabelList.hpp"
#include "Math/Angle.hpp"
#include "Geo/GeoPoint.hpp"

struct WaypointRendererSettings;
struct WaypointLook;
class Canvas;
class LabelBlock;
class MapWindowProjection;
class Waypoints;
struct PolarSettings;
struct TaskBehaviour;
struct MoreData;
struct DerivedInfo;
class ProtectedTaskManager;
class ProtectedRoutePlanner;

/**
 * Renders way point icons and labels into a #Canvas.
 */
class WaypointRenderer : private NonCopyable {
  const Waypoints *way_points;

  const WaypointLook &look;

public:
  enum Reachability
  {
    Unreachable,
    ReachableStraight,
    ReachableTerrain,
  };

  WaypointLabelList labels_last;

  /** copied from the map window */
  fixed map_scale_last; /* scale when labels last calculated */
  Angle map_angle_last; /* map angle when labels last calculated */
  const fixed angle_margin_degrees; /* margin when map angle change forces recalc */
  GeoPoint map_geo_center_last; /* map center when labels last calculated */
  PixelSize last_sample_text_size; /* last sample text size */

  WaypointRenderer(const Waypoints *_way_points,
                   const WaypointLook &_look);

  void set_way_points(const Waypoints *_way_points) {
    way_points = _way_points;
  }

  void render(Canvas &canvas, LabelBlock &label_block,
              const MapWindowProjection &projection,
              const WaypointRendererSettings &settings,
              const PolarSettings &polar_settings,
              const TaskBehaviour &task_behaviour,
              const MoreData &basic, const DerivedInfo &calculated,
              const ProtectedTaskManager *task,
              const ProtectedRoutePlanner *route_planner,
              bool mouse_down);

  const WaypointLook &GetLook() const {
    return look;
  }
};

#endif
