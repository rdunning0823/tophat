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
#include "Task/MapTaskManager.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/TaskManager.hpp"
#include "UIGlobals.hpp"
#include "Units/Units.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Timer.hpp"
#include "Thread/Notify.hpp"

#include <math.h>
#include <assert.h>

enum ControlIndex {
  SPEED_ACHIEVED = 100,
  DISTANCE,
  AAT_TIME,
  AAT_ESTIMATED,
  CancelClick,
  AddPointClick,
  MoreClick,
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

enum DelayCounter {
  DELAY_COUNTER_WAIT = 4,
};



class MatClickPanel : public NullWidget, public WndForm, private Timer//, private Notify
{
  //OrderedTask &task;
  const Waypoint &wp;

  /**
   * set to true if "Show more options" button is clicked
   */
  bool show_more_options;

  /**
   * counts the number of seconds before data is valid
   */
  unsigned delay_counter;

  PixelRect rc_prompt_text, rc_ok, rc_cancel, rc_more;
  PixelRect rc_labels, rc_values;

  WndFrame *prompt_text;
  WndFrame *labels, *values;
  DataFieldFloat *speed, *time, *distance;
  WndButton *ok, *cancel, *more;

public:
  MatClickPanel(const Waypoint &_wp)
    : WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
              UIGlobals::GetMainWindow().GetClientRect(),
              _T("Add MAT point"), GetDialogStyle()),
     wp(_wp),show_more_options(false), delay_counter(0)
  {
    assert(wp.IsTurnpoint());
    //assert(task.GetFactoryType() == TaskFactoryType::MAT);
  }

  void RefreshForm();

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
  bool GetShowMoreOptions() {
    return show_more_options;
  }
  /**
   * returns on full screen less height of the NavSliderWidget
   */
  virtual PixelRect GetSize(const PixelRect &rc);

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

};

static MatClickPanel *instance;

UPixelScalar
MatClickPanel::GetNavSliderHeight()
{
  UPixelScalar large_font_height = UIGlobals::GetLook().info_box.value.font->GetHeight();
  UPixelScalar small_font_height = UIGlobals::GetDialogLook().list.font->GetHeight();

  return large_font_height + 2 * small_font_height - Layout::Scale(3);
}

PixelRect
MatClickPanel::GetSize(const PixelRect &rc)
{
  UPixelScalar nav_slider_height = GetNavSliderHeight();

  PixelRect rc_form = rc;
  rc_form.left += Layout::landscape ? nav_slider_height / 2 :
      Layout::Scale(3);
  rc_form.right -= Layout::landscape ? nav_slider_height / 2 :
      Layout::Scale(3);
  rc_form.top = nav_slider_height + Layout::Scale(3);

  UPixelScalar height = min(rc.bottom - rc_form.top,
                            (PixelScalar)nav_slider_height * 4);
  rc_form.bottom = rc_form.top + height;

  return rc_form;
}

void
MatClickPanel::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  rc.left = rc.top = Layout::Scale(2);
  rc.right = rc_outer.right - rc_outer.left - Layout::Scale(2);
  rc.bottom = rc_outer.bottom - rc_outer.top -
      Layout::Scale(2) - GetTitleHeight();


  rc_prompt_text = rc_ok = rc_cancel = rc_more = rc_labels = rc_values = rc;

  rc_prompt_text.bottom = rc.top + Layout::Scale(35);

  rc_labels.top = rc_values.top = rc_prompt_text.bottom + 1;
  rc_labels.bottom = rc_values.bottom = rc_labels.top + Layout::Scale(45);
  rc_labels.right = (rc.right - rc.left) / 2;
  rc_values.left = rc_labels.right + 1;

  rc_ok.top = rc_cancel.top = rc_more.top = rc.bottom - GetNavSliderHeight();
  rc_ok.right = rc_more.left = (rc.right - rc.left) / 3;
  rc_more.right = rc_cancel.left = 2 * rc_more.left;
}

void
MatClickPanel::OnAction(int id)
{
  if (id == AddPointClick)
    SetModalResult(mrOK);

  else if (id == CancelClick)
    SetModalResult(mrCancel);

  else if (id == MoreClick) {
    show_more_options = true;
    SetModalResult(mrCancel);
  }
}

void
MatClickPanel::RefreshForm()
{
  if (!Timer::IsActive())
    Timer::Schedule(1000);

  if (++delay_counter < DELAY_COUNTER_WAIT) {
    StaticString<255> prompt;
    prompt.Format(_T("%s "), _("Calculating"));
    for (unsigned i = 0; i <= delay_counter; i++)
      prompt.append(_T(".."));
    labels->SetCaption(prompt.c_str());
    return;
  }

  labels->SetCaption(_T("Total time:\nDist. remaining:"));

  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  StaticString<32> time;

  FormatTimespanSmart(time.buffer(), (int)common_stats.task_time_remaining, 2);


   StaticString<24> distance;

   fixed rPlanned = task_stats.total.solution_remaining.IsDefined()
    ? task_stats.total.solution_remaining.vector.distance
    : fixed_zero;

  if (positive(rPlanned))
    FormatUserDistanceSmart(rPlanned, distance.buffer(), true);
  else
    distance.clear();

  StaticString<64> values_text;
  values_text.Format(_T("%s\n%s"), time.c_str(), distance.c_str());
  values->SetCaption(values_text.c_str());
}

void
MatClickPanel::OnTimer()
{
  RefreshForm();
}

void
MatClickPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  PixelRect rc_form = GetSize(rc);
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetRectangles(rc_form);

  WindowStyle style_frame;
  style_frame.Border();

  prompt_text = new WndFrame(GetClientAreaWindow(), look,
                          rc_prompt_text.left, rc_prompt_text.top,
                          rc_prompt_text.right - rc_prompt_text.left,
                          rc_prompt_text.bottom - rc_prompt_text.top,
                          style_frame);
  prompt_text->SetFont(Fonts::infobox_small);
  StaticString<255> prompt;
  prompt.Format(_T("%s: %s:"), _("If you add"), wp.name.c_str(),
                _("Estimated task is"));
  prompt_text->SetCaption(prompt.c_str());

  labels = new WndFrame(GetClientAreaWindow(), look,
                          rc_labels.left, rc_labels.top,
                          rc_labels.right - rc_labels.left,
                          rc_labels.bottom - rc_labels.top,
                          style_frame);
  labels->SetFont(Fonts::infobox_small);

  values = new WndFrame(GetClientAreaWindow(), look,
                          rc_values.left, rc_values.top,
                          rc_values.right - rc_values.left,
                          rc_values.bottom - rc_values.top,
                          style_frame);
  values->SetFont(Fonts::infobox_small);


  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  ok = new WndButton(GetClientAreaWindow(), dialog_look, _T("Add point"),
                           rc_ok,
                           button_style, this, AddPointClick);

  more = new WndButton(GetClientAreaWindow(), dialog_look, _T("More"),
                        rc_more,
                        button_style, this, MoreClick);

  cancel = new WndButton(GetClientAreaWindow(), dialog_look, _T("Cancel"),
                        rc_cancel,
                        button_style, this, CancelClick);


  RefreshForm();
}

void
MatClickPanel::Unprepare()
{
  Timer::Cancel();
  delete prompt_text;
  delete labels;
  delete values;
  delete ok;
  delete cancel;
  delete more;
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
  if (!task_manager->GetOrderedTask().CheckTask())
    return nullptr;

  return task_manager->Clone(XCSoarInterface::GetComputerSettings().task);
}

/**
 * returns true if wp is currently the next tp in the task (not the finish)
 * @param wp. the waypoint to be tested
 */
static bool
CheckIfAlreadyNext(const Waypoint &wp_new)
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  const OrderedTask &task = task_manager->GetOrderedTask();

  unsigned i = task.GetActiveTaskPointIndex();
  if (i == task.TaskSize() - 1)
    return false;

  if (i == 0)
    i++;
  if (i >= task.TaskSize())
    return false;

  const OrderedTaskPoint &otp_current = task.GetTaskPoint(i);
  return wp_new == otp_current.GetWaypoint();
}

/**
 * commits the task to the flight computer
 * @param task. a valid ordered task
 */
static bool
CommitTask(const OrderedTask &task)
{
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  return task_manager->Commit(task, way_points);
}

bool dlgMatItemClickShowModal(const Waypoint &wp)
{
  if (!CheckTask())
    return true; // show "Other" options

  // ask to remove it if it's already the next TP
  if (CheckIfAlreadyNext(wp)) {
    StaticString<255> remove_prompt;
    remove_prompt.Format(_T("%s %s %s?"),_("Remove"), wp.name.c_str(),
                         _("from task"));
    if (ShowMessageBox(remove_prompt.c_str(),
                       _("Remove MAT point"),
                       MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
      if (MapTaskManager::RemoveFromTask(wp) == MapTaskManager::SUCCESS) {
        remove_prompt.Format(_T("%s %s %s?"),_("Removed"), wp.name.c_str(),
                             _("from task"));
        ShowMessageBox(remove_prompt.c_str(), _("Success"), MB_OK);
      } else {
        remove_prompt.Format(_T("%s %s %s?"),_("Removed"), wp.name.c_str(),
                             _("from task"));
        ShowMessageBox(remove_prompt.c_str(), _("Error"), MB_OK | MB_ICONERROR);

      }
    }
    return false;
  }

  OrderedTask *task_new = CreateClone();
  OrderedTask *task_old = CreateClone();
  if (task_new == nullptr || task_old == nullptr)
    return false;

  if (MapTaskManager::InsertInMatProForma(*task_new, wp) !=
      MapTaskManager::SUCCESS) {
    ShowMessageBox(_("Could not insert waypoint into task"), _("Error"), MB_OK | MB_ICONERROR);
    delete task_new;
    delete task_old;
    return true;
  }

  if (!CommitTask(*task_new)) {
    ShowMessageBox(_("Failed to commit new task"), _("Error"), MB_OK | MB_ICONERROR);
    delete task_new;
    delete task_old;
    return false;
  }

  // add point to task
  ContainerWindow &w = UIGlobals::GetMainWindow();
  instance = new MatClickPanel(wp);
  instance->Initialise(w, instance->GetSize(w.GetClientRect()));
  instance->Prepare(w, instance->GetSize(w.GetClientRect()));

  if (instance->ShowModal() != mrOK) {
    if (!CommitTask(*task_old))
      ShowMessageBox(_("Failed to restore task"), _("Error"), MB_OK | MB_ICONERROR);
  }

  bool show_more_options = instance->GetShowMoreOptions();
  instance->Hide();
  instance->Unprepare();
  delete instance;
  delete task_new;
  delete task_old;

  return show_more_options;
}
