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

#include "Dialogs/dlgTaskPointUs.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Form/Form.hpp"
#include "Form/Util.hpp"
#include "Form/Frame.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Form/List.hpp"
#include "Form/Panel.hpp"
#include "Form/DataField/Float.hpp"
#include "Screen/Layout.hpp"
#include "Components.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Units/Units.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/StartPoint.hpp"
#include "Engine/Task/Ordered/Points/FinishPoint.hpp"
#include "Engine/Task/Ordered/Points/ASTPoint.hpp"
#include "Engine/Task/Ordered/Points/AATPoint.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/AnnularSectorZone.hpp"
#include "Gauge/TaskView.hpp"
#include "Compiler.h"
#include "UIGlobals.hpp"
#include "Look/MapLook.hpp"
#include "Look/DialogLook.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#endif

#include <assert.h>
#include <stdio.h>

static WndForm *wf = nullptr;
static WndFrame* wTaskView = nullptr;
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

  task_modified |= ordered_task->GetFactory().CheckAddFinish();

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

    if (ShowMessageBox(_("Task not valid. Changes will be lost.\nContinue?"),
                        _("Task Manager"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
      task_editor_return = TaskEditorReturn::TASK_REVERT;
      wf->SetModalResult(mrOK);
    }
  }
}

/**
 * for FAI tasks, make the zone sizes disabled so the user can't alter them
 * @param enable
 */
static void
EnableSizeEdit(bool enable)
{
  SetFormControlEnabled(*wf, _T("prpOZLineLength"), enable);
  SetFormControlEnabled(*wf, _T("prpOZCylinderRadius"), enable);
}

/**
 * for AT and MAT tasks, the 1-mile radius is in the description, so hide
 */
static void
ShowSizeEdit(bool visible)
{
  ShowFormControl(*wf, _T("prpOZLineLength"), visible);
  ShowFormControl(*wf, _T("prpOZCylinderRadius"), visible);
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


  const OrderedTaskBehaviour &otb = ordered_task->GetOrderedTaskBehaviour();
  const TaskFactoryType ftype = ordered_task->GetFactoryType();
  line_1 = OrderedTaskFactoryName(ftype);

  wf->SetCaption(line_1.c_str());

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

  FormatUserAltitude(fixed(otb.start_max_height), start_height.buffer(), true);
  FormatUserAltitude(fixed(otb.finish_min_height), finish_height.buffer(), true);

  StaticString<50> text_start;
  StaticString<50> text_finish;
  text_start.Format(_T("%s %s %s"),
                    _("Start"), _("MSL:"), start_height.c_str());

  text_finish.Format(_T("%s %s %s"),
                     _("Finish"), _("MSL:"), finish_height.c_str());

  line_2.Format(_T("%s, %s"), text_start.c_str(), text_finish.c_str());
  both_lines.Format(_T("%s\n%s"), line_1.c_str(), line_2.c_str());
  button_properties->SetCaption(both_lines.c_str());
}

static void
RefreshView()
{
  wTaskView->Invalidate();

  wTaskPoints->SetLength(ordered_task->TaskSize());

  wTaskPoints->SetCursorIndex(active_index);

  RefreshTaskProperties();

  ShowDetails(ordered_task->TaskSize() != 0);
  if (ordered_task->TaskSize() == 0)
    return;

  ShowFormControl(*wf, _T("frmOZLine"), false);
  ShowFormControl(*wf, _T("frmOZSector"), false);
  ShowFormControl(*wf, _T("frmOZCylinder"), false);

  const OrderedTaskPoint &tp = ordered_task->GetPoint(active_index);

  Refreshing = true; // tell onChange routines not to save form!

  const ObservationZonePoint &oz = tp.GetObservationZone();

  switch (oz.shape) {
  case ObservationZonePoint::SECTOR:
  case ObservationZonePoint::ANNULAR_SECTOR:
    ShowFormControl(*wf, _T("frmOZSector"), true);

    LoadFormProperty(*wf, _T("prpOZSectorRadius"),
                     UnitGroup::DISTANCE, ((const SectorZone &)oz).GetRadius());
    LoadFormProperty(*wf, _T("prpOZSectorStartRadial"),
                     ((const SectorZone &)oz).GetStartRadial().Degrees());
    LoadFormProperty(*wf, _T("prpOZSectorFinishRadial"),
                     ((const SectorZone &)oz).GetEndRadial().Degrees());

    if (oz.shape == ObservationZonePoint::ANNULAR_SECTOR) {
      LoadFormProperty(*wf, _T("prpOZSectorInnerRadius"),
                       UnitGroup::DISTANCE, ((const AnnularSectorZone &)oz).GetInnerRadius());

      ShowFormControl(*wf, _T("prpOZSectorInnerRadius"), true);
    } else
      ShowFormControl(*wf, _T("prpOZSectorInnerRadius"), false);

    break;

  case ObservationZonePoint::LINE:
    ShowFormControl(*wf, _T("frmOZLine"), true);

    LoadFormProperty(*wf, _T("prpOZLineLength"), UnitGroup::DISTANCE,
                     ((const LineSectorZone &)oz).GetLength());
    break;

  case ObservationZonePoint::CYLINDER:
  case ObservationZonePoint::MAT_CYLINDER:
    ShowFormControl(*wf, _T("frmOZCylinder"), true);

    LoadFormProperty(*wf, _T("prpOZCylinderRadius"), UnitGroup::DISTANCE,
                     ((const CylinderZone &)oz).GetRadius());
    break;

  default:
    break;
  }

  WndButton *button_type = (WndButton*) wf->FindByName(_T("butType"));
  assert (button_type != nullptr);
  unsigned num_types = ordered_task->GetFactory().GetValidTypes(
      active_index).size();
  button_type->SetVisible(num_types > 1u);
  button_type->SetCaption(OrderedTaskPointName(ordered_task->GetFactory().GetType(tp)));

  WndFrame* wfrm = nullptr;
  wfrm = ((WndFrame*)wf->FindByName(_T("lblType")));
  assert (wfrm != nullptr);
  wfrm->SetCaption(OrderedTaskPointName(ordered_task->GetFactory().GetType(tp)));
  wfrm->SetVisible(!button_type->IsVisible());

  bool edit_disabled = (ordered_task->GetFactoryType() ==
      TaskFactoryType::FAI_GENERAL) ||
      (((ordered_task->GetFactoryType() == TaskFactoryType::MAT) ||
        (ordered_task->GetFactoryType() == TaskFactoryType::RACING)) &&
      (active_index != ordered_task->TaskSize() - 1 && active_index != 0));

  EnableSizeEdit(!edit_disabled);

  bool hidden = ((ordered_task->GetFactoryType() == TaskFactoryType::MAT) ||
      (ordered_task->GetFactoryType() == TaskFactoryType::RACING)) &&
      (active_index != ordered_task->TaskSize() - 1 && active_index != 0);

  ShowSizeEdit(!hidden);

  Refreshing = false; // reactivate onChange routines
}

static void
ReadValues()
{
  OrderedTaskPoint &tp = ordered_task->GetPoint(active_index);
  ObservationZonePoint &oz = tp.GetObservationZone();

  switch (oz.shape) {
  case ObservationZonePoint::ANNULAR_SECTOR: {
    fixed radius = Units::ToSysDistance(
        GetFormValueFixed(*wf, _T("prpOZSectorInnerRadius")));

    if (fabs(radius - ((AnnularSectorZone &)oz).GetInnerRadius()) > fixed(49)) {
      ((AnnularSectorZone &)oz).SetInnerRadius(radius);
      task_modified = true;
    }
  }
  case ObservationZonePoint::SECTOR: {
    fixed radius =
      Units::ToSysDistance(GetFormValueFixed(*wf, _T("prpOZSectorRadius")));

    if (fabs(radius - ((SectorZone &)oz).GetRadius()) > fixed(49)) {
      ((SectorZone &)oz).SetRadius(radius);
      task_modified = true;
    }

    fixed start_radial = GetFormValueFixed(*wf, _T("prpOZSectorStartRadial"));
    if (start_radial != ((SectorZone &)oz).GetStartRadial().Degrees()) {
      ((SectorZone &)oz).SetStartRadial(Angle::Degrees(start_radial));
      task_modified = true;
    }

    fixed finish_radial = GetFormValueFixed(*wf, _T("prpOZSectorFinishRadial"));
    if (finish_radial != ((SectorZone &)oz).GetEndRadial().Degrees()) {
      ((SectorZone &)oz).SetEndRadial(Angle::Degrees(finish_radial));
      task_modified = true;
    }
    break;
  }
  case ObservationZonePoint::LINE: {
    fixed line_length = Units::ToSysDistance(
        GetFormValueFixed(*wf, _T("prpOZLineLength")));

    if (fabs(line_length - ((LineSectorZone &)oz).GetLength()) > fixed(49)) {
      ((LineSectorZone &)oz).SetLength(line_length);
      task_modified = true;
    }
    break;
  }

  case ObservationZonePoint::MAT_CYLINDER:
  case ObservationZonePoint::CYLINDER: {
    fixed radius = Units::ToSysDistance(
        GetFormValueFixed(*wf, _T("prpOZCylinderRadius")));

    if (fabs(radius - ((CylinderZone &)oz).GetRadius()) > fixed(49)) {
      ((CylinderZone &)oz).SetRadius(radius);
      task_modified = true;
    }
    break;
  }

  default:
    break;
  }
}

static void
OnTaskPaint(WndOwnerDrawFrame *Sender, Canvas &canvas)
{
  if (active_index >= ordered_task->TaskSize())
    return;

  PixelRect rc = Sender->GetClientRect();

  const OrderedTaskPoint &tp = ordered_task->GetPoint(active_index);

#ifdef ENABLE_OPENGL
  /* enable clipping */
  GLCanvasScissor scissor(canvas);
#endif

  const MapLook &look = UIGlobals::GetMapLook();
  const NMEAInfo &basic = CommonInterface::Basic();
  PaintTaskPoint(canvas, rc, *ordered_task, tp,
                 basic.location_available, basic.location,
                 XCSoarInterface::GetMapSettings(),
                 look.task, look.airspace,
                 terrain, &airspace_database);
}

static void 
OnRemoveClicked(gcc_unused WndButton &Sender)
{
  if (ShowMessageBox(_("Remove task point?"), _("Task Point"),
                  MB_YESNO | MB_ICONQUESTION) != IDYES)
    return;

  if (!ordered_task->GetFactory().Remove(active_index))
    return;

  if (active_index >= ordered_task->TaskSize())
    active_index = max(1u, active_index) - 1;

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
    ShowWaypointListDialog(wf->GetMainWindow(),
                           ordered_task->TaskSize() > 0
                           ? ordered_task->GetPoint(ordered_task->TaskSize() - 1).GetLocation()
                           : XCSoarInterface::Basic().location,
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

  const Waypoint *wp = ShowWaypointListDialog(wf->GetMainWindow(), gpBearing,
                                         ordered_task, active_index);
  if (wp == nullptr)
    return;

  ordered_task->GetFactory().Relocate(active_index, *wp);
  task_modified = true;
  RefreshView();
}

static void
OnTypeClicked(gcc_unused WndButton &Sender)
{
  if (dlgTaskPointType(wf->GetMainWindow(), &ordered_task, active_index)) {
    task_modified = true;
    RefreshView();
  }
}

static void
OnOZData(gcc_unused DataField *Sender,
         gcc_unused DataField::DataAccessMode Mode)
{
  if (!Refreshing)
    ReadValues();
  wTaskView->Invalidate();
}

static constexpr CallBackTableEntry CallBackTable[] = {

  DeclareCallBackEntry(OnTaskPropertiesClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnRemoveClicked),
  DeclareCallBackEntry(OnAddClicked),
  DeclareCallBackEntry(OnRelocateClicked),
  DeclareCallBackEntry(OnTypeClicked),
  DeclareCallBackEntry(OnTaskPaint),
  DeclareCallBackEntry(OnOZData),
  DeclareCallBackEntry(OnBrowseClicked),
  DeclareCallBackEntry(NULL)
};


static void
OnListCursorChange(unsigned i)
{
  if (i == ordered_task->TaskSize())
    return;

  active_index = i;
  RefreshView();
}

static void
OnListPaint(Canvas &canvas, const PixelRect rc, unsigned DrawListIndex)
{
//  if (ordered_task->TaskSize() == 0)
//    return;

  assert(DrawListIndex < ordered_task->TaskSize());

  TCHAR buffer[120];

  const DialogLook &look = UIGlobals::GetDialogLook();

  const Font &name_font = *look.list.font;
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
    canvas.text_clipped(left, top2, rc.right - left, buffer);

  // Draw turnpoint name
  canvas.Select(name_font);
  OrderedTaskPointLabel(tp.GetType(), tp.GetWaypoint().name.c_str(),
                        DrawListIndex, buffer);
  canvas.text_clipped(left, rc.top + Layout::FastScale(2),
                      rc.right - left, buffer);
}

TaskEditorReturn
dlgTaskPointUsShowModal(SingleWindow &parent, OrderedTask** task_pointer,
                      const unsigned index)
{
  ordered_task_pointer = task_pointer;
  ordered_task = *task_pointer;
  task_modified = false;
  task_editor_return = TaskEditorReturn::TASK_NOT_MODIFIED;
  active_index = index;

  wf = LoadDialog(CallBackTable, parent,
                  Layout::landscape ? _T("IDR_XML_TASKPOINT_US_L") :
                                      _T("IDR_XML_TASKPOINT_US"));
  assert(wf != nullptr);

  wTaskView = (WndFrame*)wf->FindByName(_T("frmTaskView"));
  assert(wTaskView != nullptr);

  const DialogLook &look = UIGlobals::GetDialogLook();

  WndFrame* wType = (WndFrame*) wf->FindByName(_T("lblType"));
  assert (wType);
  wType->SetFont(*look.caption.font);
  wType->SetAlignCenter();
  wType->SetVAlignCenter();

  wTaskPoints = (ListControl*)wf->FindByName(_T("List"));
  assert(wTaskPoints != nullptr);
  wTaskPoints->SetPaintItemCallback(OnListPaint);
  wTaskPoints->SetCursorCallback(OnListCursorChange);
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
