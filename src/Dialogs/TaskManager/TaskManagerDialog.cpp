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

#include "Internal.hpp"
#include "TaskEditPanel.hpp"
#include "TaskPropertiesPanel.hpp"
#include "TaskMiscPanel.hpp"
#include "TaskClosePanel.hpp"
#include "UIGlobals.hpp"
#include "Look/IconLook.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Dialogs/XML.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Components.hpp"
#include "Gauge/TaskView.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/TaskStore.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "Logger/Logger.hpp"
#include "Protection.hpp"
#include "Look/Look.hpp"
#include "Form/Form.hpp"
#include "Form/TabBar.hpp"
#include "Form/Panel.hpp"
#include "Form/Draw.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Profile/Profile.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/ObservationZones/ObservationZonePoint.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#endif

#include <assert.h>
#include <stdio.h>

static WndForm *wf = NULL;

static TabBarControl* wTabBar = NULL;
static OrderedTask* active_task = NULL;
static bool task_modified = false;
static bool fullscreen;
static PixelRect TaskViewRect;
static unsigned TurnpointTab = 0;
static unsigned PropertiesTab = 0;

unsigned
dlgTaskManager::GetTurnpointTab()
{
  return TurnpointTab;
}

unsigned
dlgTaskManager::GetPropertiesTab()
{
  return PropertiesTab;
}

void
dlgTaskManager::SetTitle()
{
  StaticString<128> title;
  title.Format(_T("%s - %s"), _("Task Manager"),
               wTabBar->GetButtonCaption((wTabBar->GetCurrentPage())));
  wf->SetCaption(title);
}

bool
dlgTaskManager::OnTaskViewClick(WndOwnerDrawFrame *Sender,
                                PixelScalar x, PixelScalar y)
{
  if (TaskViewRect.right == 0)
    TaskViewRect = Sender->GetPosition();

  if (!fullscreen) {
    const UPixelScalar xoffset = Layout::landscape ? wTabBar->GetTabWidth() : 0;
    const UPixelScalar yoffset = !Layout::landscape ? wTabBar->GetTabHeight() : 0;
    Sender->Move(xoffset, 0,
                 wf->GetClientAreaWindow().GetWidth() - xoffset,
                 wf->GetClientAreaWindow().GetHeight() - yoffset);
    fullscreen = true;
    Sender->ShowOnTop();
  } else {
    Sender->Move(TaskViewRect);
    fullscreen = false;
  }
  Sender->Invalidate();
  return true;
}

void
dlgTaskManager::TaskViewRestore(WndOwnerDrawFrame *wTaskView)
{
  if (TaskViewRect.right == 0) {
    TaskViewRect = wTaskView->GetPosition();
    return;
  }

  fullscreen = false;
  wTaskView->Move(TaskViewRect);
}

void
dlgTaskManager::ResetTaskView(WndOwnerDrawFrame *task_view)
{
  assert(task_view != NULL);

  task_view->Hide();
  TaskViewRestore(task_view);
  task_view->SetOnPaintNotify(OnTaskPaint);
  task_view->SetOnMouseDownNotify(dlgTaskManager::OnTaskViewClick);
}

void
dlgTaskManager::OnTaskPaint(WndOwnerDrawFrame *Sender, Canvas &canvas)
{
#ifdef ENABLE_OPENGL
  /* enable clipping */
  GLCanvasScissor scissor(canvas);
#endif

  const MapLook &look = UIGlobals::GetMapLook();
  const NMEAInfo &basic = CommonInterface::Basic();
  PaintTask(canvas, Sender->GetClientRect(), *active_task,
            basic.location_available, basic.location,
            XCSoarInterface::GetMapSettings(),
            look.task, look.airspace,
            terrain, &airspace_database);
}

/*template<typename T>
bool SetKeyEnum(unsigned i, const TCHAR *registry_key, T &value) const {

  int value2 = (int)value;
  if (!SaveValue(i, registry_key, value2))
    return false;

  value = (T)value2;
  return true;
}*/

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

bool
dlgTaskManager::CommitTaskChanges()
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
    _("Validation Errors"), MB_ICONEXCLAMATION);

  return (ShowMessageBox(_("Task not valid. Changes will be lost.\nContinue?"),
                      _("Task Manager"), MB_YESNO | MB_ICONQUESTION) == IDYES);
}

bool
dlgTaskManager::OnClose()
{
  if (CommitTaskChanges()) {
    wf->SetModalResult(mrOK);
    return true;
  }

  return false;
}

void
dlgTaskManagerShowModal(SingleWindow &parent)
{
  dlgTaskManager::dlgTaskManagerShowModal(parent);
}

void
dlgTaskManager::RevertTask()
{
  // create new task first to guarantee pointers are different
  OrderedTask* temptask = protected_task_manager->TaskClone();
  delete active_task;
  active_task = temptask;
  task_modified = false;
}

void
dlgTaskManager::dlgTaskManagerShowModal(SingleWindow &parent)
{
  if (protected_task_manager == NULL)
    return;

  /* invalidate the preview layout */
  TaskViewRect.right = 0;

  wf = LoadDialog(NULL, parent,
                  Layout::landscape ?
                  _T("IDR_XML_TASKMANAGER_L") : _T("IDR_XML_TASKMANAGER"));

  assert(wf != NULL);

  active_task = protected_task_manager->TaskClone();
  active_task->FillMatPoints(way_points);
  task_modified = false;

  // Load tabs
  wTabBar = (TabBarControl*)wf->FindByName(_T("TabBar"));
  assert(wTabBar != NULL);

  wTabBar->SetPageFlippedCallback(SetTitle);

  const MapLook &look = UIGlobals::GetMapLook();

  WndOwnerDrawFrame *task_view =
    (WndOwnerDrawFrame *)wf->FindByName(_T("TaskView"));
  assert(task_view != NULL);
  ResetTaskView(task_view);

  TaskPropertiesPanel *wProps =
    new TaskPropertiesPanel(UIGlobals::GetDialogLook(),
                            &active_task, &task_modified);
  wProps->SetTaskView(task_view);

  TaskClosePanel *wClose = new TaskClosePanel(&task_modified);
  wClose->SetTaskView(task_view);

  TaskEditPanel *wEdit = new TaskEditPanel(*wf, look.task, look.airspace,
                                           &active_task, &task_modified);
  wEdit->SetTaskView(task_view);

  TaskMiscPanel *list_tab = new TaskMiscPanel(*wTabBar,
                                              &active_task, &task_modified);
  list_tab->SetTaskView(task_view);

  const bool enable_icons =
    CommonInterface::GetUISettings().dialog.tab_style
    == DialogSettings::TabStyle::Icon;

  const IconLook &icons = UIGlobals::GetIconLook();
  const Bitmap *TurnPointIcon = enable_icons ? &icons.hBmpTabTask : NULL;
  const Bitmap *BrowseIcon = enable_icons ? &icons.hBmpTabWrench : NULL;
  const Bitmap *PropertiesIcon = enable_icons ? &icons.hBmpTabSettings : NULL;


  if (Layout::landscape) {
    wTabBar->AddTab(wEdit, _("Turn Points"), TurnPointIcon);
    TurnpointTab = 0;

    wTabBar->AddTab(list_tab, _("Manage"), BrowseIcon);

    wTabBar->AddTab(wProps, _("Rules"), PropertiesIcon);
    PropertiesTab = 2;

    wTabBar->AddTab(wClose, _("Close"));

    wTabBar->SetCurrentPage(0);
  } else {
    wTabBar->AddTab(wClose, _("Close"));

    wTabBar->AddTab(wEdit, _("Turn Points"), TurnPointIcon);
    TurnpointTab = 1;

    wTabBar->AddTab(list_tab, _("Manage"), BrowseIcon);

    wTabBar->AddTab(wProps, _("Rules"), PropertiesIcon);
    PropertiesTab = 3;

    wTabBar->SetCurrentPage(1);
  }

  fullscreen = false;

  SetTitle();
  wf->ShowModal();

  /* destroy the TabBar first, to have a well-defined destruction
     order; this is necessary because some pages refer to buttons
     belonging to the dialog */
  wTabBar->reset();

  delete wf;
  delete active_task;
}
