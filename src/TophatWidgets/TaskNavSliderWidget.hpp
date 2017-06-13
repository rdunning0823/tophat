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

#ifndef XCSOAR_TASK_NAV_SLIDER_WIDGET_HPP
#define XCSOAR_TASK_NAV_SLIDER_WIDGET_HPP

#include "TophatWidgets/MapOverlayBaseWidget.hpp"
#include "TophatWidgets/TaskNavSliderShape.hpp"
#include "Form/HorizontalList.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Util/StaticString.hxx"
#include "Task/TaskType.hpp"
#include "Menu/TophatMenu.hpp"

#include <assert.h>

struct ComputerSettings;
struct NMEAInfo;
struct DialogLook;
struct PixelRect;
struct DialogLook;
struct InfoBoxLook;
class TaskStats;
class ContainerWindow;

namespace TaskNavSlider {
/**
 * @return -1 if time under is invalid and should not be displayed,
 * else, elapsed seconds under start height
 */
  int GetTimeUnderStart(int max_height, bool show_two_minute_start);
}

class TaskNavSliderWidget : public MapOverlayBaseWidget, ListItemRenderer,
  HorizontalListCursorHandler  {
protected:

  SliderShape slider_shape;
  /**
   * timestamp from the task manager when task was last read
   */
  unsigned task_manager_time_stamp;
  /** size of ordered task when task was last read */
  unsigned ordered_task_size;
  TaskType mode;
  PixelRect last_rc_map;
  /**
   * the summed font heights of the slider shape when last resized
   */
  unsigned last_sum_font_height;

protected:

  /**
   * index of list
   * This is updated immediately when the TaskManager refreshes
   * the task in the NavBar and the animation starts to scroll.
   * It is also updated immediately in OnCursorMove at the same
   * time the TaskManager is updated.
   */
  unsigned waypoint_index;

public:
  TaskNavSliderWidget();

  const HorizontalListControl &GetList() const {
    return (const HorizontalListControl&)GetWindow();
  }

  HorizontalListControl &GetList() {
    return (HorizontalListControl &)GetWindow();
  }

  unsigned GetHeight() {
    return slider_shape.GetHeight();
  }

  /**
   * Shows or hides the widgets based on these parameters
   * @rc. the rc of the map
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   */
  virtual void UpdateVisibility(const PixelRect &rc, bool is_panning,
                                bool is_main_window_widget, bool is_map,
                                bool is_top_widget) override;

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;
  virtual void Move(const PixelRect &rc) override;
  virtual bool SetFocus() override {
    GetList().KillFocus();
    return false;
  }

  /**
   * How much height does this widget use at the top of the map screen
   */
  virtual UPixelScalar HeightFromTop() override;

  /**
   * Updates the cached values of the task from the protected_task_manager
   * Widget must be prepared (which calls CreateListEmpty) before calling
   * Assumes GetList().SetRowHeight() will be called later
   * returns true if active task is updated
   */
  bool RefreshTask();

protected:
  /**
   * returns true if active task has not changed since it was last read
   */
  bool TaskIsCurrent();

  /**
   * updates waypoint_index from protected_task_manager.
   * should be called only after verifying active task is current
   */
  void ReadWaypointIndex();

  /**
   * refreshes length and cursor index of list
   * does not lock task manager
   */
  void RefreshList(TaskType mode);

  /**
   * reads the task from the protected task manager
   */
  TaskType ReadTask();

  HorizontalListControl& CreateListEmpty(ContainerWindow &parent,
                                         const DialogLook &look,
                                         const PixelRect &rc);

  /** These routines access the ProtectedTaskManager */
  /**
   * returns task mode from protected task manager
   */
  TaskType GetTaskMode();

  /**
   * returns waypoint index from protected_task_manager or -1
   * if not in ordered
   * Also updates the TaskNavData cache's "ActiveWaypoint"
   */
  int GetCurrentWaypoint();

  /**
   * Resumes the ordered task
   */
  void ResumeTask();


#ifdef _WIN32
  /**
   * clears background adjacent to slider.
   * WIN32 does not draw transparent like OpenGL does
   */
  void PaintBackground(Canvas &canvas, unsigned idx,
                       const DialogLook &dialog_look,
                       const PixelRect rc_outer);
#endif

  /**
   * Inherited from ListControl::Handler
   */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /**
   * Toggles through the menu modes or hides if last mode
   */
  virtual void OnActivateItem(unsigned index) override;

  virtual void OnCursorMoved(unsigned index) override;

  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }

  /**
   * hide the menu if it's being moved
   */
  virtual void OnPixelMove() override;
};

#endif
