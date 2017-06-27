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

#include "GlueMapWindow.hpp"
#include "Input/InputEvents.hpp"
#include "Screen/Layout.hpp"
#include "Simulator.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Math/FastMath.h"
#include "Compiler.h"
#include "Interface.hpp"
#include "Pan.hpp"
#include "Util/Clamp.hpp"
#include "Asset.hpp"
#include "Event/Idle.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Menu/TophatMenu.hpp"
#include "Topography/Thread.hpp"

#ifdef USE_X11
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#endif

#ifdef ENABLE_SDL
#include <SDL_keyboard.h>
#endif

void
GlueMapWindow::OnDestroy()
{
  /* stop the TopographyThread */
  SetTopography(nullptr);

#ifdef ENABLE_OPENGL
  data_timer.Cancel();
#endif

  MapWindow::OnDestroy();
}

bool
GlueMapWindow::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (drag_mode == DRAG_NONE && HasTouchScreen())
    OnMouseDown(x, y);

  /* allow a bigger threshold on touch screens */
  const unsigned threshold = Layout::Scale(20);
  if (drag_mode != DRAG_NONE && arm_mapitem_list &&
      manhattan_distance(drag_start, RasterPoint{x, y}) > threshold)
    arm_mapitem_list = false;

  switch (drag_mode) {
  case DRAG_NONE:
    break;
  case DRAG_GESTURE:
    gestures.Update(x, y);

    /* invoke PaintWindow's Invalidate() implementation instead of
       DoubleBufferWindow's in order to reuse the buffered map */
    PaintWindow::Invalidate();
    return true;
  case DRAG_NON_GESTURE:

    if (unsigned((abs(drag_start.x - x) + abs(drag_start.y - y))) < Layout::Scale(threshold)
        && follow_mode != FOLLOW_PAN)
      break;
    drag_projection = visible_projection;
    follow_mode = FOLLOW_PAN;

#ifdef HAVE_MULTI_TOUCH
  case DRAG_MULTI_TOUCH_PAN:
#endif
  case DRAG_PAN:
    SetLocation(drag_projection.GetGeoLocation()
                + drag_start_geopoint
                - drag_projection.ScreenToGeo(x, y));
    QuickRedraw();

#ifdef ENABLE_OPENGL
    kinetic_x.MouseMove(x);
    kinetic_y.MouseMove(y);
#endif
    return true;

  case DRAG_SIMULATOR:
    return true;
  }

  return MapWindow::OnMouseMove(x, y, keys);
}

gcc_pure
static bool
IsCtrlKeyPressed()
{
#ifdef ENABLE_SDL
  return SDL_GetModState() & (KMOD_LCTRL|KMOD_RCTRL);
#elif defined(USE_GDI)
  return GetKeyState(VK_CONTROL) & 0x8000;
#elif defined(USE_X11)
  return event_queue->WasCtrlClick();
#else
  return false;
#endif
}

bool
GlueMapWindow::OnMouseDown(PixelScalar x, PixelScalar y)
{
#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  if (ButtonOverlaysOnMouseDown(x, y, false)) {
    QuickRedraw();
    return true;
  }
#endif

  map_item_timer.Cancel();

#ifdef ENABLE_OPENGL
  kinetic_timer.Cancel();
#endif

  // Ignore single click event if double click detected
  if (ignore_single_click || drag_mode != DRAG_NONE)
    return true;

  if (is_simulator() && IsCtrlKeyPressed() && visible_projection.IsValid()) {
    /* clicking with Ctrl key held moves the simulator to the click
       location instantly */
    const GeoPoint location = visible_projection.ScreenToGeo(x, y);
    device_blackboard->SetSimulatorLocation(location);
    return true;
  }

  mouse_down_clock.Update();
  mouse_down = true;
  arm_mapitem_list = HasFocus();

  SetFocus();

  drag_start.x = x;
  drag_start.y = y;

  if (!visible_projection.IsValid()) {
    gestures.Start(x, y, Layout::Scale(20));
    drag_mode = DRAG_GESTURE;
    SetCapture();
    return true;
  }

  drag_start_geopoint = visible_projection.ScreenToGeo(x, y);

  switch (follow_mode) {
  case FOLLOW_SELF:
    break;

  case FOLLOW_PAN:
    drag_mode = DRAG_PAN;
    drag_projection = visible_projection;

#ifdef ENABLE_OPENGL
    kinetic_x.MouseDown(x);
    kinetic_y.MouseDown(y);
#endif

    break;
  }

  if (CommonInterface::Basic().gps.simulator && drag_mode == DRAG_NONE &&
      compare_squared(visible_projection.GetScreenOrigin().x - x,
                        visible_projection.GetScreenOrigin().y - y,
                        Layout::Scale(30)) != 1) {
        drag_mode = DRAG_SIMULATOR;
  } else if (drag_mode == DRAG_NONE ) {
    if (gesture_zone.InZone(GetClientRect(), {x, y} )) {
      gestures.Start(x, y, Layout::Scale(20));
      drag_mode = DRAG_GESTURE;
    } else {
      drag_mode = DRAG_NON_GESTURE;
    }
  }

  if (drag_mode != DRAG_NONE)
    SetCapture();

  return true;
}

bool
GlueMapWindow::OnMouseUp(PixelScalar x, PixelScalar y)
{
  if (drag_mode != DRAG_NONE)
    ReleaseCapture();

  // Ignore single click event if double click detected
  if (ignore_single_click) {
    ignore_single_click = false;
    return true;
  }

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  if (ButtonOverlaysOnMouseUp(x, y, false)) {
    QuickRedraw();
    return true;
  }
#endif

  int click_time = mouse_down_clock.Elapsed();
  mouse_down_clock.Reset();
  mouse_down = false;

  DragMode old_drag_mode = drag_mode;
  drag_mode = DRAG_NONE;

  switch (old_drag_mode) {
  case DRAG_NONE:
    /* skip the arm_mapitem_list check below */
    return false;

#ifdef HAVE_MULTI_TOUCH
  case DRAG_MULTI_TOUCH_PAN:
    ::PanTo(visible_projection.GetGeoScreenCenter());
    return true;
#endif

  case DRAG_PAN:
#ifndef ENABLE_OPENGL
    /* allow the use of the stretched last buffer for the next two
       redraws */
    scale_buffer = 2;
#endif

#ifdef ENABLE_OPENGL
    kinetic_x.MouseUp(x);
    kinetic_y.MouseUp(y);
    kinetic_timer.Schedule(30);
    mouse_down = true; // still actively changing pan
#endif
    break;

  case DRAG_SIMULATOR:
    if (click_time > 50 &&
        compare_squared(drag_start.x - x, drag_start.y - y,
                        Layout::Scale(36)) == 1) {
      GeoPoint location = visible_projection.ScreenToGeo(x, y);

      double distance = hypot(drag_start.x - x, drag_start.y - y);
      // This drag moves the aircraft (changes speed and direction)
      const Angle old_bearing = CommonInterface::Basic().track;
      const fixed min_speed = fixed(1.1) *
        CommonInterface::GetComputerSettings().polar.glide_polar_task.GetVMin();
      const Angle new_bearing = drag_start_geopoint.Bearing(location);
      if (((new_bearing - old_bearing).AsDelta().AbsoluteDegrees() < fixed(30)) ||
          (CommonInterface::Basic().ground_speed < min_speed)) {
        device_blackboard->SetSpeed(Clamp(fixed(distance) / Layout::FastScale(3),
                                          min_speed, fixed(100)));
        device_blackboard->SkipNextGlideSpeedCalculation();
      }
      device_blackboard->SetTrack(new_bearing);

      // change bearing without changing speed if direction change > 30
      // 20080815 JMW prevent dragging to stop glider
      gesture_zone.ClearZoneHelp();
      return true;
    }

    break;

  case DRAG_GESTURE:
  {
    const TCHAR* gesture = gestures.Finish();
    if (gesture && OnMouseGesture(gesture)) {
      if (gesture_zone.IsHelpVisible())
        gesture_zone.ClearZoneHelp();
      return true;
    }
    break;
  }
  case DRAG_NON_GESTURE:
  {
    /* allow a bigger threshold on touch screens */
    const int threshold = IsEmbedded() ? 50 : 10;
    if ((abs(drag_start.x - x) + abs(drag_start.y - y)) > Layout::Scale(threshold)) {
      ::PanTo(visible_projection.GetGeoScreenCenter());
      gesture_zone.ClearZoneHelp();
    } else
      LeavePan();

    break;
  }
  }

  if (!InputEvents::IsDefault() && !IsPanning()) {
    InputEvents::HideMenu();
    return true;
  }

  if (arm_mapitem_list) {
    map_item_timer.Schedule(200);
    return true;
  }

  return false;
}

bool
GlueMapWindow::OnMouseWheel(PixelScalar x, PixelScalar y, int delta)
{
  map_item_timer.Cancel();

#ifdef ENABLE_OPENGL
  kinetic_timer.Cancel();
#endif

  if (drag_mode != DRAG_NONE)
    return true;

  if (delta > 0)
    // zoom in
    InputEvents::sub_ScaleZoom(1);
  else if (delta < 0)
    // zoom out
    InputEvents::sub_ScaleZoom(-1);

  return true;
}

#ifdef HAVE_MULTI_TOUCH

bool
GlueMapWindow::OnMultiTouchDown()
{
  if (!visible_projection.IsValid())
    return false;

  if (drag_mode == DRAG_GESTURE)
    gestures.Finish();
  else if (follow_mode != FOLLOW_SELF)
    return false;

  /* start panning on MultiTouch event */

  drag_mode = DRAG_MULTI_TOUCH_PAN;
  drag_projection = visible_projection;
  follow_mode = FOLLOW_PAN;
  return true;
}

#endif /* HAVE_MULTI_TOUCH */

bool
GlueMapWindow::OnMouseGesture(const TCHAR* gesture)
{
  return InputEvents::processGesture(gesture);
}

bool
GlueMapWindow::OnKeyDown(unsigned key_code)
{
  map_item_timer.Cancel();

#ifdef ENABLE_OPENGL
  kinetic_timer.Cancel();
#endif

  if (InputEvents::processKey(key_code)) {
    return true; // don't go to default handler
  }

  return false;
}

void
GlueMapWindow::OnCancelMode()
{
  MapWindow::OnCancelMode();

  if (drag_mode != DRAG_NONE) {
#ifdef HAVE_MULTI_TOUCH
    if (drag_mode == DRAG_MULTI_TOUCH_PAN)
      follow_mode = FOLLOW_SELF;
#endif

    if (drag_mode == DRAG_GESTURE)
      gestures.Finish();

    ReleaseCapture();
    drag_mode = DRAG_NONE;
  }

#ifdef ENABLE_OPENGL
  kinetic_timer.Cancel();
#endif

  map_item_timer.Cancel();
}

void
GlueMapWindow::OnPaint(Canvas &canvas)
{
  MapWindow::OnPaint(canvas);

  // Draw center screen cross hair in pan mode
  if (IsPanning())
    DrawCrossHairs(canvas);


  DrawGesture(canvas);
  if (!IsPanning()) {
    UISettings &ui_settings = CommonInterface::SetUISettings();
    if (ui_settings.restart_gesture_help) {
      gesture_zone.RestartZoneHelp();
      ui_settings.restart_gesture_help = false;
    } else if (ui_settings.clear_gesture_help) {
      gesture_zone.ClearZoneHelp();
      ui_settings.clear_gesture_help = false;
    }
    const TerrainRendererSettings &terrain = settings_map.terrain;
    bool terrain_enabled = terrain.enable;
    gesture_zone.DrawZone(canvas, GetClientRect(), terrain_enabled);
  }
}

void
GlueMapWindow::OnPaintBuffer(Canvas &canvas)
{
#ifdef ENABLE_OPENGL
  ExchangeBlackboard();

  EnterDrawThread();

  /* update terrain, topography, ... */
  if (Idle())
    /* still dirty: schedule a redraw to load more data */
    data_timer.Schedule(500);
  else
    data_timer.Cancel();
#endif

  MapWindow::OnPaintBuffer(canvas);

  DrawMapScale(canvas, GetClientRect(), render_projection);
  if (IsPanning())
    DrawPanInfo(canvas);

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  if (!IsPanning())
    DrawMainMenuButtonOverlay(canvas);
  DrawZoomButtonOverlays(canvas);
  if (!HasDraggableScreen())
    DrawTaskNavSliderShape(canvas);

#endif

#ifdef ENABLE_OPENGL
  LeaveDrawThread();
#endif
}

bool
GlueMapWindow::OnTimer(WindowTimer &timer)
{
  if (timer == map_item_timer) {
    map_item_timer.Cancel();
    if (!InputEvents::IsDefault() && !IsPanning()) {
      InputEvents::HideMenu();
      return true;
    }
    if (gesture_zone.IsHelpVisible()) {
      gesture_zone.ClearZoneHelp();
      return true;
    }
    ShowMapItems(drag_start_geopoint, false);
    return true;
#ifdef ENABLE_OPENGL
  } else if (timer == kinetic_timer) {
    if (kinetic_x.IsSteady() && kinetic_y.IsSteady()) {
      kinetic_timer.Cancel();
      mouse_down = false; // stopped actively changing pan
    } else {
      auto location = drag_projection.ScreenToGeo(kinetic_x.GetPosition(),
                                                  kinetic_y.GetPosition());
      location = drag_projection.GetGeoLocation() +
          drag_start_geopoint - location;

      SetLocation(location);
      QuickRedraw();
    }

    return true;
  } else if (timer == data_timer) {
    if (!IsUserIdle(2500))
      /* user is still active; try again later */
      return true;

    Invalidate();
    return false;
#endif
  } else
    return MapWindow::OnTimer(timer);
}

void
GlueMapWindow::Render(Canvas &canvas, const PixelRect &rc)
{
  MapWindow::Render(canvas, rc);

  if (IsNearSelf()) {
    draw_sw.Mark("DrawGlueMisc");
    if (GetMapSettings().show_thermal_profile)
      DrawThermalBand(canvas, rc, nav_slider_bar_visible_height);
    DrawStallRatio(canvas, rc);
    DrawFlightMode(canvas, rc, nav_slider_bar_visible_height);
    DrawFinalGlide(canvas, rc);
    DrawVario(canvas, rc);
    DrawGPSStatus(canvas, rc, Basic());
  }
}

#if !defined(ENABLE_OPENGL) & !defined(KOBO)

bool
GlueMapWindow::ButtonOverlaysOnMouseUp(PixelScalar x, PixelScalar y, bool test)
{
  bool main_menu_button_down = rc_main_menu_button.IsDown();
  bool zoom_out_button_down = rc_zoom_out_button.IsDown();
  bool zoom_in_button_down = rc_zoom_in_button.IsDown();
  bool slider_shape_button_down = rc_nav_slider_shape_button.IsDown();
  RasterPoint p {x, y};

  rc_main_menu_button.SetDown(false);
  rc_zoom_out_button.SetDown(false);
  rc_zoom_in_button.SetDown(false);
  rc_nav_slider_shape_button.SetDown(false);

  if (main_menu_button_down && rc_main_menu_button.IsInside(p)) {
    if (!test)
      TophatMenu::RotateMenu();
    return true;
  }
  if (zoom_out_button_down && rc_zoom_out_button.IsInside(p)) {
    if (!test) {
      InputEvents::eventZoom(_T("-"));
      InputEvents::HideMenu();
    }
    return true;
  }
  if (zoom_in_button_down && rc_zoom_in_button.IsInside(p)) {
    if (!test) {
      InputEvents::eventZoom(_T("+"));
      InputEvents::HideMenu();
    }
    return true;
  }

  //TODO: add mouse up logic to PPC slider shape (!HasDraggableScreen())
  if (!HasDraggableScreen() && slider_shape_button_down &&
      rc_nav_slider_shape_button.IsInside(p)) {
    if (!test) {
      StaticString<20> menu_ordered(_T("NavOrdered"));
      StaticString<20> menu_goto(_T("NavGoto"));
      StaticString<20> menu_teammate(_T("NavTeammate"));

      TaskType task_mode;
      {
        ProtectedTaskManager::Lease task_manager(*protected_task_manager);
        task_mode = task_manager->GetMode();
      }
      if (InputEvents::IsMode(menu_ordered.buffer())
          || InputEvents::IsMode(menu_goto.buffer())
          || InputEvents::IsMode(menu_teammate.buffer()))
        InputEvents::HideMenu();
      else if (task_mode == TaskType::GOTO
          || task_mode == TaskType::ABORT)
        InputEvents::setMode(menu_goto.buffer());
      else if (task_mode == TaskType::TEAMMATE)
          InputEvents::setMode(menu_teammate.buffer());
      else
        InputEvents::setMode(menu_ordered.buffer());
    }
    return true;
  }

  return false;
}

bool
GlueMapWindow::ButtonOverlaysOnMouseDown(PixelScalar x, PixelScalar y, bool test)
{
  RasterPoint p {x, y};

  rc_main_menu_button.SetDown(rc_main_menu_button.IsInside(p));
  rc_zoom_out_button.SetDown(rc_zoom_out_button.IsInside(p));
  rc_zoom_in_button.SetDown(rc_zoom_in_button.IsInside(p));
  rc_nav_slider_shape_button.SetDown(rc_nav_slider_shape_button.IsInside(p));

  return rc_main_menu_button.IsDown() ||
      rc_zoom_out_button.IsDown() ||
      rc_zoom_in_button.IsDown() ||
      rc_nav_slider_shape_button.IsDown();
}
#endif
