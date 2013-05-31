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

#ifndef XCSOAR_MAP_WINDOW_HPP
#define XCSOAR_MAP_WINDOW_HPP

#include "Projection/MapWindowProjection.hpp"
#include "MapWindowTimer.hpp"
#include "Renderer/AirspaceRenderer.hpp"
#include "Screen/DoubleBufferWindow.hpp"
#ifndef ENABLE_OPENGL
#include "Screen/BufferCanvas.hpp"
#endif
#include "Screen/LabelBlock.hpp"
#include "Screen/StopWatch.hpp"
#include "MapWindowBlackboard.hpp"
#include "Renderer/BackgroundRenderer.hpp"
#include "Renderer/WaypointRenderer.hpp"
#include "Renderer/TrailRenderer.hpp"
#include "Compiler.h"
#include "Weather/Features.hpp"

struct MapLook;
struct TrafficLook;
class TopographyStore;
class TopographyRenderer;
class RasterTerrain;
class RasterWeather;
class ProtectedMarkers;
class Waypoints;
struct Waypoint;
class Airspaces;
class ProtectedTaskManager;
class GlideComputer;
class GlidePolar;
class ContainerWindow;
class WaypointLabelList;
class NOAAStore;

class MapWindow :
  public DoubleBufferWindow,
  public MapWindowBlackboard,
  public MapWindowTimer
{
#ifndef ENABLE_OPENGL
  // graphics vars

  BufferCanvas buffer_canvas;
  BufferCanvas stencil_canvas;
#endif

  LabelBlock label_block;

protected:
  const MapLook &look;

  /**
   * What object does the projection's screen origin follow?
   */
  enum FollowMode {
    /**
     * Follow the user's aircraft.
     */
    FOLLOW_SELF,

    /**
     * The user pans the map.
     */
    FOLLOW_PAN,
  };

  FollowMode follow_mode;

  /**
   * The projection as currently visible on the screen.  This object
   * is being edited by the user.
   */
  MapWindowProjection visible_projection;

#ifndef ENABLE_OPENGL
  /**
   * The projection of the buffer.  This differs from
   * visible_projection only after the projection was modified, until
   * the DrawThread has finished drawing the new projection.
   */
  MapWindowProjection buffer_projection;
#endif

  /**
   * The projection of the DrawThread.  This is used to render the new
   * map.  After rendering has completed, this object is copied over
   * #buffer_projection.
   */
  MapWindowProjection render_projection;

  const Waypoints *waypoints;
  TopographyStore *topography;
  TopographyRenderer *topography_renderer;

  RasterTerrain *terrain;
  GeoPoint terrain_center;
  fixed terrain_radius;

  RasterWeather *weather;

  const TrafficLook &traffic_look;

  BackgroundRenderer background;
  WaypointRenderer waypoint_renderer;

  AirspaceRenderer airspace_renderer;

  TrailRenderer trail_renderer;

  ProtectedTaskManager *task;
  const ProtectedRoutePlanner *route_planner;
  GlideComputer *glide_computer;

  ProtectedMarkers *marks;

#ifdef HAVE_NOAA
  NOAAStore *noaa_store;
#endif

  bool compass_visible;

  /**
   * distance from the top of the map where the compass is displayed
   */
  unsigned compass_offset_y;

  /**
   * distance from the bottom of the map where the GPSStatus is displayed
   */
  unsigned gps_status_offset_y;

#ifndef ENABLE_OPENGL
  /**
   * Tracks whether the buffer canvas contains valid data.  We use
   * those attributes to prevent showing invalid data on the map, when
   * the user switches quickly to or from full-screen mode.
   */
  unsigned ui_generation, buffer_generation;

  /**
   * If non-zero, then the buffer will be scaled to the new
   * projection, and this variable is decremented.  This is used while
   * zooming and panning, to give instant visual feedback.
   */
  unsigned scale_buffer;
#endif

  /**
   * The #StopWatch used to benchmark the DrawThread,
   * i.e. OnPaintBuffer().
   */
  ScreenStopWatch draw_sw;

  friend class DrawThread;

public:
  MapWindow(const MapLook &look,
            const TrafficLook &traffic_look);
  virtual ~MapWindow();

  /**
   * Is the rendered map following the user's aircraft (i.e. near it)?
   */
  bool IsNearSelf() const {
    return follow_mode == FOLLOW_SELF;
  }

  /**
   * @param y. The distance from the top of the map where the compass
   * will display
   */
  void SetCompassOffset(unsigned y) {
    compass_offset_y = y;
  }

  /**
   * @param y. The distance from the bottom of the map where the
   * GPSStatus will display
   */
  void SetGPSStatusOffset(unsigned y) {
    gps_status_offset_y = y;
  }
#ifndef ENABLE_OPENGL
  /**
   * resizes the rc_main_menu_button for the current screen layout
   */
  virtual void SetMainMenuButtonRect() {};
  /**
   * resizes the rc_zoom_in_button and rc_zoom_out_button for the current
   * screen layout
   */
  virtual void SetZoomButtonsRect() {};
#endif

  bool IsPanning() const {
    return follow_mode == FOLLOW_PAN;
  }

  virtual void set(ContainerWindow &parent, const PixelRect &rc);

  void SetWaypoints(const Waypoints *_waypoints) {
    waypoints = _waypoints;
    waypoint_renderer.set_way_points(waypoints);
  }

  void SetTask(ProtectedTaskManager *_task) {
    task = _task;
  }

  void SetRoutePlanner(const ProtectedRoutePlanner *_route_planner) {
    route_planner = _route_planner;
  }

  void SetGlideComputer(GlideComputer *_gc);

  void SetAirspaces(Airspaces *airspaces) {
    airspace_renderer.SetAirspaces(airspaces);
  }

  void SetTopography(TopographyStore *_topography);
  void SetTerrain(RasterTerrain *_terrain);
  void SetWeather(RasterWeather *_weather);

  void SetMarks(ProtectedMarkers *_marks) {
    marks = _marks;
  }

#ifdef HAVE_NOAA
  void SetNOAAStore(NOAAStore *_noaa_store) {
    noaa_store = _noaa_store;
  }
#endif

  void ReadBlackboard(const MoreData &nmea_info,
                      const DerivedInfo &derived_info);

  void ReadBlackboard(const MoreData &nmea_info,
                      const DerivedInfo &derived_info,
                      const ComputerSettings &settings_computer,
                      const MapSettings &settings_map);

  const MapWindowProjection &VisibleProjection() const {
    return visible_projection;
  }

  gcc_pure
  const GeoPoint &GetLocation() const {
    return visible_projection.GetGeoLocation();
  }

  void SetLocation(const GeoPoint location) {
    visible_projection.SetGeoLocation(location);
  }

public:
  void DrawBestCruiseTrack(Canvas &canvas,
                           const RasterPoint aircraft_pos) const;
  void DrawTrackBearing(Canvas &canvas,
                        const RasterPoint aircraft_pos) const;
  void DrawCompass(Canvas &canvas, const PixelRect &rc) const;
  void DrawWind(Canvas &canvas, const RasterPoint &Orig,
                           const PixelRect &rc) const;
  void DrawWaypoints(Canvas &canvas);

  void DrawTrail(Canvas &canvas, const RasterPoint aircraft_pos,
                 unsigned min_time, bool enable_traildrift = false);
  virtual void RenderTrail(Canvas &canvas, const RasterPoint aircraft_pos);
  void DrawTeammate(Canvas &canvas) const;
  void DrawTask(Canvas &canvas);
  void DrawRoute(Canvas &canvas);
  void DrawTaskOffTrackIndicator(Canvas &canvas);
  virtual void DrawThermalEstimate(Canvas &canvas) const;

  void DrawGlideThroughTerrain(Canvas &canvas) const;
  void DrawTerrainAbove(Canvas &canvas);
  void DrawFLARMTraffic(Canvas &canvas, const RasterPoint aircraft_pos) const;

  // thread, main functions
  /**
   * Renders all the components of the moving map
   * @param canvas The drawing canvas
   * @param rc The area to draw in
   */
  virtual void Render(Canvas &canvas, const PixelRect &rc);

protected:
  unsigned UpdateTopography(unsigned max_update=1024);

  /**
   * @return true if UpdateTerrain() should be called again
   */
  bool UpdateTerrain();

  /**
   * @return true if UpdateWeather() should be called again
   */
  bool UpdateWeather();

  void UpdateAll() {
    UpdateTopography();
    UpdateTerrain();
    UpdateWeather();
  }

protected:
  virtual void OnCreate();
  virtual void OnDestroy();
  virtual void OnResize(UPixelScalar width, UPixelScalar height);

  virtual void OnPaint(Canvas& canvas);
  virtual void OnPaintBuffer(Canvas& canvas);

private:
  /**
   * Renders the terrain background
   * @param canvas The drawing canvas
   */
  void RenderTerrain(Canvas &canvas);
  /**
   * Renders the topography
   * @param canvas The drawing canvas
   */
  void RenderTopography(Canvas &canvas);
  /**
   * Renders the topography labels
   * @param canvas The drawing canvas
   */
  void RenderTopographyLabels(Canvas &canvas);
  /**
   * Renders the final glide shading
   * @param canvas The drawing canvas
   */
  void RenderFinalGlideShading(Canvas &canvas);
  /**
   * Renders the airspace
   * @param canvas The drawing canvas
   */
  void RenderAirspace(Canvas &canvas);
  /**
   * Renders the markers
   * @param canvas The drawing canvas
   */
  void RenderMarkers(Canvas &canvas);
  /**
   * Renders the NOAA stations
   * @param canvas The drawing canvas
   */
  void RenderNOAAStations(Canvas &canvas);
  /**
   * Render final glide through terrain marker
   * @param canvas The drawing canvas
   */
  void RenderGlide(Canvas &canvas);

public:
  void SetMapScale(const fixed x);
};

#endif
