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

#include "WaypointRenderer.hpp"
#include "WaypointRendererSettings.hpp"
#include "WaypointIconRenderer.hpp"
#include "WaypointLabelList.hpp"
#include "Projection/MapWindowProjection.hpp"
#include "Computer/Settings.hpp"
#include "Task/Visitors/TaskPointVisitor.hpp"
#include "Engine/Util/Gradient.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "Engine/Waypoint/WaypointVisitor.hpp"
#include "Engine/GlideSolvers/GlideState.hpp"
#include "Engine/GlideSolvers/GlideResult.hpp"
#include "Engine/GlideSolvers/MacCready.hpp"
#include "Engine/Task/AbstractTask.hpp"
#include "Engine/Task/Unordered/UnorderedTaskPoint.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/ProtectedRoutePlanner.hpp"
#include "Screen/Canvas.hpp"
#include "Units/Units.hpp"
#include "Util/StaticArray.hpp"
#include "NMEA/MoreData.hpp"
#include "NMEA/Derived.hpp"
#include "Engine/Route/ReachResult.hpp"
#include "Look/TaskLook.hpp"
#include "Look/MapLook.hpp"
#include "UIGlobals.hpp"
#include "Look/WaypointLook.hpp"
#include "Renderer/LabelArranger.hpp"
#include "Screen/Layout.hpp"

// #define PRINT_AVERAGE_ENERGY /* Displays label energy in label on map */
#ifdef PRINT_AVERAGE_ENERGY
#include "LogFile.hpp"
#include "Util/StringUtil.hpp"
#include "Util/StringFormat.hpp"
#endif

#include <assert.h>
#include <stdio.h>

/**
 * Metadata for a Waypoint that is about to be drawn.
 */
struct VisibleWaypoint {
  const Waypoint *waypoint;

  RasterPoint point;

  ReachResult reach;

  WaypointRenderer::Reachability reachable;

  bool in_task;

  void Set(const Waypoint &_waypoint, RasterPoint &_point,
           bool _in_task) {
    waypoint = &_waypoint;
    point = _point;
    reach.Clear();
    reachable = WaypointRenderer::Unreachable;
    in_task = _in_task;
  }

  void CalculateReachabilityDirect(const MoreData &basic,
                                   const SpeedVector &wind,
                                   const MacCready &mac_cready,
                                   const TaskBehaviour &task_behaviour) {
    assert(basic.location_available);
    assert(basic.NavAltitudeAvailable());

    const fixed elevation = waypoint->elevation +
      task_behaviour.safety_height_arrival;
    const GlideState state(GeoVector(basic.location, waypoint->location),
                           elevation, basic.nav_altitude, wind);

    const GlideResult result = mac_cready.SolveStraight(state);
    if (!result.IsOk())
      return;

    reach.direct = result.pure_glide_altitude_difference;
    if (positive(result.pure_glide_altitude_difference))
      reachable = WaypointRenderer::ReachableTerrain;
  }

  void CalculateRouteArrival(const RoutePlannerGlue &route_planner,
                             const TaskBehaviour &task_behaviour) {
    const RoughAltitude elevation(waypoint->elevation +
                                  task_behaviour.safety_height_arrival);
    const AGeoPoint p_dest (waypoint->location, elevation);
    if (route_planner.FindPositiveArrival(p_dest, reach))
      reach.Subtract(elevation);
  }

  void CalculateReachability(const RoutePlannerGlue &route_planner,
                             const TaskBehaviour &task_behaviour)
  {
    CalculateRouteArrival(route_planner, task_behaviour);

    if (!reach.IsReachableDirect())
      reachable = WaypointRenderer::Unreachable;
    else if (task_behaviour.route_planner.IsReachEnabled() &&
             !reach.IsReachableTerrain())
      reachable = WaypointRenderer::ReachableStraight;
    else
      reachable = WaypointRenderer::ReachableTerrain;
  }

  void DrawSymbol(const struct WaypointRendererSettings &settings,
                  const WaypointLook &look,
                  Canvas &canvas, bool small_icons, Angle screen_rotation) const {
    WaypointIconRenderer wir(settings, look,
                             canvas, small_icons, screen_rotation);
    wir.Draw(*waypoint, point, (WaypointIconRenderer::Reachability)reachable,
             in_task);
  }
};

class WaypointVisitorMap: 
  public WaypointVisitor, 
  public TaskPointConstVisitor
{
  const MapWindowProjection &projection;
  const WaypointRendererSettings &settings;
  const WaypointLook &look;
  const TaskBehaviour &task_behaviour;
  const TaskLook &task_look;
  const MoreData &basic;
  /**
   * is the ordered task a MAT
   */
  bool is_mat;

  TCHAR sAltUnit[4];
  bool task_valid;

  /**
   * A list of waypoints that are going to be drawn.  This list is
   * filled in the Visitor methods.  In the second stage, their
   * reachability is calculated, and the third stage draws them.  This
   * should ensure that the drawing methods don't need to hold a
   * mutex.
   */
  StaticArray<VisibleWaypoint, 256> waypoints;

public:
  WaypointLabelList labels;

public:
  WaypointVisitorMap(const MapWindowProjection &_projection,
                     const WaypointRendererSettings &_settings,
                     const WaypointLook &_look,
                     const TaskBehaviour &_task_behaviour,
                     const MoreData &_basic)
    :projection(_projection),
     settings(_settings), look(_look), task_behaviour(_task_behaviour),
     task_look(UIGlobals::GetMapLook().task),
     basic(_basic),
     is_mat(false),
     task_valid(false)
  {
    if (settings.arrival_height_unit_display) {
      _tcscpy(sAltUnit, Units::GetAltitudeName());
    } else {
      sAltUnit[0] = '\0';
    }
    labels.Init(projection.GetScreenWidth(), projection.GetScreenHeight());
  }

  /**
   * Indicate the ordered task is a MAT
   */
  void SetIsMat(bool v) {
    is_mat = v;
  }

protected:
  void
  FormatTitle(TCHAR* Buffer, const Waypoint &way_point)
  {
    Buffer[0] = _T('\0');

    if (way_point.name.length() >= NAME_SIZE - 20)
      return;

    switch (settings.display_text_type) {
    case WaypointRendererSettings::DisplayTextType::NAME:
      _tcscpy(Buffer, way_point.name.c_str());
      break;

    case WaypointRendererSettings::DisplayTextType::FIRST_FIVE:
      CopyString(Buffer, way_point.name.c_str(), 6);
      break;

    case WaypointRendererSettings::DisplayTextType::FIRST_THREE:
      CopyString(Buffer, way_point.name.c_str(), 4);
      break;

    case WaypointRendererSettings::DisplayTextType::NONE:
      Buffer[0] = '\0';
      break;

    case WaypointRendererSettings::DisplayTextType::FIRST_WORD:
      _tcscpy(Buffer, way_point.name.c_str());
      TCHAR *tmp;
      tmp = _tcsstr(Buffer, _T(" "));
      if (tmp != nullptr)
        tmp[0] = '\0';
      break;

    default:
      assert(0);
      break;
    }
  }


  void
  FormatLabel(TCHAR *buffer, const Waypoint &way_point,
              const ReachResult &reach)
  {
    FormatTitle(buffer, way_point);

    if (!way_point.IsLandable() && !way_point.flags.watched)
      return;

    if (settings.arrival_height_display == WaypointRendererSettings::ArrivalHeightDisplay::REQUIRED_GR) {
      if (!basic.location_available || !basic.NavAltitudeAvailable())
        return;

      const fixed safety_height_gr = task_behaviour.GRSafetyHeight();
      const fixed target_altitude_gr = way_point.elevation + safety_height_gr;
      const fixed delta_h_gr = basic.nav_altitude - target_altitude_gr;
      if (!positive(delta_h_gr))
        /* no L/D if below waypoint */
        return;

      const fixed distance = basic.location.DistanceS(way_point.location);
      const fixed gr = distance / delta_h_gr;
      if (!GradientValid(gr))
        return;

      if (gr > fixed(1)) {
        size_t length = _tcslen(buffer);
        if (length > 0)
          buffer[length++] = _T(' ');
        StringFormatUnsafe(buffer + length, _T("%.0f:1"), (double) gr);
      }
      return;
    }

    if (!reach.IsReachableDirect() && !way_point.flags.watched)
      return;

    if (settings.arrival_height_display == WaypointRendererSettings::ArrivalHeightDisplay::NONE)
      return;

    size_t length = _tcslen(buffer);
    int uah_glide = (int)Units::ToUserAltitude(fixed(reach.direct));
    int uah_terrain = (int)Units::ToUserAltitude(fixed(reach.terrain));

    if (settings.arrival_height_display == WaypointRendererSettings::ArrivalHeightDisplay::TERRAIN) {
      if (reach.IsReachableTerrain()) {
        if (length > 0)
          buffer[length++] = _T(':');
        StringFormatUnsafe(buffer + length, _T("%d%s"), uah_terrain, sAltUnit);
      }
      return;
    }

    if (length > 0)
      buffer[length++] = _T(':');

    if (settings.arrival_height_display == WaypointRendererSettings::ArrivalHeightDisplay::GLIDE_AND_TERRAIN &&
        reach.IsReachableDirect() && reach.IsReachableTerrain() &&
        reach.IsDeltaConsiderable()) {
      StringFormatUnsafe(buffer + length, _T("%d/%d%s"), uah_glide,
                         uah_terrain, sAltUnit);
      return;
    }

    StringFormatUnsafe(buffer + length, _T("%d%s"), uah_glide, sAltUnit);
  }

  /**
   * draw a 1-mile circle around the waypoint
   */
  void
  DrawMatTaskpointOz(Canvas &canvas, const VisibleWaypoint &vwp)
  {
    canvas.Select(task_look.oz_active_pen);
    canvas.SelectHollowBrush();
    RasterPoint p_center = projection.GeoToScreen(vwp.waypoint->location);
    canvas.DrawCircle(p_center.x, p_center.y,
                  projection.GeoToScreenDistance(fixed(1609)));
  }

  void
  DrawWaypoint(Canvas &canvas, const VisibleWaypoint &vwp)
  {
    const Waypoint &way_point = *vwp.waypoint;
    bool watchedWaypoint = way_point.flags.watched;

    vwp.DrawSymbol(settings, look, canvas,
                   projection.GetMapScale() > fixed(8000),
                   projection.GetScreenAngle());
    if (is_mat & vwp.in_task) {
      DrawMatTaskpointOz(canvas, vwp);
    }

    // Determine whether to draw the waypoint label or not
    switch (settings.label_selection) {
    case WaypointRendererSettings::LabelSelection::NONE:
      return;

    case WaypointRendererSettings::LabelSelection::TASK:
      if (!vwp.in_task && task_valid && !watchedWaypoint)
        return;
      break;

    case WaypointRendererSettings::LabelSelection::TASK_AND_AIRFIELD:
      if (!vwp.in_task && task_valid && !watchedWaypoint &&
          !way_point.IsAirport())
        return;
      break;

    case WaypointRendererSettings::LabelSelection::TASK_AND_LANDABLE:
      if (!vwp.in_task && task_valid && !watchedWaypoint &&
          !way_point.IsLandable())
        return;
      break;

    default:
      break;
    }

    TextInBoxMode text_mode;
    bool bold = false;
    if (vwp.reachable != WaypointRenderer::Unreachable &&
        way_point.IsLandable()) {
      text_mode.shape = settings.landable_render_mode;
      bold = true;
      text_mode.move_in_view = true;
    } else if (vwp.in_task) {
      text_mode.shape = LabelShape::ROUNDED_WHITE;
      bold = true;
    } else if (watchedWaypoint) {
      text_mode.shape = LabelShape::OUTLINED;
      text_mode.move_in_view = true;
    }

    TCHAR Buffer[NAME_SIZE+1];
    FormatLabel(Buffer, way_point, vwp.reach);

    RasterPoint sc = vwp.point;
    unsigned enlarged_icon = 0;
    if (vwp.reachable != WaypointRenderer::Unreachable &&
         (settings.landable_style == WaypointRendererSettings::LandableStyle::PURPLE_CIRCLE ||
        settings.vector_landable_rendering))
      // make space for the green circle
      enlarged_icon = Layout::Scale(1);

    unsigned radius = Layout::Scale(2) + enlarged_icon;
    int half_label_height = canvas.CalcTextSize(Buffer).cy / 2;
    labels.Add(vwp.waypoint != nullptr ? vwp.waypoint->id : 0, Buffer,
        sc.x + radius + Layout::Scale(2), sc.y + radius - half_label_height, sc,
        Layout::Scale(4) + enlarged_icon, text_mode, bold,
        vwp.reach.direct, vwp.in_task, way_point.IsLandable(),
        way_point.IsAirport(), watchedWaypoint);
  }

  void AddWaypoint(const Waypoint &way_point, bool in_task) {
    if (waypoints.full())
      return;

    if (!projection.WaypointInScaleFilter(way_point) && !in_task)
      return;

    RasterPoint sc;
    if (!projection.GeoToScreenIfVisible(way_point.location, sc))
      return;

    VisibleWaypoint &vwp = waypoints.append();
    vwp.Set(way_point, sc, in_task);
  }

public:
  void Visit(const Waypoint& way_point) override {
    AddWaypoint(way_point, way_point.IsTurnpoint() && is_mat);
  }

  void Visit(const TaskPoint &tp) override {
    switch (tp.GetType()) {
    case TaskPointType::UNORDERED:
      AddWaypoint(((const UnorderedTaskPoint &)tp).GetWaypoint(), true);
      break;

    case TaskPointType::START:
    case TaskPointType::AST:
    case TaskPointType::AAT:
    case TaskPointType::FINISH:
      AddWaypoint(((const OrderedTaskPoint &)tp).GetWaypoint(), true);
      break;
    }
  }

public:
  void set_task_valid() {
    task_valid = true;
  }

  void CalculateRoute(const ProtectedRoutePlanner &route_planner) {
    const ProtectedRoutePlanner::Lease lease(route_planner);

    for (VisibleWaypoint &vwp : waypoints) {
      const Waypoint &way_point = *vwp.waypoint;

      if (way_point.IsLandable() || way_point.flags.watched)
        vwp.CalculateReachability(lease, task_behaviour);
    }
  }

  void CalculateDirect(const PolarSettings &polar_settings,
                       const TaskBehaviour &task_behaviour,
                       const DerivedInfo &calculated) {
    if (!basic.location_available || !basic.NavAltitudeAvailable())
      return;

    const GlidePolar &glide_polar =
      task_behaviour.route_planner.reach_polar_mode == RoutePlannerConfig::Polar::TASK
      ? polar_settings.glide_polar_task
      : calculated.glide_polar_safety;
    const MacCready mac_cready(task_behaviour.glide, glide_polar);

    for (VisibleWaypoint &vwp : waypoints) {
      const Waypoint &way_point = *vwp.waypoint;

      if (way_point.IsLandable() || way_point.flags.watched)
        vwp.CalculateReachabilityDirect(basic, calculated.GetWindOrZero(),
                                        mac_cready, task_behaviour);
    }
  }

  void Calculate(const ProtectedRoutePlanner *route_planner,
                 const PolarSettings &polar_settings,
                 const TaskBehaviour &task_behaviour,
                 const DerivedInfo &calculated) {
    if (route_planner != nullptr && !route_planner->IsReachEmpty())
      CalculateRoute(*route_planner);
    else
      CalculateDirect(polar_settings, task_behaviour, calculated);
  }

  void Draw(Canvas &canvas) {
    for (const VisibleWaypoint &vwp : waypoints)
      DrawWaypoint(canvas, vwp);
  }
};

#ifdef PRINT_AVERAGE_ENERGY
/**
 * Updates the text of the label to replace the ending letters with its energy
 * @param l. label to be updated
 * @param scaled_energy
 * @return scaled energy for the label
 */
static int
UpdateLabelTextWithEnergy(WaypointLabelList::Label &l)
{
  if (l.energy.energy < 0)
    return 0;

  TCHAR text[NAME_SIZE+1];

  CopyString(text, l.Name, std::max(3, int(StringLength(l.Name) - 5)));
  StringFormat(l.Name, NAME_SIZE, _T("%s.%i"), text, l.energy.ScaledEnergy());

  return l.energy.ScaledEnergy();
}
#endif

static void
DrawLabel(Canvas &canvas, const WaypointLabelList::Label &l,
          UPixelScalar width, UPixelScalar height, const WaypointLook &look,
          LabelBlock &label_block)
{
  canvas.Select(l.bold ? *look.bold_font : *look.font);
  if (TextInBox(canvas, l.Name, l.Pos.x, l.Pos.y, l.Mode,
            width, height, &label_block/*, true*/)) {
//#define DRAW_LINE
#ifdef DRAW_LINE
    RasterPoint corner;
    if (l.GetCornerForLine(corner)) {
      //corner += (l.anchor - l.Pos_arranged);
      canvas.SelectBlackPen();
      canvas.DrawLine(l.Pos + l.anchor - l.Pos_arranged, corner);
    }
#endif
  }
}
/**
 * Merges labels_new into labels last, and redisplays combined list without recalculating.
 * The new labels use their default positions by the anchors
 * The old labels (labels_last) are translated by the delta x,y of the screen.
 * @param recaulculate_labels.  true if the labels should be recalculated using labels_new
 *                      false if labels_new should be merged into labels_last.
 * @param labels_new.  List of labels from most current list of waypoints.  Unsized.
 * @param labels_last.  The list of labels used when last calculated, with energy.
 */
static void
MapWaypointLabelRenderMerge(Canvas &canvas, UPixelScalar width, UPixelScalar height,
                            LabelBlock &label_block,
                            WaypointLabelList &labels_new,
                            WaypointLabelList &labels_last,
                            const WaypointLook &look,
                            const WaypointRendererSettings::DisplayTextType display_text_type)
{
  labels_last.MergeLists(labels_new, false);
  labels_last.Sort();

#ifdef PRINT_AVERAGE_ENERGY
  for (unsigned i = 0; i < labels_last.GetLabels().size(); ++i) {
    WaypointLabelList::Label &l = labels_last.GetLabels()[i]; // non-const ref
#else
    for (const auto &l : labels_last) {
#endif

#ifdef PRINT_AVERAGE_ENERGY
      UpdateLabelTextWithEnergy(l); // so they're displayed on map
#endif
    if (l.Visible()) {
      DrawLabel(canvas, l, width, height, look, label_block);
    }
  }
}

/**
 *
  * @param recaulculate_labels.  true if the labels should be recalculated using labels_new
  *                      false if labels_new should be merged into labels_last.
  * @param labels_new.  List of labels from most current list of waypoints.  Unsized.
  * @param labels_last.  The list of labels used when last calculated, with energy.
  */
static void
MapWaypointLabelRenderCalculate(Canvas &canvas, UPixelScalar width, UPixelScalar height,
                                LabelBlock &label_block,
                                WaypointLabelList &labels_new,
                                WaypointLabelList &labels_last,
                                const WaypointLook &look,
                                const WaypointRendererSettings::DisplayTextType display_text_type)
{
#ifdef PRINT_AVERAGE_ENERGY
  int energy_total = 0;
  unsigned energy_count = 0;
#endif

  labels_last.MergeLists(labels_new, true);
  labels_last.SetSize(canvas, *look.font, *look.bold_font);
  labels_last.SortY();

  Labeler label_arranger(canvas, labels_last);
  label_arranger.Start();
  labels_last.Sort();

  for (unsigned i = 0; i < labels_last.GetLabels().size(); ++i) {
    WaypointLabelList::Label &l = labels_last.GetLabels()[i]; // non-const ref

    if (l.Visible()) {
#ifdef PRINT_AVERAGE_ENERGY // defined above MapWaypointLabelRenderMerge
      energy_total += UpdateLabelTextWithEnergy(l);
      ++energy_count;
#endif
      DrawLabel(canvas, l, width, height, look, label_block);
    }
  }
#ifdef PRINT_AVERAGE_ENERGY
  LogFormat(_T("Calc count:%u Average Energy:%i"), energy_count, energy_total / energy_count);
#endif
}

WaypointRenderer::WaypointRenderer(const Waypoints *_way_points,
                 const WaypointLook &_look)
  :way_points(_way_points), look(_look), angle_margin_degrees(fixed(4))
   {}

void
WaypointRenderer::render(Canvas &canvas, LabelBlock &label_block,
                         const MapWindowProjection &projection,
                         const struct WaypointRendererSettings &settings,
                         const PolarSettings &polar_settings,
                         const TaskBehaviour &task_behaviour,
                         const MoreData &basic, const DerivedInfo &calculated,
                         const ProtectedTaskManager *task,
                         const ProtectedRoutePlanner *route_planner,
                         bool mouse_down)
{
  if (way_points == nullptr || way_points->IsEmpty())
    return;

  WaypointVisitorMap v(projection, settings, look, task_behaviour, basic);

  if (task != nullptr) {
    ProtectedTaskManager::Lease task_manager(*task);

    const TaskStats &task_stats = task_manager->GetStats();

    v.SetIsMat(task_stats.is_mat);

    // task items come first, this is the only way we know that an item is in task,
    // and we won't add it if it is already there
    if (task_stats.task_valid)
      v.set_task_valid();

    const AbstractTask *atask = task_manager->GetActiveTask();
    if (atask != nullptr)
      atask->AcceptTaskPointVisitor(v);
  }

  way_points->VisitWithinRange(projection.GetGeoScreenCenter(),
                                 projection.GetScreenDistanceMeters(), v);

  v.Calculate(route_planner, polar_settings, task_behaviour, calculated);

  v.Draw(canvas);

  Angle delta_angle(map_angle_last - projection.GetScreenAngle());
  PixelSize sample_text_size = canvas.CalcTextSize(_T("WWW"));

  v.labels.Dedupe();
  bool screen_projection_changed = fabs(map_scale_last - projection.GetMapScale()) > fixed(0.1)
      || fabs(delta_angle.Degrees()) > angle_margin_degrees
      || v.labels.GetLabels().size() != labels_last.GetLabels().size()
      || last_sample_text_size != sample_text_size;

  bool recalculate_labels = screen_projection_changed && !mouse_down;

//#define SHOW_LABEL_BORDERS
#ifdef SHOW_LABEL_BORDERS
  for (unsigned i = 0; i < v.labels.GetLabels().size(); ++i)
    v.labels.GetLabels()[i].Mode.shape = LabelShape::ROUNDED_WHITE;
#endif

  if (sample_text_size.cy > 4) {
    if (recalculate_labels) {
      MapWaypointLabelRenderCalculate(canvas,
                                      projection.GetScreenWidth(),
                                      projection.GetScreenHeight(),
                                      label_block, v.labels,
                                      labels_last,
                                      look,
                                      settings.display_text_type);

      map_scale_last = projection.GetMapScale();
      map_angle_last = projection.GetScreenAngle();
      map_geo_center_last = projection.GetGeoScreenCenter();
      last_sample_text_size = sample_text_size;

    } else {
      MapWaypointLabelRenderMerge(canvas,
                                  projection.GetScreenWidth(),
                                  projection.GetScreenHeight(),
                                  label_block, v.labels,
                                  labels_last,
                                  look,
                                  settings.display_text_type);

    }
  }
}
