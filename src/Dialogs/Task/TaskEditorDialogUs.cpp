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

#include "Dialogs/Task/TaskEditorDialogUs.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/Message.hpp"
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
#include "Screen/SingleWindow.hpp"
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
#include "Util/StaticString.hxx"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Widget/DockWindow.hpp"
#include "Widget/PanelWidget.hpp"
#include "Widgets/ObservationZoneSummaryWidget.hpp"
#include "Widgets/SectorZoneEditWidget.hpp"
#include "Widgets/CylinderZoneEditWidget.hpp"
#include "Widgets/LineSectorZoneEditWidget.hpp"
#include "Widgets/KeyholeZoneEditWidget.hpp"
#include "Dialogs/WidgetDialog.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#endif

#include <assert.h>
#include <stdio.h>

enum Actions {
  TaskPropertiesClick = 100,
  BrowseClick,
  AddClick,
  RelocateClick,
  RemoveClick,
  ZoneTypeClick,
  CloseClick,
};

class TaskPointUsDialog : public NullWidget, public ActionListener,
  public ListItemRenderer, public ListCursorHandler,
  public ObservationZoneEditWidget::Listener {
public:

  WidgetDialog &dialog;
  // setting to True during refresh so control values don't trigger form save
  bool refreshing;
  DockWindow dock;

  OrderedTask* ordered_task;
  OrderedTask** ordered_task_pointer;
  bool task_modified;
  unsigned active_index;
  /**
   * tells calling program if modified, not modified or should be reverted
   */
  TaskEditorReturn task_editor_return;

  /**
   *  one of these two widgets is displayed in the dock
   */
  ObservationZoneEditWidget *zone_edit_widget;
  ObservationZoneSummaryWidget *zone_summary_widget;

  ListControl waypoint_list;
  Button *close_button;
  Button *zone_type_button;
  Button *add_button;
  Button *task_properties_button;
  Button *browse_button;
  Button *relocate_button;
  Button *remove_button;

  PixelRect rc_task_properties_button;
  PixelRect rc_close_button;
  PixelRect rc_zone_type_button;
  PixelRect rc_add_button;
  PixelRect rc_browse_button;
  PixelRect rc_relocate_button;
  PixelRect rc_remove_button;
  PixelRect rc_type_button;
  PixelRect rc_dock;
  PixelRect rc_waypoint_list;

  TaskPointUsDialog(WidgetDialog &_dialog, OrderedTask** _task_pointer,
                    const unsigned _index)
  :dialog(_dialog), refreshing(false),
   ordered_task(*_task_pointer), ordered_task_pointer(_task_pointer),
   task_modified(false), active_index(_index),
   task_editor_return(TaskEditorReturn::TASK_NOT_MODIFIED),
   waypoint_list(UIGlobals::GetDialogLook()) {}

  /* virtual methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  void Unprepare() override;
  void Show(const PixelRect &rc) override;
  void Hide() override;
  void Move(const PixelRect &rc) override {
    SetRectangles(rc);
    MoveChildren(rc);
  }
  /* virtual methods from ListItemRenderer */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /* virtual method from ObservationZoneEditWidget */
  void OnModified(ObservationZoneEditWidget &widget) override;

  /* virtual methods from ListCursorHandler */
  virtual void OnCursorMoved(unsigned index) override;
  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }
  virtual void OnActivateItem(unsigned index) override;

  /* from ActionListener */
  virtual void OnAction(int id) override;

  void OnCloseClicked();
  void OnRemoveClicked();
  void OnTaskPropertiesClicked();
  /* shows the task browse dialog, and updates the task as needed */
  void OnBrowseClicked();
  /* appends or inserts a task point after the current item */
  void OnAddClicked();
  void OnRelocateClicked();
  void OnTypeClicked();

  /**
   * updates the turnpoing (no start or finish) defaults in the
   * task behavior of the task being build
   */
  void UpdateTurnpointDefaults();
  void CreateButtons() {
    /* tab order works for portrait. */
    /* landscape is problem b/c widget and list are created elsewhere */
    close_button = dialog.AddSymbolButton(_T("_X"), *this, CloseClick);
    task_properties_button = dialog.AddButton(_T("*"), *this, TaskPropertiesClick);
    browse_button = dialog.AddButton(_("Browse"), *this, BrowseClick);
    add_button = dialog.AddButton(_("Insert before"), *this, AddClick);
    relocate_button = dialog.AddButton(_("Replace"), *this, RelocateClick);
    remove_button = dialog.AddButton(_("Delete"), *this, RemoveClick);
    zone_type_button = dialog.AddButton(_T("*"), *this, ZoneTypeClick);

    close_button->SetFocus();
  }

  /** returns true if prior widget existed and had to be deleted */
  bool ReplaceDockWidget(Widget* widget);

  /**
   * converts last point to finish if it is not already.
   * @return true if task is valid
   */
  bool CheckAndFixTask();
  /* shows fields if task exists, else hides them */
  void ShowDetails(bool visible);
  void RefreshTaskProperties();
  void RefreshView();
  void ReadValues();
  void SetRectangles(const PixelRect rc_outer);

  /**
   *  creates a new oz edit widget of appropriate type.
   *  @return widget or nullptr if none is available
   **/
  ObservationZoneEditWidget *
  CreateObservationZoneEditWidget(ObservationZonePoint &oz,
                                  bool is_fai_general);
  /**
   * inserts or appends to task
   * @param insert_index the index of the new point
   * @param reference_location the reference point to pass to the waypoint list
   */
  void AddInsertTurnpoint(const unsigned insert_index,
                          const GeoPoint &reference_location);

  /**
   * appends tp to end of task
   */
  void AppendTurnpoint();

  /**
   * inserts turnpoint prior to index of selected item
   * active_index can be 0 to TaskSize() (i.e. append)
   */
  void InsertTurnpoint(const unsigned active_index);

  OrderedTask* GetOrderedTask() {
    return ordered_task;
  }
  bool IsModified() {
    return task_modified;
  }

  /* returns the revert, not modified or save state of the task */
  TaskEditorReturn GetReturnMode() {
    return task_editor_return;
  }
private:
  void MoveChildren(const PixelRect &rc) {
    task_properties_button->Move(rc_task_properties_button);
    close_button->Move(rc_close_button);
    zone_type_button->Move(rc_zone_type_button);
    add_button->Move(rc_add_button);
    browse_button->Move(rc_browse_button);
    relocate_button->Move(rc_relocate_button);
    remove_button->Move(rc_remove_button);
    dock.MoveAndShow(rc_dock);
    waypoint_list.Move(rc_waypoint_list);
  }
};

void
TaskPointUsDialog::Hide()
{
}

void
TaskPointUsDialog::Show(const PixelRect &rc)
{
  MoveChildren(rc);
};

ObservationZoneEditWidget *
TaskPointUsDialog::CreateObservationZoneEditWidget(ObservationZonePoint &oz,
                                                   bool is_fai_general)
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
IsFaiFactory(TaskFactoryType ftype)
{
  return (ftype == TaskFactoryType::FAI_GENERAL) ||
      (ftype == TaskFactoryType::FAI_GOAL) ||
      (ftype == TaskFactoryType::FAI_OR) ||
      (ftype == TaskFactoryType::FAI_TRIANGLE);
}

bool
TaskPointUsDialog::CheckAndFixTask()
{
  if (!task_modified)
    return true;

  ordered_task->ScanStartFinish();
  if (ordered_task->GetFactory().CheckAddFinish())
    ordered_task->ScanStartFinish();

  return (ordered_task->TaskSize() == 0) || ordered_task->CheckTask();
}

void
TaskPointUsDialog::OnAction(int id)
{
  switch (id) {
  case TaskPropertiesClick:
    OnTaskPropertiesClicked();
    break;
  case BrowseClick:
    OnBrowseClicked();
    break;
  case AddClick:
    OnAddClicked();
    break;
  case RelocateClick:
    OnRelocateClicked();
    break;
  case RemoveClick:
    OnRemoveClicked();
    break;
  case ZoneTypeClick:
    OnTypeClicked();
    break;
  case CloseClick:
    OnCloseClicked();
    break;
  }
}

void
TaskPointUsDialog::OnCloseClicked()
{
  if (CheckAndFixTask())
    dialog.SetModalResult(mrOK);
  else {

    ShowMessageBox(getTaskValidationErrors(
        ordered_task->GetFactory().GetValidationErrors()),
      _("Validation Errors"), MB_ICONEXCLAMATION | MB_OK);

    if (ShowMessageBox(_("Task not valid. Changes will be lost. Continue?"),
                        _("Task Manager"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
      task_editor_return = TaskEditorReturn::TASK_REVERT;
      dialog.SetModalResult(mrOK);
    }
  }
}

void
TaskPointUsDialog::ShowDetails(bool visible)
{
  relocate_button->SetVisible(visible);
  remove_button->SetVisible(visible);
  zone_type_button->SetVisible(visible);

  add_button->SetVisible(!ordered_task->IsFull());
  if (visible)
    add_button->SetCaption(_("Insert before"));
  else
    add_button->SetCaption(_("Insert"));

  relocate_button->SetVisible(active_index != ordered_task->TaskSize());
  remove_button->SetVisible(active_index != ordered_task->TaskSize());
}

void
TaskPointUsDialog::RefreshTaskProperties()
{
  StaticString<255> line_1;
  StaticString<255> line_2;
  StaticString<255> both_lines;

  const OrderedTaskSettings &otb = ordered_task->GetOrderedTaskSettings();
  const TaskFactoryType ftype = ordered_task->GetFactoryType();
  line_1 = OrderedTaskFactoryName(ftype);

  if (ordered_task->GetNameIsBlank())
    dialog.SetCaption(line_1.c_str());
  else
    dialog.SetCaption(ordered_task->GetName());

  if (IsFaiFactory(ordered_task->GetFactoryType())) {
    task_properties_button->SetCaption(line_1.c_str());
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
  task_properties_button->SetCaption(both_lines.c_str());
}

bool
TaskPointUsDialog::ReplaceDockWidget(Widget* widget)
{
  bool prior_exists = (dock.GetWidget() != nullptr);
  if (prior_exists)
    dock.DeleteWidget();
  dock.SetWidget(widget);
  return prior_exists;
}

void
TaskPointUsDialog::RefreshView()
{
  waypoint_list.SetLength(ordered_task->TaskSize() + 1);
  waypoint_list.SetCursorIndex(active_index);

  RefreshTaskProperties();
  ShowDetails(ordered_task->TaskSize() != 0);

  if (ordered_task->TaskSize() == 0 ||
      active_index == ordered_task->TaskSize()) {
    zone_type_button->SetVisible(false);
    dock.SetVisible(false);
    return;
  }
  zone_type_button->SetVisible(true);
  dock.SetVisible(true);

  if (active_index == ordered_task->TaskSize())
    return;

  OrderedTaskPoint &tp = ordered_task->GetPoint(active_index);
  refreshing = true; // tell onChange routines not to save form!

  ObservationZonePoint &oz = tp.GetObservationZone();
  const bool is_fai_general =
    ordered_task->GetFactoryType() == TaskFactoryType::FAI_GENERAL;
  zone_edit_widget = CreateObservationZoneEditWidget(oz, is_fai_general);
  zone_summary_widget = nullptr;

  if (zone_edit_widget != nullptr) {

    zone_edit_widget->SetWaypointName(tp.GetWaypoint().name.c_str());
    zone_edit_widget->SetListener(this);

    if (zone_edit_widget->IsSummarized()) {
      zone_summary_widget = new
          ObservationZoneSummaryWidget(*zone_edit_widget);

      ReplaceDockWidget(zone_summary_widget);
    } else {
      ReplaceDockWidget(zone_edit_widget);
    }
  } else
    ReplaceDockWidget(new PanelWidget());

  TrivialArray<TaskPointFactoryType, LegalPointSet::N> point_types;
  point_types.clear();
  ordered_task->GetFactory().GetValidTypes(active_index)
    .CopyTo(std::back_inserter(point_types));
  zone_type_button->SetVisible(point_types.size() > 1u);

  zone_type_button->SetCaption(OrderedTaskPointName(ordered_task->GetFactory().GetType(tp)));
  refreshing = false; // reactivate onChange routines
}

void
TaskPointUsDialog::ReadValues()
{
  if (zone_edit_widget != nullptr)
    zone_edit_widget->Save(task_modified);
  else if (zone_summary_widget != nullptr)
    zone_summary_widget->Save(task_modified);
}

void
TaskPointUsDialog::OnRemoveClicked()
{
  assert(active_index < ordered_task->TaskSize());

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
    unsigned result = ShowMessageBox(_("Clear all points?"), _("Confirm"),
                                     MB_OKCANCEL | MB_ICONQUESTION);

    if (result != IDOK)
      return;
    ordered_task->RemoveAllPoints();
    active_index = 0;
    break;
  }

  task_modified = true;
  RefreshView();
}

void
TaskPointUsDialog::OnTaskPropertiesClicked()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  dlgTaskPropertiesUsShowModal(look, &ordered_task, task_modified);
  RefreshView();
}

void
TaskPointUsDialog::OnBrowseClicked()
{
  dlgTaskListUsShowModal(ordered_task_pointer, task_modified);
  ordered_task = *ordered_task_pointer;
  RefreshView();
}

void
TaskPointUsDialog::AddInsertTurnpoint(const unsigned insert_index,
                   const GeoPoint &reference_location)
{
  assert(insert_index <= ordered_task->TaskSize());
  assert(!ordered_task->IsFull());

  const unsigned task_size = ordered_task->TaskSize();

  OrderedTaskPoint* point = nullptr;
  AbstractTaskFactory &factory = ordered_task->GetFactory();

  const Waypoint* way_point =
    ShowWaypointListDialog(reference_location,
                           ordered_task, insert_index);
  if (!way_point)
    return;

  if (task_size == 0) {
    point = (OrderedTaskPoint*)factory.CreateStart(*way_point);
    active_index = 0;
  } else {
    point = (OrderedTaskPoint*)factory.CreateIntermediate(*way_point);
   }
  if (point == nullptr)
    return;

  // insert after current index
  if (factory.Insert(*point, insert_index, true)) {
    task_modified = true;
    if (insert_index + 1 == ordered_task->TaskSize())
      active_index = insert_index;
    ordered_task->UpdateGeometry();
  }

  delete point;
  RefreshView();
}

void
TaskPointUsDialog::AppendTurnpoint()
{
  unsigned task_size = ordered_task->TaskSize();
  assert(task_size > 0);

  AddInsertTurnpoint(task_size,
                     ordered_task->GetPoint(task_size - 1).GetLocation());
}

void
TaskPointUsDialog::InsertTurnpoint(const unsigned active_index)
{
  if (active_index == ordered_task->TaskSize() &&
      active_index > 0) {
    AppendTurnpoint();
    return;
  }

  const GeoPoint* point = nullptr;

  if (active_index == 0)
    point = &CommonInterface::Basic().location;
  else
    point = &ordered_task->GetPoint(active_index - 1).GetLocation();

  assert (point != nullptr);
  AddInsertTurnpoint(active_index, *point);
}

void
TaskPointUsDialog::OnAddClicked()
{
  assert(!ordered_task->IsFull());
  if (active_index == ordered_task->TaskSize() &&
      active_index > 0)
    AppendTurnpoint();
  else
    InsertTurnpoint(active_index);
}

void
TaskPointUsDialog::OnRelocateClicked()
{
  assert (active_index < ordered_task->TaskSize());

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

void
TaskPointUsDialog::OnTypeClicked()
{
  if (dlgTaskPointType(*ordered_task, active_index)) {
    task_modified = true;
    RefreshView();
    ordered_task->ScanStartFinish();
  }
}

void
TaskPointUsDialog::UpdateTurnpointDefaults()
{
  if (ordered_task->TaskSize() < 2)
    return;

  const AbstractTaskFactory& factory = ordered_task->GetFactory();

  switch (ordered_task->GetPoint(active_index).GetType()) {
  case TaskPointType::AST:
  case TaskPointType::AAT: {
    auto point_type = (TaskPointFactoryType)factory.GetType(
        ordered_task->GetPoint(active_index));
    ordered_task->GetTaskBehaviour().sector_defaults.turnpoint_type = (TaskPointFactoryType)point_type;

    fixed radius = factory.GetOZSize(
        ordered_task->GetPoint(active_index).GetObservationZone());
    ordered_task->GetTaskBehaviour().sector_defaults.turnpoint_radius = radius;
    break;
  }
  case TaskPointType::UNORDERED:
  case TaskPointType::START:
  case TaskPointType::FINISH:
    break;
  }
}

void
TaskPointUsDialog::OnModified(ObservationZoneEditWidget &widget)
{
  ReadValues();
  UpdateTurnpointDefaults();
}


void
TaskPointUsDialog::OnActivateItem(unsigned i)
{
  if (i == ordered_task->TaskSize()) {
    OnAddClicked();
    return;
  }
}

void
TaskPointUsDialog::OnCursorMoved(unsigned i)
{
  assert(i <= ordered_task->TaskSize());

  active_index = i;
  RefreshView();
/*
 * TODO
 *   RefreshPropertyEditCaptions(i);*/
}

void
TaskPointUsDialog::OnPaintItem(Canvas &canvas, const PixelRect rc, unsigned DrawListIndex)
{
  assert(DrawListIndex <= ordered_task->TaskSize());

  TCHAR buffer[120];

  const DialogLook &look = UIGlobals::GetDialogLook();

  const Font &name_font = *look.button.font;
  const Font &text_font = look.text_font;
  const unsigned padding = Layout::GetTextPadding();
  const PixelScalar line_height = rc.bottom - rc.top;

  // Draw "Add turnpoint" label
  if (DrawListIndex == ordered_task->TaskSize()) {
    canvas.Select(name_font);
    _stprintf(buffer, _T("  (%s)"), _("Append turnpoint"));
    canvas.DrawText(rc.left + padding,
                    rc.top + line_height / 2 - name_font.GetHeight() / 2,
                    buffer);
    return;
  }

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

void
TaskPointUsDialog::SetRectangles(const PixelRect rc)
{
  unsigned button_height = Layout::Scale(35);
  unsigned button_width = Layout::Scale(73);
  unsigned padding = Layout::Scale(2);

  if (Layout::landscape) {
    button_width = std::min(int(button_width), int(rc.GetSize().cx / 4 - padding));
    unsigned column_two_x = rc.GetSize().cx / 2;

    rc_task_properties_button.left = Layout::Scale(2);
    rc_task_properties_button.right = column_two_x + 2 * button_width;
    rc_task_properties_button.top = Layout::Scale(2);
    rc_task_properties_button.bottom = rc_task_properties_button.top
        + button_height;

    rc_zone_type_button.left = column_two_x;
    rc_zone_type_button.right = rc_zone_type_button.left + 2 * button_width;
    rc_zone_type_button.top = rc_task_properties_button.bottom + padding;
    rc_zone_type_button.bottom = rc_zone_type_button.top + button_height;

    rc_dock = rc_zone_type_button;
    rc_dock.top = rc_zone_type_button.bottom + padding;
    rc_dock.bottom = rc_dock.top + button_height + padding;

    rc_waypoint_list.left = 0;
    rc_waypoint_list.right = column_two_x - padding;
    rc_waypoint_list.bottom = rc.bottom - button_height;
    rc_waypoint_list.top = rc_zone_type_button.top;

    rc_close_button.left = 0;
    rc_close_button.right = rc_waypoint_list.right;
    rc_close_button.bottom = rc.bottom;
    rc_close_button.top = rc.bottom - button_height;

    rc_add_button.left = column_two_x;
    rc_add_button.right = rc_add_button.left + button_width;
    rc_add_button.top = rc_dock.bottom + padding;
    rc_add_button.bottom = rc_add_button.top + button_height;

    rc_relocate_button.left = column_two_x;
    rc_relocate_button.right = rc_relocate_button.left + button_width;
    rc_relocate_button.top = rc_add_button.bottom + padding;
    rc_relocate_button.bottom = rc_relocate_button.top + button_height;

    rc_remove_button.left = column_two_x;
    rc_remove_button.right = rc_remove_button.left + button_width;
    rc_remove_button.top = rc_relocate_button.bottom + padding;
    rc_remove_button.bottom = rc_remove_button.top + button_height;

    rc_browse_button = rc_remove_button;
    rc_browse_button.left = rc_remove_button.right + padding * 2;
    rc_browse_button.right = rc_browse_button.left + button_width;

  } else {
    rc_zone_type_button.left = Layout::Scale(2);
    rc_zone_type_button.right = Layout::Scale(148);
    rc_zone_type_button.top = Layout::Scale(38);
    rc_zone_type_button.bottom = rc_zone_type_button.top + button_height;;

    rc_dock = rc_zone_type_button;
    rc_dock.top = rc_zone_type_button.bottom + padding;
    rc_dock.bottom = rc_dock.top + button_height + padding;

    rc_waypoint_list.left = 0;
    rc_waypoint_list.right = Layout::Scale(150);
    rc_waypoint_list.bottom = rc.bottom - button_height;
    rc_waypoint_list.top = rc_dock.bottom + padding;

    rc_task_properties_button.left = Layout::Scale(2);
    rc_task_properties_button.right = rc_task_properties_button.left
        + Layout::Scale(236);
    rc_task_properties_button.top = Layout::Scale(2);
    rc_task_properties_button.bottom = rc_task_properties_button.top
        + button_height;

    rc_close_button.left = 0;
    rc_close_button.right = rc_waypoint_list.right;
    rc_close_button.bottom = rc.bottom;
    rc_close_button.top = rc.bottom - button_height;

    rc_browse_button.left = rc_task_properties_button.right - button_width;
    rc_browse_button.right = rc_task_properties_button.right;
    rc_browse_button.top = Layout::Scale(38);
    rc_browse_button.bottom = rc_browse_button.top + button_height;

    rc_add_button.left = rc_browse_button.left;
    rc_add_button.right = rc_browse_button.right;
    rc_add_button.top = rc_waypoint_list.top;
    rc_add_button.bottom = rc_add_button.top + button_height;

    rc_relocate_button.left = rc_browse_button.left;
    rc_relocate_button.right = rc_browse_button.right;
    rc_relocate_button.top = rc_add_button.bottom + padding;
    rc_relocate_button.bottom = rc_relocate_button.top + button_height;

    rc_remove_button.left = rc_browse_button.left;
    rc_remove_button.right = rc_browse_button.right;
    rc_remove_button.top = rc_relocate_button.bottom + padding;
    rc_remove_button.bottom = rc_remove_button.top + button_height;
  }
}

void
TaskPointUsDialog::Unprepare()
{
  dock.DeleteWidget();
  NullWidget::Unprepare();
}

void
TaskPointUsDialog::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  NullWidget::Prepare(parent, rc);
  SetRectangles(rc);
  const DialogLook &look = UIGlobals::GetDialogLook();

  WindowStyle style_dock;
  style_dock.ControlParent();

  dock.Create(parent, rc_dock, style_dock);

  WindowStyle style_list;
  style_list.TabStop();
  unsigned line_height = look.list.font->GetHeight()
        + Layout::Scale(6) + look.text_font.GetHeight();
  waypoint_list.Create(parent, rc_waypoint_list, style_list,
                       line_height);
  waypoint_list.SetItemRenderer(this);
  waypoint_list.SetCursorHandler(this);

  RefreshView();
}

TaskEditorReturn
dlgTaskEditorShowModal(OrderedTask** task_pointer,
                      const unsigned index)
{
  const DialogLook &look = UIGlobals::GetDialogLook();

  WidgetDialog dialog(look);
  TaskPointUsDialog widget(dialog, task_pointer, index);
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Waypoint"), &widget, nullptr, 0,
                    ButtonPanel::ButtonPanelPosition::Manual);

  widget.CreateButtons();
  dialog.ShowModal();

  bool task_modified = widget.IsModified();
  OrderedTask* ordered_task = widget.GetOrderedTask();
  if (*task_pointer != ordered_task) {
    *task_pointer = ordered_task;
    task_modified = true;
  } 
  if (task_modified) {
    ordered_task->UpdateGeometry();
  }

  TaskEditorReturn ret_val = TaskEditorReturn::TASK_MODIFIED;

  if (widget.GetReturnMode() == TaskEditorReturn::TASK_REVERT)
    ret_val = widget.GetReturnMode();
  else if (task_modified)
    ret_val =  TaskEditorReturn::TASK_MODIFIED;

  dialog.StealWidget();
  return ret_val;
}
