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

#include "GlueMapWindow.hpp"
#include "Look/MapLook.hpp"
#include "Look/TaskLook.hpp"
#include "Look/GlobalFonts.hpp"
#include "Look/IconLook.hpp"
#include "Look/InfoBoxLook.hpp"
#include "UIGlobals.hpp"
#include "Screen/Icon.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "Logger/Logger.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Renderer/TextInBox.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Engine/Task/Points/TaskWaypoint.hpp"
#include "Screen/UnitSymbol.hpp"
#include "Terrain/RasterWeather.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/UserGeoPointFormatter.hpp"
#include "UIState.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "Util/Macros.hpp"
#include "Util/Clamp.hpp"
#include "Look/GestureLook.hpp"
#include "Input/InputEvents.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Widgets/MapOverlayButton.hpp"
#include "Util/StaticString.hpp"
#include "Components.hpp"
#include "GlideSolvers/MacCready.hpp"
#include "GlideSolvers/GlideState.hpp"
#ifndef ENABLE_OPENGL
#include "Engine/Task/Factory/TaskFactoryType.hpp"
#endif

#include <stdio.h>

void
GlueMapWindow::DrawGesture(Canvas &canvas) const
{
  if (!gestures.HasPoints())
    return;

  const TCHAR *gesture = gestures.GetGesture();
  if (gesture != NULL && !InputEvents::IsGesture(gesture))
    canvas.Select(gesture_look.invalid_pen);
  else
    canvas.Select(gesture_look.pen);

  canvas.SelectHollowBrush();

  const auto &points = gestures.GetPoints();
  auto it = points.begin();
  auto it_last = it++;
  for (auto it_end = points.end(); it != it_end; it_last = it++)
    canvas.DrawLinePiece(*it_last, *it);
}

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
/**
 * DrawTwoLines seems to always use pen width = 1,
 * we we'll draw consecutive rectangles to emulate thicker pen
 */
static void
DrawRect(Canvas &canvas, const PixelRect &rc)
{
  canvas.DrawTwoLines(rc.left, rc.bottom,
                      rc.right, rc.bottom,
                      rc.right, rc.top);
  canvas.DrawTwoLines(rc.left, rc.bottom,
                      rc.left, rc.top,
                      rc.right, rc.top);
  PixelRect rc1 {rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1 };

  canvas.DrawTwoLines(rc1.left, rc1.bottom,
                      rc1.right, rc1.bottom,
                      rc1.right, rc1.top);
  canvas.DrawTwoLines(rc1.left, rc1.bottom,
                      rc1.left, rc1.top,
                      rc1.right, rc1.top);
}

void
GlueMapWindow::DrawMainMenuButtonOverlay(Canvas &canvas) const
{
  const IconLook &icons = UIGlobals::GetIconLook();
  const Bitmap *bmp = &icons.hBmpMenuButton;
  const PixelSize bitmap_size = bmp->GetSize();

  UPixelScalar pen_width = !HasColors() ? 2 : 1;
  canvas.Select(Pen((UPixelScalar)Layout::Scale(pen_width), COLOR_BLACK));
  DrawRect(canvas, rc_main_menu_button);

  const UPixelScalar menu_button_height = rc_main_menu_button.bottom -
      rc_main_menu_button.top;
  const UPixelScalar menu_button_width = rc_main_menu_button.right -
      rc_main_menu_button.left;

  const int offsetx = (menu_button_width - bitmap_size.cx / 2) / 2;
  const int offsety = (menu_button_height - bitmap_size.cy) / 2;
  canvas.CopyAnd(rc_main_menu_button.left + offsetx,
                 rc_main_menu_button.top + offsety,
                  bitmap_size.cx / 2,
                  bitmap_size.cy,
                  *bmp,
                  bitmap_size.cx / 2, 0);
}

void
GlueMapWindow::DrawZoomButtonOverlays(Canvas &canvas) const
{

  UPixelScalar pen_width = !HasColors() ? 2 : 1;
  canvas.Select(Pen((UPixelScalar)Layout::Scale(pen_width), COLOR_BLACK));
  DrawRect(canvas, rc_zoom_out_button);
  DrawRect(canvas, rc_zoom_in_button);

  const IconLook &icon_look = UIGlobals::GetIconLook();
  const Bitmap *bmp_zoom_in = &icon_look.hBmpZoomInButton;
  const PixelSize bitmap_in_size = bmp_zoom_in->GetSize();
  const int offsetx = (rc_zoom_in_button.right - rc_zoom_in_button.left - bitmap_in_size.cx / 2) / 2;
  const int offsety = (rc_zoom_in_button.bottom - rc_zoom_in_button.top - bitmap_in_size.cy) / 2;
  canvas.CopyAnd(rc_zoom_in_button.left + offsetx,
                  rc_zoom_in_button.top + offsety,
                  bitmap_in_size.cx / 2,
                  bitmap_in_size.cy,
                  *bmp_zoom_in,
                  bitmap_in_size.cx / 2, 0);

  const Bitmap *bmp_zoom_out = &icon_look.hBmpZoomOutButton;
  const PixelSize bitmap_out_size = bmp_zoom_out->GetSize();
  canvas.CopyAnd(rc_zoom_out_button.left + offsetx,
                  rc_zoom_out_button.top + offsety,
                  bitmap_out_size.cx / 2,
                  bitmap_out_size.cy,
                  *bmp_zoom_out,
                  bitmap_out_size.cx / 2, 0);

}

void
GlueMapWindow::DrawTaskNavSliderShape(Canvas &canvas)
{
  TaskWaypoint *tp;
  TaskType task_mode = TaskType::GOTO;
  TaskFactoryType task_factory_type = TaskFactoryType::RACING;
  const OrderedTask *ot;
  const Waypoint *wp;
  if (true) {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);
    ot = &task_manager->GetOrderedTask();
    tp = task_manager->GetActiveTaskPoint();
    task_mode = task_manager->GetMode();
    task_factory_type = task_manager->GetOrderedTask().GetFactoryType();
  }
  StaticString<255> wp_name(_T(""));
  unsigned task_size = 1;
  bool has_entered, has_exited;
  unsigned idx = 0;
  bool tp_valid = tp != nullptr;

  if (task_mode == TaskType::ORDERED) {
    task_size = ot->TaskSize();
    idx = ot->GetActiveIndex();
    const OrderedTaskPoint *otp = &ot->GetTaskPoint(idx);
    if (otp != nullptr) {
      has_entered = otp->HasEntered();
      has_exited = otp->HasExited();
      wp_name = otp->GetWaypoint().name.c_str();
      wp = &otp->GetWaypoint();
      tp_valid = true;
    }
  } else {
    if (tp != nullptr) {
      wp_name = tp->GetWaypoint().name.c_str();
      wp = &tp->GetWaypoint();
    }
  }
  bool distance_valid = false, altitude_difference_valid = false,
      bearing_valid = false;
  fixed distance = fixed(0), altitude_difference = fixed(0);
  Angle bearing;
  if (wp != nullptr) {
    const MoreData &more_data = Basic();
    // altitude differential
    if (Basic().location_available && more_data.NavAltitudeAvailable() &&
        GetComputerSettings().polar.glide_polar_task.IsValid()) {
      const GlideState glide_state(
          Basic().location.DistanceBearing(wp->location),
        wp->elevation + GetComputerSettings().task.safety_height_arrival,
        more_data.nav_altitude,
        Calculated().GetWindOrZero());

      const GlideResult &result =
        MacCready::Solve(GetComputerSettings().task.glide,
                         GetComputerSettings().polar.glide_polar_task,
                         glide_state);
      altitude_difference =
          result.SelectAltitudeDifference(GetComputerSettings().task.glide);
      altitude_difference_valid = true;
    }

    // New dist & bearing
    if (Basic().location_available) {
      const GeoVector vector = Basic().location.DistanceBearing(wp->location);
      distance = vector.distance;
      distance_valid = true;
      bearing = vector.bearing;
      bearing_valid = true;
    }

  }
  const TerrainRendererSettings &terrain = GetMapSettings().terrain;
  unsigned border_width = Layout::ScalePenWidth(terrain.enable ? 1 : 2);
  PixelRect outer_rect = slider_shape.GetOuterRect();
  UPixelScalar x_offset = (GetClientRect().right - GetClientRect().left -
      (outer_rect.right - outer_rect.left)) / 2;
  outer_rect.Offset(x_offset, 0);

  slider_shape.Draw(canvas, outer_rect,
                    idx, false, false,
                    wp_name.c_str(),
                    has_entered, has_exited,
                    task_mode,
                    task_factory_type,
                    task_size,
                    tp_valid, distance, distance_valid,
                    altitude_difference,
                    altitude_difference_valid,
                    bearing,
                    bearing_valid,
                    border_width);
}
#endif

void
GlueMapWindow::DrawCrossHairs(Canvas &canvas) const
{
  if (!render_projection.IsValid())
    return;

  Pen pen(Layout::Scale(2), COLOR_BLACK);
  canvas.Select(pen);

  const RasterPoint center = render_projection.GetScreenOrigin();
  const PixelScalar length = Layout::Scale(20);

  canvas.DrawLine(center.x + length, center.y,
              center.x - length, center.y);
  canvas.DrawLine(center.x, center.y + length,
              center.x, center.y - length);
}

void
GlueMapWindow::DrawPanInfo(Canvas &canvas) const
{
  if (!render_projection.IsValid())
    return;

  GeoPoint location = render_projection.GetGeoLocation();

  TextInBoxMode mode;
//  mode.shape = LabelShape::OUTLINED;
  mode.shape = LabelShape::FILLED;
  mode.align = TextInBoxMode::Alignment::RIGHT;

  const Font &font = *look.overlay_font;
  canvas.Select(font);

  UPixelScalar padding = Layout::FastScale(4);
  UPixelScalar height = font.GetHeight();
  PixelScalar y = 0 + padding;
  PixelScalar x = render_projection.GetScreenWidth() - padding;

  if (compass_visible)
    /* don't obscure the north arrow */
    /* TODO: obtain offset from CompassRenderer */
    y += Layout::Scale(19) + Layout::FastScale(13);

  if (terrain) {
    short elevation = terrain->GetTerrainHeight(location);
    if (!RasterBuffer::IsSpecial(elevation)) {
      StaticString<64> elevation_short, elevation_long;
      FormatUserAltitude(fixed(elevation),
                                elevation_short.buffer(), elevation_short.MAX_SIZE);

      elevation_long = _("Elevation: ");
      elevation_long += elevation_short;

      TextInBox(canvas, elevation_long, x, y, mode,
                render_projection.GetScreenWidth(),
                render_projection.GetScreenHeight());

      y += height;
    }
  }

  if (Basic().location_available) {
    fixed dist = location.Distance(Basic().location);
    StaticString<64> distance_short;
    StaticString<64> distance_long;
    FormatUserDistance(dist, distance_short.buffer(), true, 1);
    distance_long.Format(_T("%s: %s"), _("Distance"), distance_short.c_str());
    TextInBox(canvas, distance_long, x, y, mode,
              render_projection.GetScreenWidth(),
              render_projection.GetScreenHeight());

    y += height;
  }

  TCHAR buffer[256];
  FormatGeoPoint(location, buffer, ARRAY_SIZE(buffer), _T('\n'));

  TCHAR *start = buffer;
  while (true) {
    TCHAR *newline = _tcschr(start, _T('\n'));
    if (newline != NULL)
      *newline = _T('\0');

    TextInBox(canvas, start, x, y, mode,
              render_projection.GetScreenWidth(),
              render_projection.GetScreenHeight());

    y += height;

    if (newline == NULL)
      break;

    start = newline + 1;
  }
}

void
GlueMapWindow::DrawGPSStatus(Canvas &canvas, const PixelRect &rc_unadjusted,
                             const NMEAInfo &info) const
{
  const TCHAR *txt;
  const MaskedIcon *icon;

  if (!info.alive) {
    icon = &look.no_gps_icon;
    txt = _("GPS not connected");
  } else if (!info.location_available) {
    icon = &look.waiting_for_fix_icon;
    txt = _("GPS waiting for fix");
  } else
    // early exit
    return;

  PixelRect rc = rc_unadjusted;
  rc.top -= gps_status_offset_y;
  rc.bottom -= gps_status_offset_y;

  PixelScalar x = rc.left + Layout::FastScale(2);
  PixelScalar y = rc.bottom - Layout::FastScale(2) - icon->GetSize().cy;
  icon->Draw(canvas, x, y);

  x += icon->GetSize().cx + Layout::FastScale(4);
  y += Layout::Scale(1);

  TextInBoxMode mode;
  mode.shape = LabelShape::ROUNDED_BLACK;

  const Font &font = *look.overlay_font;
  canvas.Select(font);
  TextInBox(canvas, txt, x, y, mode, rc, NULL);
}


void
GlueMapWindow::DrawMapScale(Canvas &canvas, const PixelRect &rc,
                            const MapWindowProjection &projection) const
{
  StaticString<80> buffer;

  fixed map_width = projection.GetScreenWidthMeters();

  canvas.Select(Fonts::map_bold);
  FormatUserMapScale(map_width, buffer.buffer(), true);
  PixelSize text_size = canvas.CalcTextSize(buffer);
  const PixelScalar text_padding_x = Layout::Scale(2);
  PixelSize bmp_size = look.map_scale_left_icon.GetSize();
  const PixelScalar bmp_y = (text_size.cy + bmp_size.cy) / 2;

  UPixelScalar zoom_button_width = MapOverlayButton::GetStandardButtonHeight()
      * MapOverlayButton::GetScale() + MapOverlayButton::GetClearBorderWidth();

  PixelScalar x = rc.left + (Layout::landscape ? 0 : 1) * zoom_button_width;

  canvas.DrawFilledRectangle(x, rc.bottom - text_size.cy - Layout::Scale(1),
                             x + 2 * bmp_size.cx + text_size.cx + Layout::Scale(2),
                             rc.bottom, COLOR_WHITE);

  look.map_scale_left_icon.Draw(canvas, x, rc.bottom - bmp_y);
  x += bmp_size.cx;

  canvas.SetBackgroundColor(COLOR_WHITE);
  canvas.SetBackgroundOpaque();
  canvas.SetTextColor(COLOR_BLACK);
  canvas.DrawText(x, rc.bottom - text_size.cy - Layout::Scale(1),
              buffer);

  x += text_padding_x + text_size.cx;
  look.map_scale_right_icon.Draw(canvas, x, rc.bottom - bmp_y);
}

void
GlueMapWindow::DrawFlightMode(Canvas &canvas, const PixelRect &rc,
                              unsigned nav_slider_bar_visible_height) const
{
  PixelSize button_size;
  button_size.cx = MapOverlayButton::GetStandardButtonHeight()
      * MapOverlayButton::GetScale() + MapOverlayButton::GetClearBorderWidth();
  button_size.cy = button_size.cx;

  // draw flight mode
  const MaskedIcon *bmp;

  if (Calculated().common_stats.task_type == TaskType::ABORT)
    bmp = &look.abort_mode_icon;
  else if (GetDisplayMode() == DisplayMode::CIRCLING)
    bmp = &look.climb_mode_icon;
  else if (GetDisplayMode() == DisplayMode::FINAL_GLIDE)
    bmp = &look.final_glide_mode_icon;
  else
    bmp = &look.cruise_mode_icon;

  unsigned offset = button_size.cx + Layout::Scale(2) + bmp->GetSize().cx;
  bmp->Draw(canvas, rc.right - offset,
            rc.bottom - bmp->GetSize().cy - Layout::Scale(1));

  // draw "Simulator/Replay
  StaticString<80> buffer;
  const UIState &ui_state = GetUIState();

  canvas.SetBackgroundOpaque();
  canvas.SetBackgroundColor(COLOR_WHITE);
  canvas.SetTextColor(COLOR_BLACK);

  buffer.clear();

  if (Basic().gps.replay)
    buffer += _T("REPLAY");
  else if (Basic().gps.simulator) {
    buffer += _("Simulator");
  }

  if (GetComputerSettings().polar.ballast_timer_active)
    buffer.AppendFormat(
        _T(" %s %d %s"), _("Ballast"),
        (int)GetComputerSettings().polar.glide_polar_task.GetBallastLitres(),
        _("Liters"));

  if (!buffer.empty()) {
    TextInBoxMode mode;
    mode.shape = LabelShape::FILLED;

    const Font &font = *UIGlobals::GetLook().dialog.button.font;
    PixelSize text_size = font.TextSize(buffer.c_str());
    canvas.Select(font);
    unsigned x = (rc.right - rc.left - text_size.cx) / 2;
    unsigned y = rc.top + nav_slider_bar_visible_height;
    TextInBox(canvas, buffer.c_str(), x, y, mode, rc, NULL);
  }

  return; // don't show rest of stuff

  canvas.Select(Fonts::title);
  if (weather != NULL && weather->GetParameter() > 0) {
    const TCHAR *label = weather->ItemLabel(weather->GetParameter());
    if (label != NULL) {
      buffer += _T(" ");
      buffer += label;
    }
  }

  if (ui_state.auxiliary_enabled) {
    if (!buffer.empty())
      buffer += _T(", ");
    buffer += ui_state.panel_name;
  }

  if (!buffer.empty()) {
    offset += canvas.CalcTextWidth(buffer) + Layout::Scale(2);

    canvas.DrawText(rc.right - offset, rc.bottom - canvas.CalcTextSize(buffer).cy, buffer);
  }

  const PolarSettings &polar_settings = GetComputerSettings().polar;
  // calc "Ballast" and draw above sim/replay info
  if (((int)polar_settings.glide_polar_task.GetBallastLitres() > 0 ||
      !Calculated().flight.flying)
      && polar_settings.glide_polar_task.IsBallastable()) {
    buffer = _("Ballast");
    buffer.AppendFormat(
      _T(" %d L"),
      (int)computer_settings.polar.glide_polar_task.GetBallastLitres());

    canvas.DrawText(rc.right - offset, rc.bottom - 2 * canvas.CalcTextSize(buffer).cy, buffer);
  }
}

void
GlueMapWindow::DrawFinalGlide(Canvas &canvas, const PixelRect &rc) const
{
  StaticString<64> description;

  if (GetMapSettings().final_glide_bar_display_mode ==
      FinalGlideBarDisplayMode::OFF)
    return;


  ProtectedTaskManager::Lease task_manager(*task);
  if (task_manager->GetMode() == TaskType::ORDERED)
    description = _("Task");
  else {
    const TaskWaypoint* wp = task_manager->GetActiveTaskPoint();
    if (wp != nullptr) {
      description = wp->GetWaypoint().name.c_str();
      if (description == _T("(takeoff)"))
        description = _T("TakeOff");
    } else
      description = _T("");
  }


  final_glide_bar_renderer.Draw(canvas, rc, Calculated(),
                                GetComputerSettings().task.glide,
                                GetMapSettings().final_glide_bar_mc0_enabled,
                                description.c_str());
}

void
GlueMapWindow::DrawVario(Canvas &canvas, const PixelRect &rc) const
{
  if (!GetMapSettings().vario_bar_enabled)
   return;

  vario_bar_renderer.Draw(canvas, rc, Basic(), Calculated(),
                          GetComputerSettings().polar.glide_polar_task,
                          true); //NOTE: AVG enabled for now, make it configurable ;

}

void
GlueMapWindow::DrawThermalEstimate(Canvas &canvas) const
{
  if (InCirclingMode() && IsNearSelf()) {
    // in circling mode, draw thermal at actual estimated location
    const MapWindowProjection &projection = render_projection;
    const ThermalLocatorInfo &thermal_locator = Calculated().thermal_locator;
    if (thermal_locator.estimate_valid) {
      RasterPoint sc;
      if (projection.GeoToScreenIfVisible(thermal_locator.estimate_location, sc)) {
        look.thermal_source_icon.Draw(canvas, sc);
      }
    }
  } else {
    MapWindow::DrawThermalEstimate(canvas);
  }
}

void
GlueMapWindow::RenderTrail(Canvas &canvas, const RasterPoint aircraft_pos)
{
  unsigned min_time;
  switch(GetMapSettings().trail.length) {
  case TrailSettings::Length::OFF:
    return;
  case TrailSettings::Length::LONG:
    min_time = std::max(0, (int)Basic().time - 3600);
    break;
  case TrailSettings::Length::SHORT:
    min_time = std::max(0, (int)Basic().time - 600);
    break;
  case TrailSettings::Length::FULL:
  default:
    min_time = 0; // full
    break;
  }

  DrawTrail(canvas, aircraft_pos, min_time,
            GetMapSettings().trail.wind_drift_enabled && InCirclingMode());
}

void
GlueMapWindow::RenderTrackBearing(Canvas &canvas, const RasterPoint aircraft_pos)
{
  DrawTrackBearing(canvas, aircraft_pos, InCirclingMode());
}

void
GlueMapWindow::DrawThermalBand(Canvas &canvas, const PixelRect &rc,
                               unsigned nav_slider_bar_visible_height) const
{
  if (Calculated().task_stats.total.solution_remaining.IsOk() &&
      Calculated().task_stats.total.solution_remaining.altitude_difference > fixed(50)
      && GetDisplayMode() == DisplayMode::FINAL_GLIDE)
    return;

  PixelRect tb_rect;
  tb_rect.left = rc.left;
  tb_rect.right = rc.left+Layout::Scale(20);
  tb_rect.top = nav_slider_bar_visible_height;
  tb_rect.bottom = (rc.bottom-rc.top) / 5 + tb_rect.top;

  const ThermalBandRenderer &renderer = thermal_band_renderer;
  if (task != NULL) {
    ProtectedTaskManager::Lease task_manager(*task);
    renderer.DrawThermalBand(Basic(),
                             Calculated(),
                             GetComputerSettings(),
                             canvas,
                             tb_rect,
                             GetComputerSettings().task,
                             true,
                             &task_manager->GetOrderedTask().GetOrderedTaskSettings());
  } else {
    renderer.DrawThermalBand(Basic(),
                             Calculated(),
                             GetComputerSettings(),
                             canvas,
                             tb_rect,
                             GetComputerSettings().task,
                             true);
  }
}

void
GlueMapWindow::DrawStallRatio(Canvas &canvas, const PixelRect &rc) const
{
  if (Basic().stall_ratio_available) {
    // JMW experimental, display stall sensor
    fixed s = Clamp(Basic().stall_ratio, fixed(0), fixed(1));
    PixelScalar m((rc.bottom - rc.top) * s * s);

    canvas.SelectBlackPen();
    canvas.DrawLine(rc.right - 1, rc.bottom - m, rc.right - 11, rc.bottom - m);
  }
}

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
void
GlueMapWindow::SetMainMenuButtonRect()
{
  PixelSize menu_button_size;

  menu_button_size.cx = MapOverlayButton::GetStandardButtonHeight()
      * MapOverlayButton::GetScale() + MapOverlayButton::GetClearBorderWidth();
  menu_button_size.cy = menu_button_size.cx;

  UPixelScalar pen_width = Layout::Scale(2);
  rc_main_menu_button = GetClientRect();
  rc_main_menu_button.left -= pen_width;
  rc_main_menu_button.top -= pen_width;
  rc_main_menu_button.left = rc_main_menu_button.right -  menu_button_size.cx;
  rc_main_menu_button.top = rc_main_menu_button.bottom -  menu_button_size.cy;
}

void
GlueMapWindow::SetZoomButtonsRect()
{
  const PixelRect rc_map = GetClientRect();

  PixelSize button_size;
  button_size.cx = button_size.cy =
      MapOverlayButton::GetStandardButtonHeight()
      * MapOverlayButton::GetScale();

  rc_zoom_in_button.left = rc_map.left;
  if (Layout::landscape) {
    rc_zoom_in_button.bottom = rc_map.bottom;
  } else {
    rc_zoom_in_button.bottom = rc_map.bottom - button_size.cy;
  }
  rc_zoom_in_button.right = rc_zoom_in_button.left + button_size.cx;
  rc_zoom_in_button.top = rc_zoom_in_button.bottom - button_size.cy;

  if (Layout::landscape) {
    rc_zoom_out_button.left = rc_map.left + button_size.cx;
  } else {
    rc_zoom_out_button.left = rc_map.left;
  }
  rc_zoom_out_button.bottom = rc_map.bottom;
  rc_zoom_out_button.top = rc_zoom_out_button.bottom - button_size.cy;
  rc_zoom_out_button.right = rc_zoom_out_button.left + button_size.cx;

  assert (rc_map.bottom >= rc_zoom_in_button.top);
  SetGPSStatusOffset(rc_map.bottom - rc_zoom_in_button.top);
}
#endif

void
GlueMapWindow::SetTaskNavSliderShape()
{
  slider_shape.Resize(GetClientRect().right - GetClientRect().left);
}
