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
#include "Language/Language.hpp"
#include "Input/InputEvents.hpp"
#include "Util/StaticString.hpp"
#include "OS/Clock.hpp"

#include "Math/fixed.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Renderer/OZPreviewRenderer.hpp"
#include "Formatter/AngleFormatter.hpp"

#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Look/MapLook.hpp"
#include "Look/TaskLook.hpp"
#include "Look/InfoBoxLook.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Screen/Color.hpp"
#include "Screen/Window.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Canvas.hpp"

#include "Interface.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Terrain/TerrainSettings.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Globals.hpp"
#elif defined(USE_GDI)
#include "Screen/WindowCanvas.hpp"
#endif

#include <stdio.h>
#include <stdlib.h>


TaskNavSliderWidget::TaskNavSliderWidget()
  :slider_shape(UIGlobals::GetDialogLook(), UIGlobals::GetLook().info_box),
  task_data_cache(CommonInterface::GetComputerSettings(),
                  CommonInterface::Basic(),
                  CommonInterface::Calculated().task_stats) {}

void
TaskNavSliderWidget::UpdateVisibility(const PixelRect &rc,
                                      bool is_panning,
                                      bool is_main_window_widget,
                                      bool is_map)
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

#ifdef _WIN32
void
TaskNavSliderWidget::PaintBackground(Canvas &canvas, unsigned idx,
                                     const DialogLook &dialog_look,
                                     const PixelRect rc_outer)
{
  // clear area b/c Win32 does not draw background transparently
  UPixelScalar x_offset = rc_outer.left;
  if (idx == 0) {
    RasterPoint left_mid = slider_shape.GetPoint(7);
    canvas.DrawFilledRectangle(0, 0, x_offset + left_mid.x, rc_outer.bottom,
                               Brush(dialog_look.list.GetBackgroundColor(
                                     false, true, false)));
  }
  if (idx == (GetList().GetLength() - 1)){
    RasterPoint right_mid = slider_shape.GetPoint(3);
    canvas.DrawFilledRectangle(x_offset + right_mid.x + 1, 0,
                               x_offset + right_mid.x + slider_shape.GetHintWidth() + 1,
                               rc_outer.bottom,
                               Brush(dialog_look.list.GetBackgroundColor(
                                     false, true, false)));
  }
}
#endif

void
TaskNavSliderWidget::OnPaintItem(Canvas &canvas, const PixelRect rc_outer,
                                 unsigned idx)
{
  // if task is not current (e.g. the tp being drawn may no longer exist) then abort drawing
  // hold lease on task_manager until drawing is done
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  if (task_manager->GetTaskTimeStamp() != task_data_cache.GetTaskManagerTimeStamp())
    return;

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const IconLook &icon_look = UIGlobals::GetIconLook();
  const MapSettings &settings_map = CommonInterface::GetMapSettings();
  const TerrainRendererSettings &terrain = settings_map.terrain;

  TaskNavDataCache::tp_info tp = task_data_cache.GetPoint(idx);
  bool draw_checkmark =
      (task_data_cache.GetTaskMode() == TaskManager::MODE_ORDERED)
      && (task_data_cache.GetOrderedTaskSize() > 1)
      && ((idx > 0 && tp.GetHasEntered()) || (idx == 0 && tp.GetHasEntered()));

  const bool selected = GetList().GetCursorDownIndex() == (int)idx;

  StaticString<120> buffer;
  const Font &name_font = slider_shape.GetLargeFont();
  const Font &small_font = slider_shape.GetSmallFont();
  const Font &medium_font = slider_shape.GetMediumFont();
  UPixelScalar width;
  UPixelScalar distance_width = 0u;
  PixelRect rc = rc_outer;
  rc.left += slider_shape.GetHintWidth();
  rc.right -= slider_shape.GetHintWidth();
  unsigned border_width = Layout::ScalePenWidth(terrain.enable ? 1 : 2);

  if (!tp.IsValid()) {
    canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
    canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
      selected, true, false)));
    slider_shape.Draw(canvas, rc_outer, border_width);
    canvas.Select(small_font);
    buffer = _("Click to navigate");
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.text(rc.left + (rc.right - rc.left - width) / 2,
                rc.top + (rc.bottom - rc.top - small_font.GetHeight()) / 2,
                buffer.c_str());
#ifdef _WIN32
    TaskNavSliderWidget::PaintBackground(canvas, idx, dialog_look, rc_outer);
#endif
    return;
  }

  canvas.SetTextColor(dialog_look.list.GetTextColor(selected, true, false));
  canvas.Select(Brush(dialog_look.list.GetBackgroundColor(
    selected, true, false)));
  slider_shape.Draw(canvas, rc_outer, border_width);
#ifdef _WIN32
    TaskNavSliderWidget::PaintBackground(canvas, idx, dialog_look, rc_outer);
#endif

  const unsigned line_one_y_offset = rc.top + slider_shape.GetLine1Y();
  const unsigned line_two_y_offset = rc.top + slider_shape.GetLine2Y();

  // Draw turnpoint name
  canvas.Select(name_font);
  PixelSize bitmap_size {0, 0};
  UPixelScalar left_bitmap;
  const Bitmap *bmp = &icon_look.hBmpCheckMark;
  if (draw_checkmark) {
    bitmap_size = bmp->GetSize();
  }
  width = canvas.CalcTextWidth(tp.waypoint->name.c_str()) + bitmap_size.cx / 2;
  if (width > (rc_outer.right - rc_outer.left)) {
    canvas.text_clipped(rc_outer.left + bitmap_size.cx / 2,
                        line_two_y_offset,
                        rc_outer.right - rc_outer.left - bitmap_size.cx / 2,
                        tp.waypoint->name.c_str());
    left_bitmap = rc_outer.left;
  } else {
    left_bitmap = rc_outer.left + (rc_outer.right - rc_outer.left - width) / 2;
    canvas.text(left_bitmap + bitmap_size.cx / 2,
                line_two_y_offset,
                tp.waypoint->name.c_str());
  }

  // draw checkmark next to name if oz entered
  if (draw_checkmark) {
    const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
    canvas.CopyAnd(left_bitmap,
                    rc.top + offsety,
                    bitmap_size.cx / 2,
                    bitmap_size.cy,
                    *bmp,
                    bitmap_size.cx / 2, 0);
  }

  // Draw distance to turnpoint
  if (tp.distance_valid) {

    canvas.Select(medium_font);
    FormatUserDistanceSmart(tp.distance, buffer.buffer(), true);
    distance_width = canvas.CalcTextWidth(buffer.c_str());
    canvas.text(rc.right - Layout::FastScale(2) - distance_width,
                line_one_y_offset, buffer.c_str());
  }

  // Draw arrival altitude
  canvas.Select(small_font);
  if (tp.altitude_difference_valid) {
    FormatRelativeUserAltitude(tp.altitude_difference, buffer.buffer(), true);
    width = canvas.CalcTextWidth(buffer.c_str());
    width += distance_width;
    canvas.text(rc.right - canvas.CalcTextWidth(_T("     ")) - width,
                line_one_y_offset, buffer.c_str());
  }


  // draw title "goto" abort, tp#
  switch (task_data_cache.GetTaskMode()) {
  case TaskManager::MODE_ORDERED:
    if (task_data_cache.GetOrderedTaskSize() == 0)
      buffer = _("Go'n home:");

    else if (idx == 0)
      buffer = _("Start");
    else if (idx + 1 == task_data_cache.GetOrderedTaskSize())
        buffer = _("Finish");
    else
      _stprintf(buffer.buffer(), _T("TP# %u"), idx);

    break;
  case TaskManager::MODE_GOTO:
  case TaskManager::MODE_ABORT:
    buffer = _("Goto:");
    break;

  case TaskManager::MODE_NULL:
    buffer = _("Go'n home:");

    break;
  }
  width = canvas.CalcTextWidth(buffer.c_str());
  canvas.Select(small_font);
  canvas.text(rc.left + Layout::FastScale(2),
              line_one_y_offset, buffer.c_str());

  // bearing delta waypoint for ordered when not start
  // or for non ordered task
  // TODO make this configurable to show delta or true bearing
  bool do_bearing = false;
  Angle bearing;
  if (tp.bearing_valid && task_data_cache.GetTaskMode() ==
      TaskManager::MODE_ORDERED && idx > 0) {
    do_bearing = true;
    bearing = tp.delta_bearing;
  } else if (task_data_cache.GetTaskMode() != TaskManager::MODE_ORDERED &&
      tp.bearing_valid) {
    do_bearing = true;
    bearing = tp.delta_bearing;
  }

  if (false && do_bearing) {
    FormatAngleDelta(buffer.buffer(), buffer.MAX_SIZE, bearing);
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.Select(small_font);
    canvas.text((rc.left + rc.right - width) / 2,
                line_one_y_offset, buffer.c_str());
  }
}

void
TaskNavSliderWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
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
  return task_manager->GetTaskTimeStamp() ==
      task_data_cache.GetTaskManagerTimeStamp();
}

void
TaskNavSliderWidget::ReadWaypointIndex()
{

  int task_manager_current_index = GetCurrentWaypoint();
  if (task_manager_current_index == -1) {
    return; // non-ordered mode
  }

  if ((task_manager_current_index != (int)waypoint_index) && GetList().IsSteady())
      GetList().ScrollToItem(task_manager_current_index);
}

HorizontalListControl &
TaskNavSliderWidget::CreateListEmpty(ContainerWindow &parent,
                                const DialogLook &look,
                                const PixelRect &rc)
{
  WindowStyle list_style;
  list_style.TabStop();

  HorizontalListControl *list =
    new HorizontalListControl(parent, look, rc, list_style, 20);
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
  task_data_cache.SetActiveTaskPointIndex(task_manager->GetActiveTaskPointIndex());

  if (task_manager->GetMode() != TaskManager::MODE_ORDERED)
    return -1;

  return task_data_cache.GetActiveTaskPointIndex();
}

void
TaskNavSliderWidget::Unprepare()
{
  WindowWidget::Unprepare();
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
  --rc.top;
  rc.bottom = rc.top + slider_shape.GetHeight();
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

  if (InputEvents::IsMode(menu_ordered.buffer()) ||
      InputEvents::IsMode(menu_goto.buffer()))
    InputEvents::HideMenu();
  else if (task_data_cache.GetTaskMode() == TaskManager::MODE_GOTO ||
        task_data_cache.GetTaskMode() == TaskManager::MODE_ABORT)
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

