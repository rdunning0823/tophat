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

#include "Blackboard/DeviceBlackboard.hpp"
#include "Compiler.h"
#include "Components.hpp"
#include "ComputerSettings.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/Message.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Form/Widget.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/Draw.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "Look/MapLook.hpp"
#include "Operation/MessageOperationEnvironment.hpp"
#include "Protection.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Layout.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/TaskManager.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "UIGlobals.hpp"
#include "Units/Units.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Timer.hpp"
#include "Thread/Notify.hpp"

#include <math.h>
#include <assert.h>

enum ControlIndexMatAdd {
  AddPointClick,
  DeletePointClick,
  ReplacePointClick,
  MoreClick,
  CancelClick,
};

/**
 * the type of change made to a task
 */
enum MatMode {
  MAT_ADD_AFTER_OR_REPLACE_FIRST,
  MAT_ADD_AFTER_OR_REPLACE_INDEX,
  MAT_INSERT_BEFORE_FINISH,
  MAT_DELETE,
};

/**
 * a struct that tracks status of the task and the changes made to it
 * not valid for delete mode
 */
struct ModifiedTask {
  /**
   * nullptr if error creating task
   * not used for delete task
   */
  OrderedTask *task;

  /**
   * the index of the inserted point in the temp task
   * for delete task, the index of the first tp eligible for delete
   */
  unsigned index;

  /**
   * the name of the wp in the old task pointed to in the index
   * not used for delete mode
   */
  StaticString<255> index_wp_name;

  /**
   * the type of change made to the task
   */
  MatMode mat_mode;
  ModifiedTask() {
    task = nullptr;
  }
  bool IsValid() const {
    return task != nullptr;
  }

  const TCHAR *GetCaption() const {
    switch (mat_mode) {
    case MAT_ADD_AFTER_OR_REPLACE_FIRST:
    case MAT_ADD_AFTER_OR_REPLACE_INDEX:
    case MAT_INSERT_BEFORE_FINISH:
      return _("Add MAT point");
    case MAT_DELETE:
      return _("Delete MAT point");
    }
    return _T("");
  }
};

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}


class MatClickPanel : public NullWidget, public WndForm, private Timer//, private Notify
{
public:
  enum ReturnState {
    /**
     * (Either Add, Delete or REPLACE was clicked)
     */
    OK,

    /**
     * need to replace the tp in the task
     * (the widget does not do this)
     */
    REPLACE,

    /**
     * Cancel was clicked
     */
    CANCEL,

    /**
     * the "More" button was clicked
     */
    SHOW_MORE_ITEMS,
  };

private:
  enum DelayCounter {
    DELAY_COUNTER_WAIT = 4,
  };

  const Waypoint &wp_clicked;

  /**
   * is the next waypoint in the task
   * (after which we normally insert) the finish?
   */
  bool next_is_finish;

  /**
   * name of current wp in current task
   */
  const TCHAR *current_wp_name;

  /**
   * the return state of the widget
   */
  ReturnState return_state;

  /**
   * counts the number of seconds before data is valid
   */
  unsigned delay_counter;

  /**
   * the modified task and information about the changes
   */
  const ModifiedTask modified_task;

  PixelRect rc_add, rc_delete, rc_replace, rc_cancel, rc_more;

  WndButton *add_button, *delete_button, *replace_button, *cancel, *more;

  /**
   *  base text (either with "calculating..." or with details appended later)
   */
  StaticString<255> add_base_text;

public:
  MatClickPanel(const Waypoint &_wp_clicked, const ModifiedTask _modified_task)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _modified_task.GetCaption(),
           GetDialogStyle()),
   wp_clicked(_wp_clicked),
   current_wp_name(_modified_task.IsValid() ?
       _modified_task.index_wp_name : wp_clicked.name.c_str()),
       return_state(OK), delay_counter(0),
   modified_task(_modified_task)
  {
    assert(wp_clicked.IsTurnpoint());
  }

  void RefreshFormForAdd();

  /* virtual methods from class Timer */
  virtual void OnTimer() gcc_override;

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual bool Save(bool &changed, bool &require_restart) {
    return true;
  }
  virtual void Show(const PixelRect &rc) {};
  virtual void Hide() {};
  virtual void Move(const PixelRect &rc) {};

  /**
   * returns true if the "Show other options" button was clicked
   */
  ReturnState GetReturnState() {
    return return_state;
  }

  /**
   * returns height of TaskNavSlider bar
   */
  UPixelScalar GetNavSliderHeight();

  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);

  /**
   * from ActionListener
   */
  virtual void OnAction(int id);

private:
  /**
   * Removes the first occurance of the wp in the task including or after the
   * index but not including the finish
   * Displays message with Success/Failure
   * assumes point is in task and not yet achieved
   */
  void RemovePoint();

};

static MatClickPanel *instance;

void
MatClickPanel::RemovePoint()
{
  assert(protected_task_manager != NULL);
  OrderedTask *task;
  {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);
    task = task_manager->Clone(XCSoarInterface::GetComputerSettings().task);
  }
  int delete_index = -1;
  for (unsigned i = modified_task.index; i < task->TaskSize() - 1; i ++) {
    const OrderedTaskPoint &tp = task->GetPoint(i);

    if (tp.GetWaypoint() == wp_clicked) {
      delete_index = i;
      break;
    }
  }

  bool success = false;
  StaticString<255> remove_prompt;
  if (delete_index > -1)
    success = task->Remove(delete_index);

  if (success) {
    if (!task->CheckTask()) {
      remove_prompt.Format(_T("%s %s"),_("Error: could not remove"), wp_clicked.name.c_str());
      ShowMessageBox(getTaskValidationErrors(
          task->GetFactory().GetValidationErrors()),
          remove_prompt.c_str(), MB_ICONERROR | MB_OK);

      delete task;
      return;
    }
    ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
    success = task_manager->Commit(*task, way_points);
  }

  if (success) {
    remove_prompt.Format(_T("%s %s %s"),_("Removed"), wp_clicked.name.c_str(),
                         _("from task"));
    ShowMessageBox(remove_prompt.c_str(), _("Success"), MB_OK);
  } else {
    remove_prompt.Format(_T("%s %s %s"),_("Could not remove"), wp_clicked.name.c_str(),
                         _("from task"));
    ShowMessageBox(remove_prompt.c_str(), _("Error"), MB_OK | MB_ICONERROR);
  }

  delete task;
}

UPixelScalar
MatClickPanel::GetNavSliderHeight()
{
  UPixelScalar large_font_height = UIGlobals::GetLook().info_box.value.font->GetHeight();
  UPixelScalar small_font_height = UIGlobals::GetDialogLook().list.font->GetHeight();

  return large_font_height + 2 * small_font_height - Layout::Scale(3);
}

void
MatClickPanel::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  UPixelScalar side_margin = Layout::landscape ? Layout::Scale(10) : Layout::Scale(4);
  unsigned line_height = Fonts::infobox_small.GetHeight();
  rc.left = side_margin;
  rc.top = Layout::Scale(2);
  rc.right = rc_outer.right - rc_outer.left - side_margin;
  rc.bottom = rc_outer.bottom - rc_outer.top -
      Layout::Scale(2) - GetTitleHeight();

  rc_cancel = rc_more = rc_add = rc_delete = rc_replace = rc;
  rc_cancel.top = rc_more.top = rc.bottom - GetNavSliderHeight();
  rc_cancel.right = (rc.right - rc.left) / 2;
  rc_more.left = rc_cancel.right + 1;

  rc_add.top = line_height;
  rc_add.bottom = rc_add.top + 5 * line_height;
  rc_delete = rc_add;

  rc_replace.top = rc_add.bottom + line_height;
  rc_replace.bottom = rc_replace.top + 4 * line_height;
}

void
MatClickPanel::OnAction(int id)
{
  switch (id) {
  case AddPointClick:
    return_state = OK;
    SetModalResult(mrOK);
    break;

  case DeletePointClick:
    RemovePoint();
    return_state = OK;
    SetModalResult(mrOK);
    break;

  case ReplacePointClick:
    return_state = REPLACE;
    SetModalResult(mrOK);
    break;

  case CancelClick:
    return_state = CANCEL;
    SetModalResult(mrCancel);
    break;

  case MoreClick:
    return_state = SHOW_MORE_ITEMS;
    SetModalResult(mrCancel);
    break;
  }
}

void
MatClickPanel::RefreshFormForAdd()
{
  assert (modified_task.mat_mode != MAT_DELETE);

  if (!Timer::IsActive())
    Timer::Schedule(1000);

  StaticString<20> time_remaining;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  FormatTimespanSmart(time_remaining.buffer(),
                      (int)common_stats.aat_time_remaining, 2);

  if (++delay_counter < DELAY_COUNTER_WAIT) {
    StaticString<255> prompt;
    prompt.Format(_T("%s\n%s: %s\n%s "), add_base_text.c_str(),
                  ("Time remaining"),
                  time_remaining.c_str(),
                  _("Calculating"));
    for (unsigned i = 0; i <= delay_counter; i++)
      prompt.append(_T(".."));
    add_button->SetCaption(prompt.c_str());
    return;
  }

  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  StaticString<32> time;

  FormatTimespanSmart(time.buffer(), (int)common_stats.task_time_remaining, 2);
//  FormatTimespanSmart(time.buffer(), (int)task_stats.total.time_remaining, 2);

  StaticString<25> altitude_text(_T(""));
  fixed altitude_difference = task_stats.total.solution_remaining.altitude_difference;
  if (task_stats.total.solution_remaining.IsDefined())
    FormatRelativeUserAltitude(altitude_difference, altitude_text.buffer(), true);

  StaticString<255> prompt;
  prompt.Format(_T("%s\n%s: %s\n%s %s     %s"), add_base_text.c_str(),
                ("Time remaining"),
                time_remaining.c_str(),
                _("Est. Finish:"), time.c_str(),
                altitude_text.c_str());
  add_button->SetCaption(prompt.c_str());
}

void
MatClickPanel::OnTimer()
{
  if (modified_task.mat_mode != MAT_DELETE)
    RefreshFormForAdd();
}

void
MatClickPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  PixelRect rc_form = rc; //GetSize(rc);
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetRectangles(rc_form);
  StaticString<255> replace_text(_T(""));
  StaticString<255> delete_text;

  switch (modified_task.mat_mode) {
  case MAT_ADD_AFTER_OR_REPLACE_FIRST:
  case MAT_ADD_AFTER_OR_REPLACE_INDEX:
    add_base_text.Format(_T("%s '%s' %s '%s?'"),
                         _("Add"),
                         wp_clicked.name.c_str(),
                         _("to task after"),
                         current_wp_name);
    replace_text.Format(_T("%s '%s' %s\n'%s?'"),
                        _("Replace"),
                        current_wp_name,
                        _("with"),
                        wp_clicked.name.c_str());
    break;

  case MAT_INSERT_BEFORE_FINISH:
    add_base_text.Format(_T("%s '%s' %s '%s?'"),
                         _("Insert"),
                         wp_clicked.name.c_str(),
                         _("before"),
                         current_wp_name);
    break;
  case MAT_DELETE:
    delete_text.Format(_T("%s '%s' %s?"), _("Remove"), wp_clicked.name.c_str(),
                       _("from task"));
    break;
  }

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  if (modified_task.mat_mode == MAT_DELETE) {
    delete_button = new WndButton(GetClientAreaWindow(), dialog_look,
                                  delete_text.c_str(),
                                  rc_delete,
                                  button_style, this, DeletePointClick);
    AddDestruct(delete_button);
  } else {
    add_button = new WndButton(GetClientAreaWindow(), dialog_look,
                               add_base_text.c_str(),
                               rc_add,
                               button_style, this, AddPointClick);
    AddDestruct(add_button);

    if (modified_task.mat_mode != MAT_INSERT_BEFORE_FINISH) {
      replace_button = new WndButton(GetClientAreaWindow(), dialog_look,
                                     replace_text.c_str(),
                                     rc_replace,
                                     button_style, this, ReplacePointClick);
      AddDestruct(replace_button);
    }
  }

  more = new WndButton(GetClientAreaWindow(), dialog_look, _T("More"),
                       rc_more,
                       button_style, this, MoreClick);
  AddDestruct(more);

  cancel = new WndButton(GetClientAreaWindow(), dialog_look, _T("Cancel"),
                         rc_cancel,
                         button_style, this, CancelClick);
  AddDestruct(cancel);

  if (modified_task.mat_mode != MAT_DELETE)
    RefreshFormForAdd();
}

void
MatClickPanel::Unprepare()
{
  Timer::Cancel();
}

/**
 * returns true if current ordered task is valid
 */
static bool
CheckTask()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  return task_manager->GetOrderedTask().CheckTask();
}

/**
 * creates a clone of the current ordered task and returns a pointer to it
 */
static OrderedTask*
CreateClone()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  return task_manager->Clone(XCSoarInterface::GetComputerSettings().task);
}

/**
 * returns intermediate point index in task of latter of active wp or
 * first after last achieved tp.
 */
static unsigned
GetMatIndexToReplaceOrAddAfter(const OrderedTask &task)
{
  unsigned index = max(task.GetActiveIndex(),
                       task.GetLastIntermediateAchieved() + 1);
  if (index + 1 >= task.TaskSize())
    index = task.TaskSize() - 1;
  return index;
}

/** can we delete this wp from the current task
 * (is this wp in front of us in the task)?
 */
static bool
CanDeleteFromMat(const Waypoint &wp)
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  const OrderedTask &task = task_manager->GetOrderedTask();

  assert(task.TaskSize() > 1);
  for (unsigned i = GetMatIndexToReplaceOrAddAfter(task); i < task.TaskSize() - 1; i++) {
    if (task.GetPoint(i).GetWaypoint() == wp)
      return true;
  }
  return false;
}

/**
 * creates a clone of the ordered task and inserts wp
 * after the latter of the currently navigated or last achieved
 * intermediate point
 * @param task_old. a reference to the old task
 * @param wp. the waypoint to be added
 * @return. ModifiedTask struct with the modified task info
 */
static ModifiedTask
CreateCloneWithWaypoint(const OrderedTask &task_old, const Waypoint &wp)
{
  ModifiedTask result;

  if (!task_old.CheckTask())
    return result;
  assert(task_old.TaskSize() > 1);

  unsigned index = GetMatIndexToReplaceOrAddAfter(task_old);

  bool insert_before = index == task_old.TaskSize() - 1;

  const TaskBehaviour &tb = XCSoarInterface::GetComputerSettings().task;
  OrderedTask *task_new = task_old.Clone(tb);

  OrderedTaskPoint *task_point =
      (OrderedTaskPoint*)task_new->GetFactory().CreateIntermediate(wp);
  if (task_point == nullptr) {
    delete task_new;
    return result;
  }
  assert(index + 1 < task_new->TaskSize() || insert_before);
  if (task_new->TaskSize() == 2) {
    if (task_new->Insert(*task_point, 1)) {
      // "insert before finish.  index = 1"
      delete task_point;
      result.task = task_new;
      result.index = 1;
      result.index_wp_name = task_old.GetPoint(result.index).GetWaypoint().name.c_str();
      result.mat_mode = MatMode::MAT_INSERT_BEFORE_FINISH;
      return result;
    }
  } else if (insert_before) {
    if (task_new->Insert(*task_point, index)) {
      // "insert before finish. index = n-1"
      delete task_point;
      result.task = task_new;
      result.index = index;
      result.index_wp_name = task_old.GetPoint(result.index).GetWaypoint().name.c_str();
      result.mat_mode = MatMode::MAT_INSERT_BEFORE_FINISH;
      return result;
    }
  } else if (index == 0) {
    if (task_new->Insert(*task_point, 1)) {
      // new point will have "1" as index "Add after [1st tp] or replace with"
      delete task_point;
      result.task = task_new;
      result.index = 1;
      result.index_wp_name = task_old.GetPoint(result.index).GetWaypoint().name.c_str();
      result.mat_mode = MatMode::MAT_ADD_AFTER_OR_REPLACE_FIRST;
      return result;
    }
  } else {
    if (task_new->Insert(*task_point, index + 1)) {
      // new point will be index + 1 "Add after [tp] or replace with"
      delete task_point;
      result.task = task_new;
      result.index = index;
      result.index_wp_name = task_old.GetPoint(result.index).GetWaypoint().name.c_str();
      result.mat_mode = MatMode::MAT_ADD_AFTER_OR_REPLACE_INDEX;
      return result;
    }
  }

  delete task_point;
  delete task_new;
  result.task = nullptr;
  return result;
}

/**
 * wrapper with Lease for ShoudAddToMat()
 * @param wp. the waypoint to be tested
 * @return. true if the wp should be added after the
 * last achieved intermediate tp in task
 */
/*
static bool
ShouldAddToMat(const Waypoint &wp_new)
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  const OrderedTask &task = task_manager->GetOrderedTask();

  return task.ShouldAddToMatAchieved(wp_new);
}
*/

/**
 * commits the task to the flight computer
 * @param task. a valid ordered task
 */
static bool
CommitTask(const OrderedTask &task_new)
{
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  if (task_manager->Commit(task_new, way_points)) {
    const OrderedTask &task = task_manager->GetOrderedTask();
    task_manager->SetActiveTaskPoint(task.GetLastIntermediateAchieved() + 1u);
    return true;
  }
  return false;
}

/**
 * show the dialog in either Add or Delete mode
 * @return.  The return state of the dialog
 */
static MatClickPanel::ReturnState
ShowDialog(const Waypoint &wp, const ModifiedTask modified_task)
{
  // add point to task
  ContainerWindow &w = UIGlobals::GetMainWindow();
  instance = new MatClickPanel(wp, modified_task);

  instance->Initialise(w, w.GetClientRect());
  instance->Prepare(w, w.GetClientRect());
  instance->ShowModal();
  MatClickPanel::ReturnState return_state = instance->GetReturnState();

  instance->Hide();
  instance->Unprepare();
  delete instance;

  return return_state;
}

/** alters the task by replacing the wp
 *
 * @param task. the task to be altered
 * @param index. index of wp to be replaced
 * @param wp.  new wp
 * return true if successful
 */
static bool
ReplaceInTask(OrderedTask *task, unsigned index, const Waypoint &wp)
{
  assert(index > 0);
  assert(index + 1 < task->TaskSize());
  assert(task->TaskSize() > 1);

  OrderedTaskPoint *task_point =
      (OrderedTaskPoint*)task->GetFactory().CreateIntermediate(wp);
  if (task_point == nullptr)
    return false;

  if (!task->Replace(*task_point, index)) {
    delete task_point;
    return false;
  }
  return true;
}

/**
 * prompts to add the waypoint
 * Commits the "what if" task before displaying so that
 * the statistics about the task can be displayed
 * Commits the original task if the "Add" button is not clicked
 * @return True if "More items" is clicked
 */
static bool
DoAdd(const Waypoint &wp)
{
  OrderedTask *task_old = CreateClone();
  assert(task_old->TaskSize() > 1);

  ModifiedTask result = CreateCloneWithWaypoint(*task_old, wp);

  if (!result.IsValid()) {
    ShowMessageBox(_("Could not insert waypoint into task"), _("Error"), MB_OK | MB_ICONERROR);
    return false;
  }

  if (!CommitTask(*result.task)) {
    ShowMessageBox(_("Failed to commit new task"), _("Error"), MB_OK | MB_ICONERROR);
    delete result.task;
    delete task_old;
    return false;
  }
  MatClickPanel::ReturnState return_state = ShowDialog(wp, result);

  switch (return_state) {
  case MatClickPanel::ReturnState::OK:
    break;

  case MatClickPanel::ReturnState::REPLACE:
    if (!ReplaceInTask(task_old, result.index, wp)) {
      ShowMessageBox(_("Failed to replace point in task"), _("Error"), MB_OK | MB_ICONERROR);
      break;
    }
  case MatClickPanel::ReturnState::CANCEL:
  case MatClickPanel::ReturnState::SHOW_MORE_ITEMS:
    if (!CommitTask(*task_old))
      ShowMessageBox(_("Failed to restore task"), _("Error"), MB_OK | MB_ICONERROR);
    break;
  }

  delete result.task;
  delete task_old;

  return return_state == MatClickPanel::ReturnState::SHOW_MORE_ITEMS;
}

/**
 * prompt to delete the task point from the Mat
 * Assumes point is in task and not yet achieved
 * @return True if "More items" was clicked
 */
static bool
DoDelete(const Waypoint &wp)
{
  ModifiedTask dummy;
  const OrderedTask *task;
  {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);
    task = &task_manager->GetOrderedTask();
  }
  dummy.index = GetMatIndexToReplaceOrAddAfter(*task);
  dummy.mat_mode = MAT_DELETE;
  return ShowDialog(wp, dummy) ==
      MatClickPanel::ReturnState::SHOW_MORE_ITEMS;
}

/**
 * show either the Add or the Delete Mat item dialog
 * @return True if "More items" was clicked
 */
bool
dlgMatItemClickShowModal(const Waypoint &wp)
{
  if (!CheckTask())
    return true; // show "Other" options
  assert(protected_task_manager != NULL);

  // ask to remove it if it's already the next TP
  if (CanDeleteFromMat(wp))
    return DoDelete(wp);
  else
    return DoAdd(wp);
}
