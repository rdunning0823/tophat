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

#include "TaskStartMonitor.hpp"
#include "PageActions.hpp"
#include "Widget/QuestionWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Language/Language.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/TaskNationalities.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Engine/Task/Ordered/TaskAdvance.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Util/StaticString.hxx"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Time/LocalTime.hpp"
#include "Event/Timer.hpp"

class TaskStartWidget final
  : public QuestionWidget, private ActionListener, public Timer  {
  TaskStartMonitor &monitor;

  enum Action {
    DISMISS,
    ACCEPT,
  };

public:
  TaskStartWidget(TaskStartMonitor &_monitor, const TCHAR *_message,
                  unsigned rows_text)
    :QuestionWidget(_message, *this, rows_text),
     monitor(_monitor) {
    AddButton(_("Accept"), ACCEPT);
    AddButton(_("Ignore"), DISMISS);
  }

  ~TaskStartWidget() {
    Timer::Cancel();
    assert(monitor.widget == this);
    monitor.widget = nullptr;
  }

  virtual void Show(const PixelRect &rc) override {
    QuestionWidget::Show(rc);
  }

private:
  virtual void OnAction(int id) override;

public:
  /* virtual methods from Timer */
  virtual void OnTimer() override {
    OnAction(ACCEPT);
  }
};

void
TaskStartWidget::OnAction(int id)
{
  switch ((Action)id) {
  case DISMISS:
  {
    ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
    task_manager->SavedStartRestore();
  }
    break;

  case ACCEPT:
  {
    ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
    task_manager->SavedStartInvalidate();
  }
  }
  Timer::Cancel();
  PageActions::RestoreTop();
}

unsigned
TaskStartMonitor::GetMessage1(const StartStats &start, message_string &message)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;
  const auto &calculated = CommonInterface::Calculated();
  const CommonStats &common_stats = calculated.common_stats;


  const TCHAR task_start[] = N_("Task Start");
  unsigned rows_text = 2;
  TCHAR time_start[32];
  TCHAR altitude_start[32];
  FormatUserAltitude(start.altitude, altitude_start, true);
  FormatSignedTimeHHMM(time_start, TimeLocal((int)start.time,
                                             settings_computer.utc_offset));

  message.Format(_T("%s:   %s    %s"), task_start, altitude_start , time_start);
  if (task_behaviour.contest_nationality == ContestNationalities::AMERICAN) {
    TCHAR time_under[32];
    const int dd = (int)(start.time - common_stats.time_transition_below_max_start_height);
    FormatSignedTimeMMSS(time_under, dd);

    if (dd < 120)
      message.AppendFormat(_T("\n%s: %s"), _("Time under max height"), time_under);
    else
      message.AppendFormat(_T("\n%s"), _("Good"));
  } else {
    rows_text = 1;
  }
  return rows_text;
}

void
TaskStartMonitor::Check()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const CommonStats &common_stats = calculated.common_stats;
  const TaskStats &stats = calculated.ordered_task_stats;
  {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);
    if (!task_manager->SavedStartIsValid())
      return;
  }
  /* are we restarting the task? */
  if (common_stats.task_type != TaskType::ORDERED ||
      /* require a valid task */
      !stats.task_valid ||
      /* task must be started */
      !stats.start.task_started ||
      /* but not finished */
      stats.task_finished ||
      /* we don't have a newer start */
      stats.start.time <= last_start_time ||
      /** not navigating to start or first tp */
      stats.active_index > 1 ||
      /* valid GPS fix required to calculate nearest turn point */
      !basic.location_available) {
    last_start_time = stats.start.time;
    return;
  }

  // what happens if we showed the dialog, and then user swiped screens to show bottom widget, killing dialog?
  message_string message;
  GetMessage1(stats.start, message);
  if (widget == nullptr) {
    widget = new TaskStartWidget(*this, message.c_str(), 2);
    PageActions::SetCustomTop(widget);
  } else
    widget->UpdateMessage(message.c_str());

  widget->Schedule(30000);
  last_start_time = stats.start.time;
}
