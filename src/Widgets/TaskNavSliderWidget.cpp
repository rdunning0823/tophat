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

#include "Widgets/TaskNavSliderWidget.hpp"
#include "Input/InputEvents.hpp"
#include "Util/StaticString.hpp"
#include "OS/Clock.hpp"

#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Look/DialogLook.hpp"
#include "Dialogs/Message.hpp"
#include "Screen/Window.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Canvas.hpp"

#include "Interface.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Task/Ordered/OrderedTask.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Globals.hpp"
#elif defined(USE_GDI)
#include "Screen/WindowCanvas.hpp"
#endif

#include <stdio.h>
#include <stdlib.h>

TaskNavSliderWidget::TaskNavSliderWidget()
  :task_data_cache(CommonInterface::GetComputerSettings(),
                   CommonInterface::Basic(),
                   CommonInterface::Calculated().task_stats) {}

void
TaskNavSliderWidget::UpdateVisibility(const PixelRect &rc, bool is_panning,
                                      bool is_main_window_widget, bool is_map)
{
  if (is_map && !is_main_window_widget && !is_panning)
    Show(rc);
  else
    Hide();
}

bool
TaskNavSliderWidget::RefreshTask()
{
  if (!GetList().IsDefined())
    return false;

  // only update if it's been changed
  if (TaskIsCurrent()) {
    if (!task_data_cache.AreTransistionsAndTargetsCurrent()) {
      ProtectedTaskManager::Lease task_manager(*protected_task_manager);
      task_data_cache.UpdateTransitions(task_manager->GetOrderedTask());
      task_data_cache.UpdateTargets(task_manager->GetOrderedTask());
    }

    ReadWaypointIndex();

    return false;
  }

  RefreshList(ReadTask());

  return true;
}

TaskManager::TaskMode
TaskNavSliderWidget::ReadTask()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  task_data_cache.UpdateOrderedTask(task_manager->GetOrderedTask(),
                                    task_manager->GetMode(),
                                    task_manager->GetActiveTaskPointIndex(),
                                    task_manager->GetTaskTimeStamp());

  switch (task_manager->GetMode()) {
  case TaskManager::MODE_ORDERED:
    waypoint_index = task_manager->GetActiveTaskPointIndex();
    break;
  case TaskManager::MODE_GOTO:
  case TaskManager::MODE_ABORT:
  case TaskManager::MODE_NULL:
    waypoint_index = 0;
    break;
  }

  return task_manager->GetMode();
}

void
TaskNavSliderWidget::RefreshList(TaskManager::TaskMode mode)
{
  switch (mode) {
  case TaskManager::MODE_ORDERED:
    GetList().SetLength(task_data_cache.GetOrderedTaskSize());
    break;
  case TaskManager::MODE_NULL:
  case TaskManager::MODE_ABORT:
    waypoint_index = 0;
    GetList().SetLength(1);
    break;
  case TaskManager::MODE_GOTO:
    waypoint_index = 0;
    GetList().SetLength(1);
    break;
  }
  GetList().SetCursorIndex(waypoint_index);
  GetList().Invalidate();
}

void
TaskNavSliderWidget::OnPaintItem(Canvas &canvas, const PixelRect rc_outer,
                                 unsigned idx)
{
  // if task is not current (e.g. the tp being drawn may no longer exist) then abort drawing
  // hold lease on task_manager until drawing is done
  {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);
    if (task_manager->GetTaskTimeStamp()
        != task_data_cache.GetTaskManagerTimeStamp())
      return;
  }

  TaskNavDataCache::tp_info tp = task_data_cache.GetPoint(idx);

  const MapSettings &settings_map = CommonInterface::GetMapSettings();
  const TerrainRendererSettings &terrain = settings_map.terrain;
  unsigned border_width = Layout::ScalePenWidth(terrain.enable ? 1 : 2);
  slider_shape.Draw(canvas, rc_outer,
                    idx, GetList().GetCursorDownIndex() == (int)idx,
                    tp.IsValid() ? tp.waypoint->name.c_str() : _T(""),
                    tp.GetHasEntered(), tp.GetHasExited(),
                    task_data_cache.GetTaskMode(),
                    task_data_cache.GetOrderedTaskSize(),
                    tp.IsValid(), tp.distance, tp.distance_valid,
                    tp.altitude_difference,
                    tp.altitude_difference_valid,
                    border_width);
}

void
TaskNavSliderWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  CreateListEmpty(parent, dialog_look, rc);
  Move(rc);
  WindowWidget::Prepare(parent, rc);
}

bool
TaskNavSliderWidget::TaskIsCurrent()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  return task_manager->GetTaskTimeStamp()
      == task_data_cache.GetTaskManagerTimeStamp();
}

void
TaskNavSliderWidget::ReadWaypointIndex()
{

  int task_manager_current_index = GetCurrentWaypoint();
  if (task_manager_current_index == -1) {
    return; // non-ordered mode
  }

  if ((task_manager_current_index != (int)waypoint_index)
      && GetList().IsSteady())
    GetList().ScrollToItem(task_manager_current_index);
}

HorizontalListControl &
TaskNavSliderWidget::CreateListEmpty(ContainerWindow &parent,
                                     const DialogLook &look,
                                     const PixelRect &rc)
{
  WindowStyle list_style;
  list_style.TabStop();

  HorizontalListControl *list = new HorizontalListControl(parent, look, rc,
                                                          list_style, 20);
  list->SetHasScrollBar(false);
  list->SetLength(0);
  list->SetHandler(this);
  SetWindow(list);
  return *list;
}

void
TaskNavSliderWidget::OnCursorMoved(unsigned index)
{
  if (task_data_cache.GetTaskMode() != TaskManager::MODE_ORDERED)
    return;

  assert(index < task_data_cache.GetOrderedTaskSize());

  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  task_manager->SetActiveTaskPoint(index);
  waypoint_index = index;
}

TaskManager::TaskMode
TaskNavSliderWidget::GetTaskMode()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  return task_manager->GetMode();
}

int
TaskNavSliderWidget::GetCurrentWaypoint()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);

  task_data_cache.SetActiveWaypoint(task_manager->GetActiveTaskPoint());
  task_data_cache.SetActiveTaskPointIndex(
      task_manager->GetActiveTaskPointIndex());

  if (task_manager->GetMode() != TaskManager::MODE_ORDERED)
    return -1;

  return task_data_cache.GetActiveTaskPointIndex();
}

void
TaskNavSliderWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
TaskNavSliderWidget::Show(const PixelRect &rc)
{
  GetWindow()->Show();
}

void
TaskNavSliderWidget::Hide()
{
  GetWindow()->Hide();
}

void
TaskNavSliderWidget::Move(const PixelRect &rc_map)
{
  PixelRect rc = rc_map;
  slider_shape.Resize(rc_map.right - rc_map.left);
  rc.bottom = rc.top + slider_shape.GetHeight();
  --rc.top;
  GetList().SetOverScrollMax(slider_shape.GetOverScrollWidth());

  WindowWidget::Move(rc);
  GetWindow()->Move(rc);
  GetList().SetItemHeight(slider_shape.GetWidth());
}

void
TaskNavSliderWidget::ResumeTask()
{
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  if (task_manager->GetMode() != TaskManager::MODE_ORDERED)
    task_manager->Resume();
}

void
TaskNavSliderWidget::OnActivateItem(unsigned index)
{
  StaticString<20> menu_ordered;
  StaticString<20> menu_goto;

  menu_ordered = _T("NavOrdered");
  menu_goto = _T("NavGoto");

  if (InputEvents::IsMode(menu_ordered.buffer())
      || InputEvents::IsMode(menu_goto.buffer()))
    InputEvents::HideMenu();
  else if (task_data_cache.GetTaskMode() == TaskManager::MODE_GOTO
      || task_data_cache.GetTaskMode() == TaskManager::MODE_ABORT)
    InputEvents::setMode(menu_goto.buffer());
  else
    InputEvents::setMode(menu_ordered.buffer());

  return;
}

void
TaskNavSliderWidget::OnPixelMove()
{
  InputEvents::HideMenu();
}

UPixelScalar
TaskNavSliderWidget::HeightFromTop()
{
  return GetWindow()->IsVisible() ? GetHeight() - Layout::Scale(1) : 0;
}

