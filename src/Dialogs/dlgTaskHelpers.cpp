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

#include "dlgTaskHelpers.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "Units/Units.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/SectorZone.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/FAITriangleValidator.hpp"
#include "Components.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Interface.hpp"
#include "Engine/Task/TaskBehaviour.hpp"

#include <assert.h>
#include <stdio.h>
#include <windef.h> /* for MAX_PATH */

const TCHAR*
OrderedTaskFactoryName(TaskFactoryType type)
{
  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  TaskBehaviour &tb = settings_computer.task;
  bool is_usa = tb.contest_nationality == ContestNationalities::AMERICAN;
  switch(type) {
  case TaskFactoryType::RACING:
    if (is_usa)
      return _("AT - Assigned task");
    else
      return _("Racing");
  case TaskFactoryType::FAI_GENERAL:
    return _("FAI badges/records");
  case TaskFactoryType::FAI_TRIANGLE:
    return _("FAI triangle");
  case TaskFactoryType::FAI_OR:
    return _("FAI out and return");
  case TaskFactoryType::FAI_GOAL:
    return _("FAI goal");
  case TaskFactoryType::AAT:
    if (is_usa)
      return _("TAT - Turn area task");
    else
      return _("AAT");
  case TaskFactoryType::MAT:
    return _("MAT - Modified area task");
  case TaskFactoryType::MIXED:
    return _("Mixed");
  case TaskFactoryType::TOURING:
    return _("Touring");
  }

  assert(false);
  return NULL;
}

const TCHAR*
OrderedTaskFactoryDescription(TaskFactoryType type)
{
  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  TaskBehaviour &tb = settings_computer.task;
  bool is_usa = tb.contest_nationality == ContestNationalities::AMERICAN;

  switch(type) {
  case TaskFactoryType::RACING:
    if (is_usa)
      return _("Assigned task. Has 1 mile turn point cylinders.  No minimum time.");
    else
      return _("Racing task around turn points.  Can also be used for FAI badge and record tasks.  "
        "Allows all shapes of observation zones.");
  case TaskFactoryType::FAI_GENERAL:
    return _("FAI rules, allows only FAI start, finish and turn point types, for badges and "
        "records.  Enables FAI finish height for final glide calculation.");
  case TaskFactoryType::FAI_TRIANGLE:
    return _("FAI rules, path from a start to two turn points and return.");
  case TaskFactoryType::FAI_OR:
    return _("FAI rules, path from start to a single turn point and return.");
  case TaskFactoryType::FAI_GOAL:
    return _("FAI rules, path from start to a goal destination.");
  case TaskFactoryType::AAT:
    if (is_usa)
      return _("Turn area task.  Has cylinders of varying sizes.  Minimum task time applies.");
    else
      return _("Assigned area task.  Has cylinders of varying sizes.  Minimum task time applies.");
  case TaskFactoryType::MAT:
    return _("Modified area task.  Task with start, finish and at least one predefined 1-mile cylinder.  Pilot can add additional points as needed.  Minimum task time applies.");
  case TaskFactoryType::MIXED:
    return _("Racing task with a mix of assigned areas and turn points, minimum task time applies.");
  case TaskFactoryType::TOURING:
    return _("Casual touring task, uses start and finish cylinders and FAI sector turn points.");
  }

  assert(false);
  return NULL;
}

/**
 *
 * @param task
 * @param text
 * @return True if FAI shape
 */
static bool
TaskSummaryShape(OrderedTask* task, TCHAR* text)
{
  bool FAIShape = false;
  switch (task->TaskSize()) {
  case 0:
    text[0] = '\0';
    break;

  case 1:
    _tcscpy(text, _("Unknown"));
    break;

  case 2:
    _tcscpy(text, _("Goal"));
    FAIShape = true;

    break;

  case 3:
    if (task->GetFactory().IsClosed()) {
      _tcscpy(text, _("Out and return"));
      FAIShape = true;
    }
    else
      _tcscpy(text, _("Two legs"));
    break;

  case 4:
    if (!task->GetFactory().IsUnique() ||!task->GetFactory().IsClosed())
      _tcscpy(text, _("Three legs"));
    else if (FAITriangleValidator::Validate(*task)) {
      _tcscpy(text, _("FAI triangle"));
      FAIShape = true;
    }
    else
      _tcscpy(text, _("triangle"));
    break;

  default:
    _stprintf(text, _("%d legs"), task->TaskSize() - 1);
    break;
  }
  return FAIShape;
}
void
OrderedTaskSummary(OrderedTask* task, TCHAR* text, bool linebreaks)
{

  StaticString<120> gate_info;
  gate_info.clear();
  const OrderedTaskBehaviour &otb = task->GetOrderedTaskBehaviour();

  if (!otb.fai_finish) {
    StaticString<25> start_height;
    StaticString<25> finish_height;

    FormatUserAltitude(fixed(otb.start_max_height), start_height.buffer(), true);
    FormatUserAltitude(fixed(otb.finish_min_height), finish_height.buffer(), true);

    gate_info.Format(_T("\nStart Height: %s %s. Finish Height: %s %s. "),
                     start_height.c_str(),
                     (otb.start_max_height_ref == HeightReferenceType::AGL)
                       ? _("AGL") : _("MSL"),
                     finish_height.c_str(),
                     (otb.finish_min_height_ref == HeightReferenceType::AGL)
                       ? _("AGL") : _("MSL"));
  }

  if (task->HasTargets()) {
    StaticString<50> time_info;
    FormatSignedTimeHHMM(time_info.buffer(), (int)otb.aat_min_time);
    gate_info.append(_("Min Time: "));
    gate_info.append(time_info.c_str());
  }

  const TaskStats &stats = task->GetStats();
  TCHAR summary_shape[100];
  TaskSummaryShape(task, summary_shape);

  TCHAR linebreak[3];
  if (linebreaks) {
    linebreak[0] = '\n';
    linebreak[1] = 0;
  } else {
    linebreak[0] = ',';
    linebreak[1] = ' ';
    linebreak[2] = 0;
  }

  if (!task->TaskSize()) {
    _stprintf(text, _("Task is empty (%s)"),
             OrderedTaskFactoryName(task->GetFactoryType()));
  } else {
    if (task->HasTargets())
      _stprintf(text, _T("%s. %s%s%.0f %s%s%s %.0f %s%s%s %.0f %s %s"),
                OrderedTaskFactoryName(task->GetFactoryType()),
                summary_shape,
                linebreak,
                (double)Units::ToUserDistance(stats.distance_nominal),
                Units::GetDistanceName(),
                linebreak,
                _("max."),
                (double)Units::ToUserDistance(stats.distance_max),
                Units::GetDistanceName(),
                linebreak,
                _("min."),
                (double)Units::ToUserDistance(stats.distance_min),
                Units::GetDistanceName(),
                gate_info.c_str());
    else
      _stprintf(text, _T("%s. %s%s%s %.0f %s %s"),
                OrderedTaskFactoryName(task->GetFactoryType()),
                summary_shape,
                linebreak,
                _("dist."),
                (double)Units::ToUserDistance(stats.distance_nominal),
                Units::GetDistanceName(),
                gate_info.c_str());
  }
}

void
OrderedTaskPointLabel(TaskPoint::Type type, const TCHAR *name,
                      unsigned index, TCHAR* buffer)
{
  switch (type) {
  case TaskPoint::START:
    _stprintf(buffer, _T("S:  %s"), name);
    break;

  case TaskPoint::AST:
    _stprintf(buffer, _T("%d: %s"), index, name);
    break;

  case TaskPoint::AAT:
    _stprintf(buffer, _T("%d: %s"), index, name);
    break;

  case TaskPoint::FINISH:
    _stprintf(buffer, _T("F:  %s"), name);
    break;

  default:
    break;
  }
}

void
OrderedTaskPointLabelMapAction(TaskPoint::Type type, const TCHAR *name,
                               unsigned index, TCHAR* buffer)
{
  switch (type) {
  case TaskPoint::START:
    _stprintf(buffer, _T("Zoom to start: %s"), name);
    break;

  case TaskPoint::AST:
    _stprintf(buffer, _T("Zoom to T%d: %s"), index, name);
    break;

  case TaskPoint::AAT:
    _stprintf(buffer, _T("Drag target %d: %s"), index, name);
    break;

  case TaskPoint::FINISH:
    _stprintf(buffer, _T("Zoom to finish:  %s"), name);
    break;

  default:
    break;
  }
}
void
OrderedTaskPointRadiusLabel(const ObservationZonePoint &ozp, TCHAR* buffer)
{
  switch (ozp.shape) {
  case ObservationZonePoint::FAI_SECTOR:
    _tcscpy(buffer, _("FAI quadrant"));
    return;

  case ObservationZonePoint::SECTOR:
  case ObservationZonePoint::ANNULAR_SECTOR:
    _stprintf(buffer,_T("%s  - %s: %.1f%s"), _("Sector"), _("Radius"),
              (double)Units::ToUserDistance(((const SectorZone &)ozp).GetRadius()),
              Units::GetDistanceName());
    return;

  case ObservationZonePoint::LINE:
    _stprintf(buffer,_T("%s: %.1f%s"), _("Gate Width"),
              (double)Units::ToUserDistance(((const LineSectorZone &)ozp).GetLength()),
              Units::GetDistanceName());
    return;

  case ObservationZonePoint::CYLINDER:
  case ObservationZonePoint::MAT_CYLINDER:
    _stprintf(buffer,_T("%.1f%s"),
              (double)Units::ToUserDistance(((const CylinderZone &)ozp).GetRadius()),
              Units::GetDistanceName());
    return;

  case ObservationZonePoint::KEYHOLE:
    _tcscpy(buffer, _("DAeC Keyhole"));
    return;

  case ObservationZonePoint::BGAFIXEDCOURSE:
    _tcscpy(buffer, _("BGA Fixed Course"));
    return;

  case ObservationZonePoint::BGAENHANCEDOPTION:
    _tcscpy(buffer, _("BGA Enhanced Option"));
    return;

  case ObservationZonePoint::BGA_START:
    _tcscpy(buffer, _("BGA Start Sector"));
    return;
  }

  assert(false);
}

const TCHAR*
OrderedTaskPointDescription(TaskPointFactoryType type)
{
  switch (type) {
  case TaskPointFactoryType::START_SECTOR:
    return _("A 90 degree sector with 1km radius. Cross corner edge from inside area to start.");
  case TaskPointFactoryType::START_LINE:
    return _("A straight line start gate.  Cross start gate from inside area to start.");
  case TaskPointFactoryType::START_CYLINDER:
    return _("A cylinder.  Exit area to start.");
  case TaskPointFactoryType::START_BGA:
    return _("A 180 degree sector with 5km radius.  Exit area in any direction to start.");
  case TaskPointFactoryType::FAI_SECTOR:
    return _("A 90 degree sector with 'infinite' length sides.  Cross any edge, scored from "
        "corner point.");
  case TaskPointFactoryType::AST_CYLINDER:
    return _("A cylinder.  Any point within area scored from center.");
  case TaskPointFactoryType::KEYHOLE_SECTOR:
    return _("(German rules) Any point within 1/2 km of center or 10km of a 90 degree sector.  "
        "Scored from center.");
  case TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR:
    return _("(British rules) Any point within 1/2 km of center or 20km of a 90 degree sector.  "
        "Scored from center.");
  case TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR:
    return _("(British rules) Any point within 1/2 km of center or 10km of a 180 degree sector.  "
        "Scored from center.");
  case TaskPointFactoryType::AAT_CYLINDER:
    return _("A cylinder.  Scored by farthest point reached in area.");
  case TaskPointFactoryType::MAT_CYLINDER:
    return _("A 1 mile cylinder.  Scored by farthest point reached in area.");
  case TaskPointFactoryType::AAT_SEGMENT:
    return _("A sector that can vary in angle and radius.  Scored by farthest point reached "
        "inside area.");
  case TaskPointFactoryType::AAT_ANNULAR_SECTOR:
    return _("A sector that can vary in angle, inner and outer radius.  Scored by farthest point "
        "reached inside area.");
  case TaskPointFactoryType::FINISH_SECTOR:
    return _("A 90 degree sector with 1km radius.  Cross edge to finish.");
  case TaskPointFactoryType::FINISH_LINE:
    return _("Cross finish gate line into area to finish.");
  case TaskPointFactoryType::FINISH_CYLINDER:
    return _("Enter cylinder to finish.");
  }

  assert(false);
  return NULL;
}

const TCHAR*
OrderedTaskPointName(TaskPointFactoryType type)
{
  switch (type) {
  case TaskPointFactoryType::START_SECTOR:
    return _("FAI start quadrant");
  case TaskPointFactoryType::START_LINE:
    return _("Start line");
  case TaskPointFactoryType::START_CYLINDER:
    return _("Start cylinder");
  case TaskPointFactoryType::START_BGA:
    return _("BGA start sector");
  case TaskPointFactoryType::FAI_SECTOR:
    return _("FAI quadrant");
  case TaskPointFactoryType::KEYHOLE_SECTOR:
    return _("Keyhole sector (DAeC)");
  case TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR:
    return _("BGA Fixed Course sector");
  case TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR:
    return _("BGA Enhanced Option Fixed Course sector");
  case TaskPointFactoryType::AST_CYLINDER:
    return _("Turn point cylinder");
  case TaskPointFactoryType::AAT_CYLINDER:
    return _("Area cylinder");
  case TaskPointFactoryType::MAT_CYLINDER:
    return _("Cylinder with 1 mile radius.");
  case TaskPointFactoryType::AAT_SEGMENT:
    return _("Area sector");
  case TaskPointFactoryType::AAT_ANNULAR_SECTOR:
    return _("Area sector with inner radius");
  case TaskPointFactoryType::FINISH_SECTOR:
    return _("FAI finish quadrant");
  case TaskPointFactoryType::FINISH_LINE:
    return _("Finish line");
  case TaskPointFactoryType::FINISH_CYLINDER:
    return _("Finish cylinder");
  }

  assert(false);
  return NULL;
}

bool
OrderedTaskSave(SingleWindow &parent, const OrderedTask& task, bool noask)
{
  assert(protected_task_manager != NULL);

  if (!noask
      && ShowMessageBox(_("Save task?"), _("Task Selection"),
                     MB_YESNO | MB_ICONQUESTION) != IDYES)
    return false;

  TCHAR fname[69] = _T("");
  if (!dlgTextEntryShowModal(parent, fname, 64))
    return false;

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("tasks"));
  Directory::Create(path);

  _tcscat(fname, _T(".tsk"));
  LocalPath(path, _T("tasks"), fname);
  protected_task_manager->TaskSave(path, task);
  return true;
}

const TCHAR*
getTaskValidationErrors(const AbstractTaskFactory::TaskValidationErrorVector v)
{

  static TCHAR err[MAX_PATH];
  err[0] = '\0';

  for (unsigned i = 0; i < v.size(); i++)
    if ((_tcslen(err) + _tcslen(TaskValidationError(v[i]))) < MAX_PATH)
      _tcscat(err, TaskValidationError(v[i]));

  return err;
}
const TCHAR*
TaskValidationError(AbstractTaskFactory::TaskValidationErrorType type)
{
  switch (type) {
  case AbstractTaskFactory::NO_VALID_START:
    return _("No valid start.\n");
  case AbstractTaskFactory::NO_VALID_FINISH:
    return _("No valid finish.\n");
  case AbstractTaskFactory::TASK_NOT_CLOSED:
    return _("Task not closed.\n");
  case AbstractTaskFactory::TASK_NOT_HOMOGENEOUS:
    return _("All turnpoints not the same type.\n");
  case AbstractTaskFactory::INCORRECT_NUMBER_TURNPOINTS:
    return _("Incorrect number of turnpoints.\n");
  case AbstractTaskFactory::EXCEEDS_MAX_TURNPOINTS:
    return _("Too many turnpoints.\n");
  case AbstractTaskFactory::UNDER_MIN_TURNPOINTS:
    return _("Not enough turnpoints.\n");
  case AbstractTaskFactory::TURNPOINTS_NOT_UNIQUE:
    return _("Turnpoints not unique.\n");
  case AbstractTaskFactory::INVALID_FAI_TRIANGLE_GEOMETRY:
    return _("Invalid FAI triangle shape.\n");
  case AbstractTaskFactory::EMPTY_TASK:
    return _("Empty task.\n");
  case AbstractTaskFactory::NON_FAI_OZS:
    return _("non-FAI turn points");
  case AbstractTaskFactory::NON_MAT_OZS:
    return _("non-MAT turn points");
  }

  assert(false);
  return NULL;
}
