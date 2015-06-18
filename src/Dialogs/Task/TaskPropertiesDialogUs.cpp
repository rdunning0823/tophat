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

#include "Dialogs/Task/Manager/Internal.hpp"
#include "Screen/Layout.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Float.hpp"
#include "Dialogs/Task/dlgTaskHelpers.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/Edit.hpp"
#include "Form/Draw.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Task/TaskNationalities.hpp"
#include "Task/TypeStrings.hpp"
#include "Engine/Task/Factory/AbstractTaskFactory.hpp"
#include "Units/Units.hpp"
#include "Language/Language.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Engine/Task/TaskBehaviour.hpp"
#include "UIGlobals.hpp"
#include "Screen/SingleWindow.hpp"

enum Controls {
  TASK_TYPE,
  MIN_TIME,
  START_MAX_HEIGHT,
  START_HEIGHT_REF,
  FINISH_MIN_HEIGHT,
  FINISH_HEIGHT_REF,
};

/**
 * returns true if task is an FAI type
 * @param ftype. task type being checked
 */
static bool
IsFai(TaskFactoryType ftype)
{
  return (ftype == TaskFactoryType::FAI_GENERAL) ||
      (ftype == TaskFactoryType::FAI_GOAL) ||
      (ftype == TaskFactoryType::FAI_OR) ||
      (ftype == TaskFactoryType::FAI_TRIANGLE);
}

class TaskPropertiesPanelUs : public RowFormWidget,
                              private DataFieldListener {
  OrderedTask **ordered_task_pointer, *ordered_task;
  bool &task_changed;

  TaskFactoryType orig_taskType;

public:
  TaskPropertiesPanelUs(const DialogLook &look,
                        OrderedTask **_active_task, bool &_task_modified)
    :RowFormWidget(look),
     ordered_task_pointer(_active_task), ordered_task(*ordered_task_pointer),
     task_changed(_task_modified) {}

  void OnTaskTypeChange(DataFieldEnum &df);

  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Show(const PixelRect &rc);
  virtual void Hide();
  virtual bool Save(bool &changed);

protected:
  void RefreshView();
  /**
   * reads the values from the form into the ordered_task object
   */
  void ReadValues();

  /**
   * returns true if using US Task rules
   */
  bool IsUs();

private:
  /* virtual methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

static TaskPropertiesPanelUs *instance;

bool
TaskPropertiesPanelUs::IsUs()
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &tb = settings_computer.task;
  return tb.contest_nationality == ContestNationalities::AMERICAN;
}

void
TaskPropertiesPanelUs::RefreshView()
{
  const TaskFactoryType ftype = ordered_task->GetFactoryType();
  const OrderedTaskSettings &p = ordered_task->GetOrderedTaskSettings();

  bool aat_types = (ftype == TaskFactoryType::AAT || ftype == TaskFactoryType::MAT);
  bool fai_start_finish = p.finish_constraints.fai_finish;

  SetRowVisible(MIN_TIME, aat_types);
  LoadValueTime(MIN_TIME, (int)p.aat_min_time);

  SetRowVisible(START_MAX_HEIGHT, !fai_start_finish);
  LoadValue(START_MAX_HEIGHT, fixed(p.start_constraints.max_height), UnitGroup::ALTITUDE);

  if (!IsUs()) {
    SetRowVisible(START_HEIGHT_REF, !fai_start_finish);
    LoadValueEnum(START_HEIGHT_REF, p.start_constraints.max_height_ref);
  }

  SetRowVisible(FINISH_MIN_HEIGHT, !fai_start_finish);
  LoadValue(FINISH_MIN_HEIGHT, fixed(p.finish_constraints.min_height),
            UnitGroup::ALTITUDE);

  if (!IsUs()) {
    SetRowVisible(FINISH_HEIGHT_REF, !fai_start_finish);
    LoadValueEnum(FINISH_HEIGHT_REF, p.finish_constraints.min_height_ref);
  }

  LoadValueEnum(TASK_TYPE, ftype);
}

void
TaskPropertiesPanelUs::ReadValues()
{
  OrderedTaskSettings p = ordered_task->GetOrderedTaskSettings();

  TaskFactoryType newtype = ordered_task->GetFactoryType();
  task_changed |= SaveValueEnum(TASK_TYPE, newtype);

  int min_time = GetValueInteger(MIN_TIME);
  if (min_time != (int)p.aat_min_time) {
    p.aat_min_time = fixed(min_time);
    task_changed = true;
  }

  unsigned max_height =
    iround(Units::ToSysAltitude(GetValueFloat(START_MAX_HEIGHT)));
  if (max_height != p.start_constraints.max_height) {
    p.start_constraints.max_height = max_height;
    task_changed = true;
  }

  if (IsUs())
    p.start_constraints.max_height_ref = AltitudeReference::MSL;
  else {
    task_changed |= SaveValueEnum(START_HEIGHT_REF,
                                  p.start_constraints.max_height_ref);
  }

  unsigned min_height =
    iround(Units::ToSysAltitude(GetValueFloat(FINISH_MIN_HEIGHT)));
  if (min_height != p.finish_constraints.min_height) {
    p.finish_constraints.min_height = min_height;
    task_changed = true;
  }

  if (IsUs()) {
    p.finish_constraints.min_height_ref = AltitudeReference::MSL;
  } else {
    task_changed |= SaveValueEnum(FINISH_HEIGHT_REF,
                             p.finish_constraints.min_height_ref);
  }

  p.finish_constraints.fai_finish = IsFai(newtype);

  ordered_task->SetOrderedTaskSettings(p);
}

void
TaskPropertiesPanelUs::OnTaskTypeChange(DataFieldEnum &df)
{
  const TaskFactoryType newtype =
    (TaskFactoryType)df.GetAsInteger();
  if (newtype != ordered_task->GetFactoryType()) {
    ReadValues();
    ordered_task->SetFactory(newtype);
    task_changed = true;
    RefreshView();
  }
}

void
TaskPropertiesPanelUs::OnModified(DataField &df)
{
  if (IsDataField(TASK_TYPE, df))
    OnTaskTypeChange((DataFieldEnum &)df);
}

void
TaskPropertiesPanelUs::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  static constexpr StaticEnumChoice altitude_reference_list[] = {
    { (unsigned)AltitudeReference::AGL, N_("AGL"),
      N_("Reference is the height above the task point."), },
    { (unsigned)AltitudeReference::MSL, N_("MSL"),
      N_("Reference is altitude above mean sea level."), },
    { 0 }
  };

  DataFieldEnum *dfe = new DataFieldEnum(this);
  dfe->EnableItemHelp(true);
  const std::vector<TaskFactoryType> factory_types =
    ordered_task->GetFactoryTypes();
  for (unsigned i = 0; i < factory_types.size(); i++) {
    dfe->addEnumText(OrderedTaskFactoryName(factory_types[i]),
                     (unsigned)factory_types[i],
                     OrderedTaskFactoryDescription(factory_types[i]));
    if (factory_types[i] == ordered_task->GetFactoryType())
      dfe->Set((unsigned)factory_types[i]);
  }
  Add(_("Task type"), _("Sets the behavior for the current task."), dfe);

  AddTime(_("AAT min. time"), _("Minimum AAT task time in minutes."),
          0, 36000, 60, 180);

  StaticString<25> label;
  StaticString<100> help;
  if (IsUs()) {
    label = _("Start max. height (MSL)");
    help = _("Maximum height based on start height reference (MSL) while starting the task.  Set to 0 for no limit.");
  } else {
    label = _("Start max. height");
    help = _("Maximum height based on start height reference (AGL or MSL) while starting the task.  Set to 0 for no limit.");
  }
  AddFloat(label.c_str(), help.c_str(), _T("%.0f %s"), _T("%.0f"),
           fixed(0), fixed(10000), fixed(25), false, fixed(0));

  if (IsUs())
    RowFormWidget::AddDummy();
  else {
    AddEnum(_("Start height ref."),
            _("Reference used for start max height rule."),
            altitude_reference_list);
  }

  if (IsUs()) {
    label = _("Finish min. height (MSL)");
    help = _("Minimum height based on finish height reference (MSL) while finishing the task.  Set to 0 for no limit.");
  } else {
    label = _("Finish min. height");
    help = _("Minimum height based on finish height reference (AGL or MSL) while finishing the task.  Set to 0 for no limit.");
  }
  AddFloat(label.c_str(), help.c_str(), _T("%.0f %s"), _T("%.0f"),
           fixed(0), fixed(10000), fixed(25), false, fixed(0));

  if (IsUs())
    RowFormWidget::AddDummy();
  else {
    AddEnum(_("Finish height ref."),
            _("Reference used for finish min height rule."),
            altitude_reference_list);
  }
}

void
TaskPropertiesPanelUs::Show(const PixelRect &rc)
{
  ordered_task = *ordered_task_pointer;
  orig_taskType = ordered_task->GetFactoryType();

  RefreshView();

  RowFormWidget::Show(rc);
}

void
TaskPropertiesPanelUs::Hide()
{
  RowFormWidget::Hide();
}

bool
TaskPropertiesPanelUs::Save(bool &changed)
{
  ReadValues();
  if (orig_taskType != ordered_task->GetFactoryType())
    ordered_task->GetFactory().MutateTPsToTaskType();

  return true;
}

void
dlgTaskPropertiesUsShowModal(const DialogLook &look,
                             OrderedTask **_active_task, bool &_task_modified)
{

  instance = new TaskPropertiesPanelUs(look,
                                       _active_task, _task_modified);

  ButtonPanel::ButtonPanelPosition position = ButtonPanel::ButtonPanelPosition::Bottom;

  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(),_("Task rules"), instance,
                nullptr, 0, position);

  dialog.AddSymbolButton(_T("_X"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);
  dialog.ShowModal();
}



