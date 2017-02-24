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

#ifndef XCSOAR_GLUE_MAP_WINDOW_HPP
#define XCSOAR_GLUE_MAP_WINDOW_HPP

#include "MapWindow.hpp"
#include "Time/PeriodClock.hpp"
#include "UIUtil/TrackingGestureManager.hpp"
#include "UIUtil/KineticManager.hpp"
#include "Renderer/ThermalBandRenderer.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Renderer/VarioBarRenderer.hpp"
#include "Screen/Timer.hpp"
#include "Screen/Features.hpp"
#include "TophatWidgets/TaskNavSliderShape.hpp"
#include "UIUtil/GestureZone.hpp"
#include "Engine/Task/TaskType.hpp"

#include <array>

struct Look;
struct GestureLook;
class TopographyThread;
class MaskedIcon;

class OffsetHistory
{
  unsigned int pos;
  std::array<RasterPoint, 30> offsets;

public:
  OffsetHistory():pos(0) {
    Reset();
  }

  void Reset();
  void Add(RasterPoint p);
  RasterPoint GetAverage() const;
};


class GlueMapWindow : public MapWindow {
  enum class Command {
    INVALIDATE,
  };

  TopographyThread *topography_thread;

#ifdef ENABLE_OPENGL
  /**
   * A timer that triggers a redraw periodically until all data files
   * (terrain, topography, ...) have been loaded / updated.  This is
   * necessary if there is no valid GPS input, and no other reason to
   * redraw is present.  This timer will cease automatically once all
   * data is loaded, i.e. Idle() returned false.
   */
  WindowTimer data_timer;
#endif

  PeriodClock mouse_down_clock;

  enum DragMode {
    DRAG_NONE,

#ifdef HAVE_MULTI_TOUCH
    /**
     * Dragging the map with two fingers; enters the "real" pan mode
     * as soon as the user releases the finger press.
     */
    DRAG_MULTI_TOUCH_PAN,
#endif

    DRAG_PAN,

    /**
     * when a drag is initiated by does not start within the
     * Gesture zone
     */
    DRAG_NON_GESTURE,

    /**
     * when a drag is initiated that starts within the
     * Gesture zone
     */
    DRAG_GESTURE,
    DRAG_SIMULATOR,
  } drag_mode;

  GeoPoint drag_start_geopoint;
  RasterPoint drag_start;
  TrackingGestureManager gestures;

  /**
   * the area where it is valid to start a gesture
   */
  GestureZone gesture_zone;
  bool ignore_single_click;

  /**
   * Skip the next Idle() call?  This is set to true when a new frame
   * shall be rendered quickly without I/O delay, e.g. to display the
   * first frame quickly.
   */
  bool skip_idle;

#ifdef ENABLE_OPENGL
  KineticManager kinetic_x, kinetic_y;
  WindowTimer kinetic_timer;
#endif

  /** flag to indicate if the MapItemList should be shown on mouse up */
  bool arm_mapitem_list;

  /**
   * The projection which was active when dragging started.
   */
  Projection drag_projection;

  DisplayMode last_display_mode;

  /** keep last angle to use if temporary INVALID status of vector_remaining */
  Angle last_screen_angle;

  /**
   * With TARGET_UP orientation, we freeze the orientation when target is reached
   * until the NavBar is advanced by the user
   * or the task index is incremented automatically
   * -1 if not frozen
   */
  int nav_to_target_frozen_index;

  /* the type of task the last time the screen orientation was calculated */
  TaskType last_task_type;

  OffsetHistory offset_history;

#ifndef ENABLE_OPENGL
  /**
   * This mutex protects the attributes that are read by the
   * DrawThread but written by another thread.
   */
  Mutex next_mutex;

  /**
   * The new map settings.  It is passed to
   * MapWindowBlackboard::ReadMapSettings() before the next frame.
   */
  MapSettings next_settings_map;

  /**
   * The new glide computer settings.  It is passed to
   * MapWindowBlackboard::ReadGetComputerSettings() before the next
   * frame.
   */
  ComputerSettings next_settings_computer;

  UIState next_ui_state;
#endif

  ThermalBandRenderer thermal_band_renderer;
  FinalGlideBarRenderer final_glide_bar_renderer;
  VarioBarRenderer vario_bar_renderer;

  const GestureLook &gesture_look;

  WindowTimer map_item_timer;

public:
  GlueMapWindow(const Look &look);
  virtual ~GlueMapWindow();

  void SetTopography(TopographyStore *_topography);

  void SetMapSettings(const MapSettings &new_value);
  void SetComputerSettings(const ComputerSettings &new_value);
  void SetUIState(const UIState &new_value);

  /**
   * Update the blackboard from DeviceBlackboard and
   * InterfaceBlackboard.
   */
  void ExchangeBlackboard();

  /**
   * Suspend threads that are owned by this object.
   */
  void SuspendThreads();

  /**
   * Resumt threads that are owned by this object.
   */
  void ResumeThreads();

  /**
   * Trigger a full redraw of the map.
   */
  void FullRedraw();

  void QuickRedraw();

  bool Idle();

  void Create(ContainerWindow &parent, const PixelRect &rc);

  void SetPan(bool enable);
  void TogglePan();
  void PanTo(const GeoPoint &location);

  bool ShowMapItems(const GeoPoint &location,
                    bool show_empty_message = true) const;

protected:
  /* virtual methods from class MapWindow */
  virtual void Render(Canvas &canvas, const PixelRect &rc) override;
  virtual void DrawThermalEstimate(Canvas &canvas) const override;
  virtual void RenderTrail(Canvas &canvas,
                           const RasterPoint aircraft_pos) override;
  virtual void RenderTrackBearing(Canvas &canvas,
                                  const RasterPoint aircraft_pos) override;

  /* virtual methods from class Window */
  virtual void OnDestroy() override;
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys) override;
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y) override;
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y) override;
  virtual bool OnMouseWheel(PixelScalar x, PixelScalar y, int delta) override;

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  /**
   * returns true if handled by the button overlays.
   *
   * @param test if true then only tests location.  If false handles event
   */
  virtual bool ButtonOverlaysOnMouseUp(PixelScalar x, PixelScalar y,
                                         bool test);
  /**
   * returns true if handled by the button overlays.
   *
   * @param test if true then only tests location.  If false handles event
   */
  virtual bool ButtonOverlaysOnMouseDown(PixelScalar x, PixelScalar y,
                                         bool test);
#endif

#ifdef HAVE_MULTI_TOUCH
  virtual bool OnMultiTouchDown() override;
#endif

  virtual bool OnKeyDown(unsigned key_code) override;
  virtual void OnCancelMode() override;
  virtual void OnPaint(Canvas &canvas) override;
  virtual void OnPaintBuffer(Canvas& canvas) override;
  virtual bool OnTimer(WindowTimer &timer) override;
  bool OnUser(unsigned id) override;

  /**
   * This event handler gets called when a gesture has
   * been painted by the user
   * @param gesture The gesture string (e.g. "ULR")
   * @return True if the gesture was handled by the
   * event handler, False otherwise
   */
  bool OnMouseGesture(const TCHAR* gesture);

private:
  void DrawGesture(Canvas &canvas) const;
  void DrawMapScale(Canvas &canvas, const PixelRect &rc,
                    const MapWindowProjection &projection) const;
  void DrawFlightMode(Canvas &canvas, const PixelRect &rc,
                      unsigned nav_slider_bar_visible_height) const;
  void DrawGPSStatus(Canvas &canvas, const PixelRect &rc,
                     const NMEAInfo &info) const;
  void DrawCrossHairs(Canvas &canvas) const;
  void DrawPanInfo(Canvas &canvas) const;
  void DrawThermalBand(Canvas &canvas, const PixelRect &rc,
                       unsigned nav_slider_bar_visible_height) const;
  void DrawFinalGlide(Canvas &canvas, const PixelRect &rc) const;
  void DrawVario(Canvas &canvas, const PixelRect &rc) const;
  void DrawStallRatio(Canvas &canvas, const PixelRect &rc) const;

#ifndef ENABLE_OPENGL
  /**
   * render transparent buttons on the screen with GDI
   * Duplicates code in Widget that is used with OPENGL
   * Todo: remove duplicate code
   *
   */
  void DrawMainMenuButtonOverlay(Canvas &canvas) const;

  /**
   * render transparaent buttons for zoom in / out with GDI
   * Duplicates code in Widget that is used with OPENGL
   * Todo: remove duplicate code
   */
  void DrawZoomButtonOverlays(Canvas &canvas) const;

  /**
   * draws the task nav with transparencies if !HasDraggableScreen()
   * this does not support dragging
   */
  void DrawTaskNavSliderShape(Canvas &canvas);
#endif

  void SwitchZoomClimb();

  void SaveDisplayModeScales();

  /**
   * The attribute visible_projection has been edited.
   */
  void OnProjectionModified() {}

  /**
   * Invoke WindowProjection::UpdateScreenBounds() and trigger updates
   * of data file caches for the new bounds (e.g. topography).
   */
  void UpdateScreenBounds();

  void UpdateScreenAngle();
  void UpdateProjection();

public:
  void SetLocation(const GeoPoint location);

  /**
   * Update the visible_projection location, but only if the new
   * location is sufficiently distant from the current one.  This
   * shall avoid unnecessary map jiggling.  This is a great
   * improvement for E Ink displays to reduce flickering.
   */
  void SetLocationLazy(const GeoPoint location);

  void UpdateMapScale();

  /**
   * Restore the map scale from MapSettings::cruise_scale or
   * MapSettings::circling_scale.
   */
  void RestoreMapScale();

  void UpdateDisplayMode();
  void SetMapScale(fixed scale);

  /**
   * resizes the TaskNavSlider shape for the current screen layout
   * only when it's !HasDraggableScreen()
   */
  void SetTaskNavSliderShape();

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
/**
   * resizes the rc_main_menu_button for the current screen layout
   * when drawing without OPENGL
   */
  virtual void SetMainMenuButtonRect();
  /**
   * resizes the rc_zoom_in_button and rc_zoom_out_button for the current
   * screen layout when drawing without OPENGL
   */
  virtual void SetZoomButtonsRect();

  /**
   * helper class for drawing Win PC map overlay buttons
   */
  class WinMapOverlayButton : public PixelRect
  {
    bool down;
    const MaskedIcon *icon;
    PixelRect rc_icon;
  public:
    WinMapOverlayButton()
    : down(false) {}

    void Draw(Canvas &canvas) const;

    void SetDown(bool _down) {
      down = _down;
    }
    bool IsDown() const {
      return down;
    }
    void SetIcon(const MaskedIcon *_icon) {
      icon = _icon;
      SetIconRect();
    }
    bool IsValid() const {
      return bottom > top && right > left;
    }

    WinMapOverlayButton operator=(const PixelRect rect) {
      left = rect.left;
      right = rect.right;
      top = rect.top;
      bottom = rect.bottom;
      return *this;
    }
  private:
    void SetIconRect();
  };

  /**
   * locations of Map overlay buttons when not using
   * OPENGL or KOBO
   */
  WinMapOverlayButton rc_nav_slider_shape_button;
  WinMapOverlayButton rc_main_menu_button;
  WinMapOverlayButton rc_zoom_out_button;
  WinMapOverlayButton rc_zoom_in_button;
#endif

  /**
   * draws the slider shape if the screen does not have good dragging
   * e.g. !HasDraggableScreen()
   */
  SliderShape slider_shape;

protected:
  DisplayMode GetDisplayMode() const {
    return GetUIState().display_mode;
  }

  bool InCirclingMode() const {
    return GetUIState().display_mode == DisplayMode::CIRCLING;
  }
};

#endif
