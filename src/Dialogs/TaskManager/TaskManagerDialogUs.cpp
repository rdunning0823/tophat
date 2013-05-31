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

#include "TaskManagerDialogUs.hpp"
#include "UIGlobals.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/dlgTaskPointUs.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Dialogs/Message.hpp"
#include "Screen/Layout.hpp"
#include "Components.hpp"
#include "Gauge/TaskView.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "OS/FileUtil.hpp"
#include "Protection.hpp"
#include "Look/Look.hpp"
#include "Form/Frame.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Screen/Fonts.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Profile/Profile.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/ObservationZones/ObservationZonePoint.hpp"
#include "Logger/ExternalLogger.hpp"
#include "Device/Declaration.hpp"
#include "Screen/SingleWindow.hpp"


#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#endif

#include <assert.h>
#include <stdio.h>

static void UpdateTaskDefaults(OrderedTask &task);

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

TaskManagerDialogUs::TaskManagerDialogUs(const DialogLook &look,
                                         OrderedTask **_active_task_pointer,
                                         bool _task_modified)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _("Task Manager"), GetDialogStyle()),
   active_task_pointer(_active_task_pointer),
   active_task(*active_task_pointer),
   task_modified(_task_modified),
   fullscreen(false), task_view(active_task_pointer)
{
}

void
TaskManagerDialogUs::OnAction(int id)
{

  if (id == SAVE_AS) {
    SaveTask();
  }

  else if (id == FLY) {
    CommitTaskChanges();
    SetModalResult(mrOK);
  }

  else if (id == REVERT) {
    UpdateTaskDefaults(*active_task);
    SetModalResult(mrOK);
  }
}

void
TaskManagerDialogUs::UpdateButtons()
{
  revert_button->SetVisible(task_modified);
}

void
TaskManagerDialogUs::Unprepare()
{
  delete fly_button;
  delete revert_button;
  delete save_as_button;
  delete task_summary;
}

void
TaskManagerDialogUs::Declare()
{
  if (!active_task->CheckTask())
    return;

  if (ExternalLogger::LoggerAttachedCount() == 0)
    return;

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const DerivedInfo &calculated = XCSoarInterface::Calculated();

  if (calculated.flight.flying)
    return;

  Declaration decl(settings.logger, settings.plane, active_task);
  ExternalLogger::Declare(decl, way_points.GetHome());
}

bool
TaskManagerDialogUs::Save(bool &changed, bool &require_restart)
{
  Declare();
  return true;
}

void
TaskManagerDialogUs::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  NullWidget::Prepare(parent, rc);
  WndForm::Move(rc);
  SetRectangles(rc);

  WindowStyle style_frame;
  style_frame.Border();
  task_summary = new WndFrame(GetClientAreaWindow(), look,
                              rc_task_summary.left, rc_task_summary.top,
                              rc_task_summary.right - rc_task_summary.left,
                              rc_task_summary.bottom - rc_task_summary.top,
                              style_frame);
  task_summary->SetFont(Fonts::infobox_small);
  StaticString<300> text;
  OrderedTaskSummary(active_task, text.buffer(), false);
  task_summary->SetCaption(text.c_str());

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  fly_button = new WndButton(GetClientAreaWindow(), look, _T("Fly"),
                             rc_fly_button,
                             button_style, this, FLY);

  revert_button = new WndButton(GetClientAreaWindow(), look, _T("Revert"),
                             rc_revert_button,
                             button_style, this, REVERT);

  save_as_button = new WndButton(GetClientAreaWindow(), look, _T("Save as"),
                             rc_save_as_button,
                             button_style, this, SAVE_AS);

  WindowStyle style;
  task_view.set(GetClientAreaWindow(),rc_task_view, style);
  task_view.SetFullScreenRect(GetClientRect());
  task_view.SetPartialScreenRect(rc_task_view);

  UpdateButtons();
}

void
TaskManagerDialogUs::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  rc.left = rc.top = Layout::Scale(2);
  rc.right = rc_outer.right - rc_outer.left - Layout::Scale(2);
  rc.bottom = rc_outer.bottom - rc_outer.top - Layout::Scale(2) -
      WndForm::GetTitleHeight();

  UPixelScalar button_height = Layout::Scale(35);
  UPixelScalar button_width = (rc.right - rc.left) / 3;

  rc_task_view = rc_fly_button = rc_revert_button = rc_save_as_button =
      rc_task_summary = rc;

  rc_fly_button.top = rc_revert_button.top = rc_save_as_button.top =
      rc.bottom - button_height;

  rc_fly_button.right = rc_fly_button.left + button_width;

  rc_save_as_button.left = rc_fly_button.right + 1;
  rc_save_as_button.right = rc_save_as_button.left + button_width;

  rc_revert_button.left = rc_save_as_button.right + 1;
  rc_revert_button.right = rc_revert_button.left + button_width;

  rc_task_summary.bottom = rc_revert_button.top - 1;
  rc_task_summary.top = rc_task_summary.bottom - 2 * button_height;

  UPixelScalar task_view_width = min(rc_task_summary.top - rc.top,
                                     rc.right - rc.left);
  rc_task_view.right = rc_task_view.left + task_view_width;
  rc_task_view.bottom = rc_task_view.top + task_view_width;
}

TaskManagerDialogUs::TaskView::TaskView(OrderedTask **_ordered_task_pointer)
  :ordered_task_pointer(_ordered_task_pointer), fullscreen(false) {}


bool
TaskManagerDialogUs::TaskView::OnMouseUp(PixelScalar x, PixelScalar y)
{
   if (!fullscreen) {
     Move(rc_full_screen);
   } else {
     Move(rc_partial_screen);
   }
   ShowOnTop();
   fullscreen = !fullscreen;

   return true;
}

void
TaskManagerDialogUs::TaskView::Restore()
{
  fullscreen = false;
  Move(rc_partial_screen);
}

void
TaskManagerDialogUs::TaskView::SetFullScreenRect(PixelRect _rc_full_screen)
{
  rc_full_screen = _rc_full_screen;
}

void
TaskManagerDialogUs::TaskView::SetPartialScreenRect(PixelRect _rc_partial_screen)
{
  rc_partial_screen = _rc_partial_screen;
}

void
TaskManagerDialogUs::TaskView::OnPaint(Canvas &canvas)
{
  PaintWindow::OnPaint(canvas);

#ifdef ENABLE_OPENGL
  /* enable clipping */
  GLCanvasScissor scissor(canvas);
#endif

  const MapLook &map_look = UIGlobals::GetMapLook();
  const NMEAInfo &basic = CommonInterface::Basic();

  PaintTask(canvas, GetClientRect(), **ordered_task_pointer,
            basic.location_available, basic.location,
            XCSoarInterface::GetMapSettings(),
            map_look.task, map_look.airspace,
            terrain, &airspace_database);
}

/**
 * updates profile values for task defaults so
 */
static void UpdateTaskDefaults(OrderedTask &task)
{
  if (task.TaskSize() < 2)
    return;

  const AbstractTaskFactory& factory = task.GetFactory();
  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;
  const OrderedTaskBehaviour &otb = task.GetOrderedTaskBehaviour();

  unsigned index = 0;
  auto point_type = factory.GetType(task.GetPoint(index));
  Profile::Set(ProfileKeys::StartType, (unsigned)point_type);
  task_behaviour.sector_defaults.start_type = (TaskPointFactoryType)point_type;

  fixed radius = factory.GetOZSize(task.GetPoint(index).GetObservationZone());
  Profile::Set(ProfileKeys::StartRadius, radius);
  task_behaviour.sector_defaults.start_radius = radius;

  index = task.TaskSize() - 1;
  point_type = factory.GetType(task.GetPoint(index));
  Profile::Set(ProfileKeys::FinishType, (unsigned)point_type);
  task_behaviour.sector_defaults.finish_type = (TaskPointFactoryType)point_type;

  radius = factory.GetOZSize(task.GetPoint(index).GetObservationZone());
  Profile::Set(ProfileKeys::FinishRadius, radius);
  task_behaviour.sector_defaults.finish_radius = radius;

  if (task.TaskSize() > 2) {
    index = 1;
    point_type = (TaskPointFactoryType)factory.GetType(task.GetPoint(index));
    Profile::Set(ProfileKeys::TurnpointType, (unsigned)point_type);
    task_behaviour.sector_defaults.turnpoint_type = (TaskPointFactoryType)point_type;

    radius = factory.GetOZSize(task.GetPoint(index).GetObservationZone());
    Profile::Set(ProfileKeys::TurnpointRadius, radius);
    task_behaviour.sector_defaults.turnpoint_radius = radius;
  }

  auto factory_type = task.GetFactoryType();
  Profile::Set(ProfileKeys::TaskType, (unsigned)factory_type);
  task_behaviour.task_type_default = task.GetFactoryType();


  if (task.GetFactoryType() == TaskFactoryType::AAT ||
      task.GetFactoryType() == TaskFactoryType::MAT) {

    task_behaviour.ordered_defaults.aat_min_time = otb.aat_min_time;
    Profile::Set(ProfileKeys::AATMinTime, otb.aat_min_time);
  }

  task_behaviour.ordered_defaults.start_max_speed = otb.start_max_speed;
  Profile::Set(ProfileKeys::StartMaxSpeed, otb.start_max_speed);

  task_behaviour.ordered_defaults.start_max_height = otb.start_max_height;
  Profile::Set(ProfileKeys::StartMaxHeight, otb.start_max_height);

  task_behaviour.ordered_defaults.start_max_height_ref = otb.start_max_height_ref;
  Profile::Set(ProfileKeys::StartHeightRef, (unsigned)otb.start_max_height_ref);

  task_behaviour.ordered_defaults.finish_min_height = otb.finish_min_height;
  Profile::Set(ProfileKeys::FinishMinHeight, otb.finish_min_height);

  task_behaviour.ordered_defaults.finish_min_height_ref = otb.finish_min_height_ref;
  Profile::Set(ProfileKeys::FinishHeightRef, (unsigned)otb.finish_min_height_ref);

  Profile::Save();
}

void
TaskManagerDialogUs::SaveTask()
{
  active_task->GetFactory().CheckAddFinish();

  if (active_task->CheckTask()) {
    if (!OrderedTaskSave(UIGlobals::GetMainWindow(), *active_task, true))
      return;

  } else {
    ShowMessageBox(getTaskValidationErrors(
        active_task->GetFactory().GetValidationErrors()), _("Task not saved"),
        MB_ICONEXCLAMATION);
  }
}

bool
TaskManagerDialogUs::CommitTaskChanges()
{
  if (!task_modified)
    return true;

  task_modified |= active_task->GetFactory().CheckAddFinish();

  if (!active_task->TaskSize() || active_task->CheckTask()) {

    { // this must be done in thread lock because it potentially changes the
      // waypoints database
      ScopeSuspendAllThreads suspend;
      active_task->CheckDuplicateWaypoints(way_points);
      way_points.Optimise();
    }

    protected_task_manager->TaskCommit(*active_task, way_points);
    protected_task_manager->TaskSaveDefault();
    UpdateTaskDefaults(*active_task);

    task_modified = false;
    return true;
  }

  ShowMessageBox(getTaskValidationErrors(
    active_task->GetFactory().GetValidationErrors()),
    _("Validation Errors"), MB_ICONEXCLAMATION | MB_OK);

  return (ShowMessageBox(_("Task not valid. Changes will be lost.\nContinue?"),
                      _("Task Manager"), MB_YESNO | MB_ICONQUESTION) == IDYES);
}

void
TaskManagerDialogUsShowModal(SingleWindow &parent)
{
  bool changed, require_restart;

  if (protected_task_manager == NULL)
    return;

  OrderedTask *active_task = protected_task_manager->TaskClone();
  active_task->FillMatPoints(way_points);

  const DialogLook &look = UIGlobals::GetDialogLook();

  TaskEditorReturn task_editor_return = dlgTaskPointUsShowModal(parent, &active_task, 0);

  if (task_editor_return == TaskEditorReturn::TASK_REVERT) {
    delete active_task;
    return;
  }

  bool task_modified = task_editor_return == TaskEditorReturn::TASK_MODIFIED;

  ContainerWindow &w = UIGlobals::GetMainWindow();
  TaskManagerDialogUs *instance;
  instance = new TaskManagerDialogUs(look, &active_task, task_modified);

  instance->Initialise(w, w.GetClientRect());
  instance->Prepare(w, w.GetClientRect());
  instance->ShowModal();
  instance->Save(changed, require_restart);
  instance->Hide();
  instance->Unprepare();
  delete instance;



  delete active_task;
}
