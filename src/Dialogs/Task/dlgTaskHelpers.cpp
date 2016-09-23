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

#include "dlgTaskHelpers.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "Units/Units.hpp"
#include "Task/TypeStrings.hpp"
#include "Task/ValidationErrorStrings.hpp"
#include "Task/SaveFile.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/SectorZone.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/KeyholeZone.hpp"
#include "Task/Shapes/FAITriangleTask.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Points/Type.hpp"
#include "Engine/Task/Factory/AbstractTaskFactory.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Util/StringUtil.hpp"
#include "Util/StaticString.hxx"
#include "Interface.hpp"
#include "OS/PathName.hpp"

#include <assert.h>
#include <stdio.h>
#include <windef.h> /* for MAX_PATH */

/**
 *
 * @param task
 * @param text
 * @return True if FAI shape
 */
static bool
TaskSummaryShape(const OrderedTask *task, TCHAR *text)
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
      _tcscpy(text, _("non-FAI triangle"));
    break;

  default:
    StringFormatUnsafe(text, _("%d legs"), task->TaskSize() - 1);
    break;
  }
  return FAIShape;
}
void
OrderedTaskSummary(const OrderedTask *task, TCHAR *text, bool linebreaks)
{
  StaticString<120> gate_info;
  gate_info.clear();
  const OrderedTaskSettings &otb = task->GetOrderedTaskSettings();

  if (!otb.finish_constraints.fai_finish) {
    StaticString<25> start_height;
    StaticString<25> finish_height;

    FormatUserAltitude(fixed(otb.start_constraints.max_height), start_height.buffer(), true);
    FormatUserAltitude(fixed(otb.finish_constraints.min_height), finish_height.buffer(), true);

    gate_info.Format(_T("\n%s: %s %s. %s: %s %s. "),
                     _("Start Height"),
                     start_height.c_str(),
                     (otb.start_constraints.max_height_ref == AltitudeReference::AGL)
                     ? _("AGL") : _("MSL"),
                     _("Finish Height"),
                     finish_height.c_str(),
                     (otb.finish_constraints.min_height_ref == AltitudeReference::AGL)
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
    StringFormatUnsafe(text, _("Task is empty (%s)"),
                       OrderedTaskFactoryName(task->GetFactoryType()));
  } else {
    if (task->HasTargets())
      StringFormatUnsafe(text, _T("%s. %s%s%.0f %s%s%s %.0f %s%s%s %.0f %s %s"),
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
    else {
      UnitSetting &config = CommonInterface::SetUISettings().format.units;
      StaticString<15>km_display (_T(""));
      if (config.distance_unit != Unit::KILOMETER) {
        fixed km = Units::ToUserUnit(stats.distance_nominal, Unit::KILOMETER);
        km_display.Format(_T(" (%.1f km)"), (double)km);
      }
      StringFormatUnsafe(text, _T("%s. %s%s%s %.0f %s %s%s"),
                         OrderedTaskFactoryName(task->GetFactoryType()),
                         summary_shape,
                         linebreak,
                         _("dist."),
                         (double)Units::ToUserDistance(stats.distance_nominal),
                         Units::GetDistanceName(),
                         gate_info.c_str(),
                         km_display.c_str());
    }
  }
}

void
OrderedTaskPointLabel(TaskPointType type, const TCHAR *name,
                      unsigned index, TCHAR* buffer)
{
  switch (type) {
  case TaskPointType::START:
    StringFormatUnsafe(buffer, _T("S: %s"), name);
    break;

  case TaskPointType::AST:
    StringFormatUnsafe(buffer, _T("%d: %s"), index, name);
    break;

  case TaskPointType::AAT:
    StringFormatUnsafe(buffer, _T("%d: %s"), index, name);
    break;

  case TaskPointType::FINISH:
    StringFormatUnsafe(buffer, _T("F: %s"), name);
    break;

  default:
    break;
  }
}

void
OrderedTaskPointLabelMapAction(TaskPointType type, const TCHAR *name,
                               unsigned index, TCHAR* buffer)
{
  switch (type) {
  case TaskPointType::START:
    _stprintf(buffer, _T("%s %s: %s"), _("Zoom to"), _("Start"), name);
    break;

  case TaskPointType::AST:
    _stprintf(buffer, _T("%s T%d: %s"), _("Zoom to"), index, name);
    break;

  case TaskPointType::AAT:
    _stprintf(buffer, _T("%s: %s"), _("Drag target"), name);
    break;

  case TaskPointType::FINISH:
    _stprintf(buffer, _T("%s finish:  %s"), _("Zoom to"), name);
    break;

  default:
    break;
  }
}

void
OrderedTaskPointRadiusLabel(const ObservationZonePoint &ozp, TCHAR* buffer)
{
  switch (ozp.GetShape()) {
  case ObservationZone::Shape::FAI_SECTOR:
    _tcscpy(buffer, _("FAI quadrant"));
    return;

  case ObservationZone::Shape::SECTOR:
  case ObservationZone::Shape::ANNULAR_SECTOR:
    StringFormatUnsafe(buffer,_T("%s - %s: %.1f%s"), _("Sector"), _("Radius"),
                       (double)Units::ToUserDistance(((const SectorZone &)ozp).GetRadius()),
                       Units::GetDistanceName());
    return;

  case ObservationZone::Shape::LINE:
    StringFormatUnsafe(buffer,_T("%s - %s: %.1f%s"), _("Line"), _("Gate width"),
                       (double)Units::ToUserDistance(((const LineSectorZone &)ozp).GetLength()),
                       Units::GetDistanceName());
    return;

  case ObservationZone::Shape::CYLINDER:
  case ObservationZone::Shape::MAT_CYLINDER:
    StringFormatUnsafe(buffer,_T("%.1f%s"),
                       (double)Units::ToUserDistance(((const CylinderZone &)ozp).GetRadius()),
                       Units::GetDistanceName());
    return;

  case ObservationZone::Shape::CUSTOM_KEYHOLE:
    StringFormatUnsafe(buffer,_T("%s - %s: %.1f%s"), _("Keyhole"), _("Radius"),
                       (double)Units::ToUserDistance(((const KeyholeZone &)ozp).GetRadius()),
                       Units::GetDistanceName());
    return;

  case ObservationZone::Shape::DAEC_KEYHOLE:
    _tcscpy(buffer, _("DAeC Keyhole"));
    return;

  case ObservationZone::Shape::BGAFIXEDCOURSE:
    _tcscpy(buffer, _("BGA Fixed Course"));
    return;

  case ObservationZone::Shape::BGAENHANCEDOPTION:
    _tcscpy(buffer, _("BGA Enhanced Option"));
    return;

  case ObservationZone::Shape::BGA_START:
    _tcscpy(buffer, _("BGA Start Sector"));
    return;

  case ObservationZone::Shape::SYMMETRIC_QUADRANT:
    _tcscpy(buffer, _("Symmetric quadrant"));
    return;
  }

  gcc_unreachable();
  assert(false);
}

bool
OrderedTaskSave(OrderedTask &task)
{
  TCHAR fname[69] = _T("");
  CopyString(fname, task.GetName(), StringLength(task.GetName()) + 1);

  if (!TextEntryDialog(fname, 64, _("Enter a task name")))
    return false;

  task.SetName(fname);
  TCHAR path[MAX_PATH];
  LocalPath(path, _T("tasks"));
  Directory::Create(path);

  if (!MatchesExtension(fname, _T(".tsk")))
    _tcscat(fname, _T(".tsk"));
  LocalPath(path, _T("tasks"), fname);
  SaveTask(path, task);
  return true;
}
