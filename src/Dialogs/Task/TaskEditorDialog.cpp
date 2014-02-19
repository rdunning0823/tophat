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

#include "Dialogs/Task/TaskEditorDialog.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Form/Form.hpp"
#include "Form/Util.hpp"
#include "Form/Edit.hpp"
#include "Form/Frame.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Form/List.hpp"
#include "Form/Panel.hpp"
#include "Form/DataField/Float.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Font.hpp"
#include "Screen/Canvas.hpp"
#include "Components.hpp"
#include "Dialogs/Task/dlgTaskHelpers.hpp"
#include "Units/Units.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/StartPoint.hpp"
#include "Engine/Task/Ordered/Points/FinishPoint.hpp"
#include "Engine/Task/Ordered/Points/ASTPoint.hpp"
#include "Engine/Task/Ordered/Points/AATPoint.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/AnnularSectorZone.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "Task/Factory/LegalPointSet.hpp"
#include "Task/TypeStrings.hpp"
#include "Task/ValidationErrorStrings.hpp"
#include "Compiler.h"
#include "UIGlobals.hpp"
#include "Look/MapLook.hpp"
#include "Look/DialogLook.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Widget/DockWindow.hpp"
#include "Widget/PanelWidget.hpp"
#include "Widgets/ObservationZoneSummaryWidget.hpp"
#include "Widgets/SectorZoneEditWidget.hpp"
#include "Widgets/CylinderZoneEditWidget.hpp"
#include "Widgets/LineSectorZoneEditWidget.hpp"
#include "Widgets/KeyholeZoneEditWidget.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#endif

#include <assert.h>
#include <stdio.h>


static WndForm *wf = nullptr;
static DockWindow *dock;
static ObservationZoneEditWidget *properties_widget;
static ObservationZoneSummaryWidget *properties_summary_widget;
ListControl *wTaskPoints;
static OrderedTask* ordered_task = nullptr;
static OrderedTask** ordered_task_pointer = nullptr;
static bool task_modified = false;
static unsigned active_index = 0;
/**
 * tells calling program if modified, not modified or should be reverted
 */
TaskEditorReturn task_editor_return;

// setting to True during refresh so control values don't trigger form save
static bool Refreshing = false;

class TaskPointUsDialog : public ListItemRenderer, public ListCursorHandler {
public:
  /* virtual methods from ListItemRenderer */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /* virtual methods from ListCursorHandler */
  virtual void OnCursorMoved(unsigned index) override;

};

class TPOZListenerUs : public ObservationZoneEditWidget::Listener {
public:
  /* virtual methods from class ObservationZoneEditWidget::Listener */
  virtual void OnModified(ObservationZoneEditWidget &widget) override;
};

static TPOZListenerUs listener;

static ObservationZoneEditWidget *
CreateObservationZoneEditWidget(ObservationZonePoint &oz, bool is_fai_general)
{
  switch (oz.GetShape()) {
  case ObservationZone::Shape::SECTOR:
  case ObservationZone::Shape::ANNULAR_SECTOR:
  case ObservationZone::Shape::SYMMETRIC_QUADRANT:
    return new SectorZoneEditWidget((SectorZone &)oz); // 2 to 4 edits

  case ObservationZone::Shape::LINE:
    return new LineSectorZoneEditWidget((LineSectorZone &)oz, !is_fai_general); // 1 edit

  case ObservationZone::Shape::CYLINDER:
    return new CylinderZoneEditWidget((CylinderZone &)oz, !is_fai_general); // 1 edit

  case ObservationZone::Shape::MAT_CYLINDER:
    return new CylinderZoneEditWidget((CylinderZone &)oz, false); // 1 edit

  case ObservationZone::Shape::CUSTOM_KEYHOLE:
    return new KeyholeZoneEditWidget((KeyholeZone &)oz); //  3edits

  case ObservationZone::Shape::FAI_SECTOR:
  case ObservationZone::Shape::DAEC_KEYHOLE:
  case ObservationZone::Shape::BGAFIXEDCOURSE:
  case ObservationZone::Shape::BGAENHANCEDOPTION:
  case ObservationZone::Shape::BGA_START:
    break;
  }

  return nullptr;
}

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

/**
 * converts last point to finish if it is not already.
 * @return true if task is valid
 */
static bool
CheckAndFixTask()
{
  if (!task_modified)
    return true;

  ordered_task->ScanStartFinish();
  if (ordered_task->GetFactory().CheckAddFinish())
    ordered_task->ScanStartFinish();

  return (ordered_task->TaskSize() == 0) || ordered_task->CheckTask();
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  if (CheckAndFixTask())
    wf->SetModalResult(mrOK);
  else {

    ShowMessageBox(getTaskValidationErrors(
        ordered_task->GetFactory().GetValidationErrors()),
      _("Validation Errors"), MB_ICONEXCLAMATION | MB_OK);

    if (ShowMessageBox(_("Task not valid. Changes will be lost. Continue?"),
                        _("Task Manager"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
      task_editor_return = TaskEditorReturn::TASK_REVERT;
      wf->SetModalResult(mrOK);
    }
  }
}

static void
ShowDetails(bool visible)
{
  PanelControl *p = (PanelControl*)wf->FindByName(_T("frmDetails"));
  assert(p != nullptr);
  p->SetVisible(visible);

  WndButton *button_add = (WndButton*) wf->FindByName(_T("butAdd"));
  assert (button_add != nullptr);
  button_add->SetVisible(!ordered_task->IsFull());
  if (visible)
    button_add->SetCaption(_("Insert after"));
  else
    button_add->SetCaption(_("Insert"));
}

static void
RefreshTaskProperties()
{
  WndButton *button_properties = (WndButton*) wf->FindByName(_T("butTaskProperties"));
  assert (button_properties != nullptr);
  StaticString<255> line_1;
  StaticString<255> line_2;
  StaticString<255> both_lines;

  const OrderedTaskSettings &otb = ordered_task->GetOrderedTaskSettings();
  const TaskFactoryType ftype = ordered_task->GetFactoryType();
  line_1 = OrderedTaskFactoryName(ftype);

  if (ordered_task->GetTaskNameIsBlank())
    wf->SetCaption(line_1.c_str());
  else
    wf->SetCaption(ordered_task->GetTaskName());

  if (IsFai(ordered_task->GetFactoryType())) {
    button_properties->SetCaption(line_1.c_str());
    return;
  }

  if (ordered_task->HasTargets()) {
    StaticString<50> time_info;
    FormatSignedTimeHHMM(time_info.buffer(), (int)otb.aat_min_time);
    line_1.AppendFormat(_T(".  %s %s"), _("Time"), time_info.c_str());
  }
  StaticString<25> start_height;
  StaticString<25> finish_height;

  FormatUserAltitude(fixed(otb.start_constraints.max_height), start_height.buffer(), true);
  FormatUserAltitude(fixed(otb.finish_constraints.min_height), finish_height.buffer(), true);

  StaticString<50> text_start;
  StaticString<50> text_finish;
  StaticString<5> text_start_height_ref(N_("MSL"));
  StaticString<5> text_finish_height_ref(N_("MSL"));
  if (otb.start_constraints.max_height_ref == AltitudeReference::AGL)
    text_start_height_ref = N_("AGL");

  if (otb.finish_constraints.min_height_ref == AltitudeReference::AGL)
    text_finish_height_ref = N_("AGL");

  text_start.Format(_T("%s %s: %s"),
                    _("Start"), text_start_height_ref.c_str(), start_height.c_str());

  text_finish.Format(_T("%s %s: %s"),
                     _("Finish"), text_finish_height_ref.c_str(), finish_height.c_str());

  line_2.Format(_T("%s, %s"), text_start.c_str(), text_finish.c_str());
  both_lines.Format(_T("%s\n%s"), line_1.c_str(), line_2.c_str());
  button_properties->SetCaption(both_lines.c_str());
}

static void
RefreshView()
{
  wTaskPoints->SetLength(ordered_task->TaskSize());

  wTaskPoints->SetCursorIndex(active_index);

  RefreshTaskProperties();

  ShowDetails(ordered_task->TaskSize() != 0);
  if (ordered_task->TaskSize() == 0)
    return;

  OrderedTaskPoint &tp = ordered_task->GetPoint(active_index);

  Refreshing = true; // tell onChange routines not to save form!

  dock->SetWidget(new PanelWidget());

  ObservationZonePoint &oz = tp.GetObservationZone();
  const bool is_fai_general =
    ordered_task->GetFactoryType() == TaskFactoryType::FAI_GENERAL;
  properties_widget = CreateObservationZoneEditWidget(oz, is_fai_general);
  properties_summary_widget = nullptr;
  if (properties_widget != nullptr) {

    properties_widget->SetWaypointName(tp.GetWaypoint().name.c_str());
    properties_widget->SetListener(&listener);

    if (properties_widget->IsSummarized()) {
      properties_summary_widget = new
          ObservationZoneSummaryWidget(*properties_widget);
      dock->SetWidget(properties_summary_widget);
    } else {
      dock->SetWidget(properties_widget);
    }
  }

  WndButton *button_type = (WndButton*) wf->FindByName(_T("butType"));
  assert (button_type != nullptr);

  TrivialArray<TaskPointFactoryType, LegalPointSet::N> point_types;
  point_types.clear();
  ordered_task->GetFactory().GetValidTypes(active_index)
    .CopyTo(std::back_inserter(point_types));
  button_type->SetVisible(point_types.size() > 1u);

  button_type->SetCaption(OrderedTaskPointName(ordered_task->GetFactory().GetType(tp)));

  Refreshing = false; // reactivate onChange routines
}

static void
ReadValues()
{
  if (properties_widget != nullptr)
    properties_widget->Save(task_modified);
  else if (properties_summary_widget != nullptr)
    properties_summary_widget->Save(task_modified);
}

static void 
OnRemoveClicked(gcc_unused WndButton &Sender)
{
  unsigned result = ShowMessageBox(_("Remove task point?"), _("Task Point"),
                                   MB_YESNOALL | MB_ICONQUESTION);

  if (result != IDYES && result != IDALL)
    return;

  switch (result) {
  case IDYES:
    if (!ordered_task->GetFactory().Remove(active_index))
      return;
    ordered_task->UpdateGeometry();
    if (active_index >= ordered_task->TaskSize())
      active_index = std::max(1u, active_index) - 1;

    break;
  case IDALL:
    ordered_task->RemoveAllPoints();
    break;
  }

  task_modified = true;
  RefreshView();
}

static void
OnTaskPropertiesClicked(gcc_unused WndButton &Sender)
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  dlgTaskPropertiesUsShowModal(look, &ordered_task, task_modified);
  RefreshView();
}
/**
 * shows the task browse dialog, and updates the task as needed
 */
static void
OnBrowseClicked(gcc_unused WndButton &Sender)
{
  dlgTaskListUsShowModal(ordered_task_pointer, task_modified);
  ordered_task = *ordered_task_pointer;
  RefreshView();
}

/**
 * appends or inserts a task point after the current item
 */
static void
OnAddClicked(gcc_unused WndButton &Sender)
{
  assert(!ordered_task->IsFull());

  OrderedTaskPoint* point = nullptr;
  AbstractTaskFactory &factory = ordered_task->GetFactory();
  const Waypoint* way_point =
    ShowWaypointListDialog(ordered_task->TaskSize() > 0
                           ? ordered_task->GetPoint(ordered_task->TaskSize() - 1).GetLocation()
                           : CommonInterface::Basic().location,
                             ordered_task, active_index);
  if (!way_point)
    return;

  if (ordered_task->TaskSize() == 0) {
    point = (OrderedTaskPoint*)factory.CreateStart(*way_point);
    active_index = 0;
  } else {
    point = (OrderedTaskPoint*)factory.CreateIntermediate(*way_point);
   }
  if (point == nullptr)
    return;

  // insert after current index
  if (factory.Insert(*point, active_index + 1, true)) {
    task_modified = true;
    if (ordered_task->TaskSize() > 1)
      active_index++;
    ordered_task->UpdateGeometry();
  }

  delete point;
  RefreshView();
}

static void
OnRelocateClicked(gcc_unused WndButton &Sender)
{
  const GeoPoint &gpBearing = active_index > 0
    ? ordered_task->GetPoint(active_index - 1).GetLocation()
    : CommonInterface::Basic().location;

  const Waypoint *wp = ShowWaypointListDialog(gpBearing,
                                              ordered_task, active_index);
  if (wp == nullptr)
    return;

  ordered_task->GetFactory().Relocate(active_index, *wp);
  ordered_task->UpdateGeometry();
  task_modified = true;
  RefreshView();
}

static void
OnTypeClicked(gcc_unused WndButton &Sender)
{
  if (dlgTaskPointType(&ordered_task, active_index)) {
    task_modified = true;
    RefreshView();
    ordered_task->ScanStartFinish();
  }
}

static constexpr CallBackTableEntry CallBackTable[] = {

  DeclareCallBackEntry(OnTaskPropertiesClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnRemoveClicked),
  DeclareCallBackEntry(OnBrowseClicked),
  DeclareCallBackEntry(OnAddClicked),
  DeclareCallBackEntry(OnRelocateClicked),
  DeclareCallBackEntry(OnTypeClicked),
  DeclareCallBackEntry(nullptr)
};

void
TPOZListenerUs::OnModified(ObservationZoneEditWidget &widget)
{
  ReadValues();
}

void
TaskPointUsDialog::OnCursorMoved(unsigned i)
{
  if (i == ordered_task->TaskSize())
    return;

  active_index = i;
  RefreshView();
/*
 * TODO
 *   RefreshPropertyEditCaptions(i);*/
}

void
TaskPointUsDialog::OnPaintItem(Canvas &canvas, const PixelRect rc, unsigned DrawListIndex)
{
  assert(DrawListIndex < ordered_task->TaskSize());

  TCHAR buffer[120];

  const DialogLook &look = UIGlobals::GetDialogLook();

  const Font &name_font = *look.button.font;
  const Font &text_font = *look.text_font;

  const OrderedTaskPoint &tp = ordered_task->GetTaskPoint(DrawListIndex);

  // Y-Coordinate of the second row
  PixelScalar top2 = rc.top + name_font.GetHeight() + Layout::FastScale(4);

  // Use small font for details
  canvas.Select(text_font);

  // Draw details line
  PixelScalar left = rc.left + Layout::Scale(2);
  OrderedTaskPointRadiusLabel(tp.GetObservationZone(), buffer);
  if (!StringIsEmpty(buffer))
    canvas.DrawClippedText(left, top2, rc.right - left, buffer);

  // Draw turnpoint name
  canvas.Select(name_font);
  OrderedTaskPointLabel(tp.GetType(), tp.GetWaypoint().name.c_str(),
                        DrawListIndex, buffer);
  canvas.DrawClippedText(left, rc.top + Layout::FastScale(2),
                      rc.right - left, buffer);
}

TaskEditorReturn
dlgTaskEditorShowModal(OrderedTask** task_pointer,
                      const unsigned index)
{
  ordered_task = *task_pointer;
  ordered_task_pointer = task_pointer;
  task_modified = false;
  task_editor_return = TaskEditorReturn::TASK_NOT_MODIFIED;
  active_index = index;

  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ? _T("IDR_XML_TASKEDITOR_L") :
                                      _T("IDR_XML_TASKEDITOR"));
  assert(wf != nullptr);

  dock = (DockWindow *)wf->FindByName(_T("properties"));
  assert(dock != nullptr);

  const DialogLook &look = UIGlobals::GetDialogLook();

  TaskPointUsDialog dialog2;

  wTaskPoints = (ListControl*)wf->FindByName(_T("List"));
  assert(wTaskPoints != nullptr);

  wTaskPoints->SetItemRenderer(&dialog2);
  wTaskPoints->SetCursorHandler(&dialog2);
  UPixelScalar line_height = look.list.font->GetHeight()
    + Layout::Scale(6) + look.text_font->GetHeight();
  wTaskPoints->SetItemHeight(line_height);

  RefreshView();

  wf->ShowModal();

  delete wf;

  if (*task_pointer != ordered_task) {
    *task_pointer = ordered_task;
    task_modified = true;
  } 
  if (task_modified) {
    ordered_task->UpdateGeometry();
  }

  if (task_editor_return == TaskEditorReturn::TASK_REVERT)
    return task_editor_return;
  else if (task_modified)
    return TaskEditorReturn::TASK_MODIFIED;

  return TaskEditorReturn::TASK_NOT_MODIFIED;
}
