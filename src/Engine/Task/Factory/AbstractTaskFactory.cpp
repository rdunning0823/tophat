/* Copyright_License {

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

#include "AbstractTaskFactory.hpp"
#include "TaskFactoryConstraints.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Ordered/Points/StartPoint.hpp"
#include "Task/Ordered/Points/AATPoint.hpp"
#include "Task/Ordered/Points/ASTPoint.hpp"
#include "Task/Ordered/Points/FinishPoint.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/FAISectorZone.hpp"
#include "Task/ObservationZones/KeyholeZone.hpp"
#include "Task/ObservationZones/BGAFixedCourseZone.hpp"
#include "Task/ObservationZones/BGAEnhancedOptionZone.hpp"
#include "Task/ObservationZones/BGAStartSectorZone.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/MatCylinderZone.hpp"
#include "Task/ObservationZones/AnnularSectorZone.hpp"

#include <algorithm>

fixed
AbstractTaskFactory::GetOZSize(const ObservationZonePoint &oz) const
{
  switch (oz.shape) {
  case ObservationZonePoint::SECTOR:
    return ((const SectorZone &)oz).GetRadius();

  case ObservationZonePoint::LINE:
    return ((const LineSectorZone &)oz).GetLength();

  case ObservationZonePoint::CYLINDER:
  case ObservationZonePoint::MAT_CYLINDER:
    return ((const CylinderZone &)oz).GetRadius();

  case ObservationZonePoint::ANNULAR_SECTOR:
    return ((const AnnularSectorZone &)oz).GetRadius();

  default:
    return fixed_minus_one;
  }
}

OrderedTaskPoint*
AbstractTaskFactory::CreateMutatedPoint(const OrderedTaskPoint &tp,
                                        const TaskPointFactoryType newtype) const
{
  fixed ozsize = GetOZSize(tp.GetObservationZone());
  return CreatePoint(newtype, tp.GetWaypoint(), ozsize, ozsize, ozsize);
}

TaskPointFactoryType
AbstractTaskFactory::GetMutatedPointType(const OrderedTaskPoint &tp) const
{
  const TaskPointFactoryType oldtype = GetType(tp);
  TaskPointFactoryType newtype = oldtype;

  switch (tp.GetType()) {
  case TaskPoint::START:
    if (!IsValidStartType(newtype)) {
      newtype = behaviour.sector_defaults.start_type;
      if (!IsValidStartType(newtype))
        newtype = *start_types.begin();
    }
    break;

  case TaskPoint::AST:
  case TaskPoint::AAT:
    if (!IsValidIntermediateType(newtype)) {
      newtype = behaviour.sector_defaults.turnpoint_type;
      if (!IsValidIntermediateType(newtype)) {
        newtype = *intermediate_types.begin();
      }
    }
    break;

  case TaskPoint::FINISH:
    if (!IsValidFinishType(newtype)) {
      newtype = behaviour.sector_defaults.finish_type;
      if (!IsValidFinishType(newtype))
        newtype = *finish_types.begin();
    }
    break;

  case TaskPoint::UNORDERED:
  case TaskPoint::ROUTE:
    break;
  }
  return newtype;
}

StartPoint*
AbstractTaskFactory::CreateStart(ObservationZonePoint* oz,
                                 const Waypoint& wp) const
{
  return new StartPoint(oz, wp, behaviour, GetOrderedTaskBehaviour());
}

FinishPoint*
AbstractTaskFactory::CreateFinish(ObservationZonePoint* oz,
                                  const Waypoint& wp) const
{
  return new FinishPoint(oz, wp, behaviour, GetOrderedTaskBehaviour());
}

AATPoint*
AbstractTaskFactory::CreateAATPoint(ObservationZonePoint* oz,
                               const Waypoint& wp) const
{
  return new AATPoint(oz, wp, behaviour, GetOrderedTaskBehaviour());
}

ASTPoint*
AbstractTaskFactory::CreateASTPoint(ObservationZonePoint* oz,
                               const Waypoint& wp) const
{
  return new ASTPoint(oz, wp, behaviour, GetOrderedTaskBehaviour());
}

StartPoint* 
AbstractTaskFactory::CreateStart(const Waypoint &wp) const
{
  TaskPointFactoryType type = behaviour.sector_defaults.start_type;
  if (!IsValidStartType(type))
    type = *start_types.begin();

  return CreateStart(type, wp);
}

IntermediateTaskPoint* 
AbstractTaskFactory::CreateIntermediate(const Waypoint &wp) const
{
  if (constraints.homogeneous_tps && task.TaskSize() > 1) {
    TaskPointFactoryType type = GetType(task.GetPoint(1));
    if (IsValidIntermediateType(type))
      return CreateIntermediate(type, wp);
  }

  TaskPointFactoryType type = behaviour.sector_defaults.turnpoint_type;
  if (!IsValidIntermediateType(type))
    type = *intermediate_types.begin();

  return CreateIntermediate(type, wp);
}

FinishPoint* 
AbstractTaskFactory::CreateFinish(const Waypoint &wp) const
{
  TaskPointFactoryType type = behaviour.sector_defaults.finish_type;
  if (!IsValidFinishType(type))
    type = *finish_types.begin();

  return CreateFinish(type, wp);
}

TaskPointFactoryType 
AbstractTaskFactory::GetType(const OrderedTaskPoint &point) const
{
  const ObservationZonePoint &oz = point.GetObservationZone();

  switch (point.GetType()) {
  case TaskPoint::START:
    switch (oz.shape) {
    case ObservationZonePoint::FAI_SECTOR:
      return TaskPointFactoryType::START_SECTOR;

    case ObservationZonePoint::LINE:
      return TaskPointFactoryType::START_LINE;

    case ObservationZonePoint::CYLINDER:
    case ObservationZonePoint::MAT_CYLINDER:
    case ObservationZonePoint::SECTOR:
    case ObservationZonePoint::KEYHOLE:
    case ObservationZonePoint::BGAFIXEDCOURSE:
    case ObservationZonePoint::BGAENHANCEDOPTION:
    case ObservationZonePoint::ANNULAR_SECTOR:
      return TaskPointFactoryType::START_CYLINDER;

    case ObservationZonePoint::BGA_START:
      return TaskPointFactoryType::START_BGA;
    }
    break;

  case TaskPoint::AAT:
    switch (oz.shape) {
    case ObservationZonePoint::SECTOR:
    case ObservationZonePoint::FAI_SECTOR:
    case ObservationZonePoint::KEYHOLE:
    case ObservationZonePoint::BGAFIXEDCOURSE:
    case ObservationZonePoint::BGAENHANCEDOPTION:
    case ObservationZonePoint::BGA_START:
    case ObservationZonePoint::LINE:
      return TaskPointFactoryType::AAT_SEGMENT;

    case ObservationZonePoint::ANNULAR_SECTOR:
      return TaskPointFactoryType::AAT_ANNULAR_SECTOR;

    case ObservationZonePoint::CYLINDER:
      return TaskPointFactoryType::AAT_CYLINDER;

    case ObservationZonePoint::MAT_CYLINDER:
      return TaskPointFactoryType::MAT_CYLINDER;
    }
    break;

  case TaskPoint::AST:
    switch (oz.shape) {
    case ObservationZonePoint::FAI_SECTOR:
      return TaskPointFactoryType::FAI_SECTOR;

    case ObservationZonePoint::KEYHOLE:
      return TaskPointFactoryType::KEYHOLE_SECTOR;

    case ObservationZonePoint::BGAFIXEDCOURSE:
      return TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR;

    case ObservationZonePoint::BGAENHANCEDOPTION:
      return TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR;

    case ObservationZonePoint::BGA_START:
    case ObservationZonePoint::CYLINDER:
    case ObservationZonePoint::MAT_CYLINDER:
    case ObservationZonePoint::SECTOR:
    case ObservationZonePoint::LINE:
    case ObservationZonePoint::ANNULAR_SECTOR:
      return TaskPointFactoryType::AST_CYLINDER;
    }
    break;

  case TaskPoint::FINISH:
    switch (oz.shape) {
    case ObservationZonePoint::BGA_START:
    case ObservationZonePoint::FAI_SECTOR:
      return TaskPointFactoryType::FINISH_SECTOR;

    case ObservationZonePoint::LINE:
      return TaskPointFactoryType::FINISH_LINE;

    case ObservationZonePoint::CYLINDER:
    case ObservationZonePoint::MAT_CYLINDER:
    case ObservationZonePoint::SECTOR:
    case ObservationZonePoint::KEYHOLE:
    case ObservationZonePoint::BGAFIXEDCOURSE:
    case ObservationZonePoint::BGAENHANCEDOPTION:
    case ObservationZonePoint::ANNULAR_SECTOR:
      return TaskPointFactoryType::FINISH_CYLINDER;
    }
    break;

  case TaskPoint::UNORDERED:
  case TaskPoint::ROUTE:
    /* obviously, when we check the type of an OrderedTaskPoint, we
       should never get type==UNORDERED or ROUTE. */
    assert(false);
    break;
  }

  // fail, should never get here
  assert(1);
  return TaskPointFactoryType::START_LINE;
}

OrderedTaskPoint* 
AbstractTaskFactory::CreatePoint(const TaskPointFactoryType type,
                                 const Waypoint &wp) const
{
  return CreatePoint(type, wp, fixed_minus_one, fixed_minus_one, fixed_minus_one);
}

void
AbstractTaskFactory::GetPointDefaultSizes(const TaskPointFactoryType type,
                                          fixed &start_radius,
                                          fixed &turnpoint_radius,
                                          fixed &finish_radius) const
{
  TaskBehaviour ob = this->behaviour;
  if (!positive(ob.sector_defaults.start_radius))
    ob.sector_defaults.start_radius = fixed(1000);

  if (!positive(ob.sector_defaults.turnpoint_radius))
    ob.sector_defaults.turnpoint_radius = fixed(1000);

  if (!positive(ob.sector_defaults.finish_radius))
    ob.sector_defaults.finish_radius = fixed(1000);

  if (start_radius < fixed_zero)
    start_radius = ob.sector_defaults.start_radius;

  if (turnpoint_radius < fixed_zero)
    turnpoint_radius = ob.sector_defaults.turnpoint_radius;

  if (finish_radius < fixed_zero)
    finish_radius = ob.sector_defaults.finish_radius;
}

OrderedTaskPoint*
AbstractTaskFactory::CreatePoint(const TaskPointFactoryType type,
                                 const Waypoint &wp,
                                 fixed start_radius,
                                 fixed turnpoint_radius,
                                 fixed finish_radius) const
{
  GetPointDefaultSizes(type, start_radius, turnpoint_radius, finish_radius);

  switch (type) {
  case TaskPointFactoryType::START_SECTOR:
    return CreateStart(new FAISectorZone(wp.location, false), wp);
  case TaskPointFactoryType::START_LINE:
    return CreateStart(new LineSectorZone(wp.location, start_radius), wp);
  case TaskPointFactoryType::START_CYLINDER:
    return CreateStart(new CylinderZone(wp.location, start_radius), wp);
  case TaskPointFactoryType::START_BGA:
    return CreateStart(new BGAStartSectorZone(wp.location), wp);
  case TaskPointFactoryType::FAI_SECTOR:
    return CreateASTPoint(new FAISectorZone(wp.location, true), wp);
  case TaskPointFactoryType::KEYHOLE_SECTOR:
    return CreateASTPoint(new KeyholeZone(wp.location), wp);
  case TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR:
    return CreateASTPoint(new BGAFixedCourseZone(wp.location), wp);
  case TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR:
    return CreateASTPoint(new BGAEnhancedOptionZone(wp.location), wp);
  case TaskPointFactoryType::AST_CYLINDER:
    return CreateASTPoint(new CylinderZone(wp.location, turnpoint_radius), wp);
  case TaskPointFactoryType::MAT_CYLINDER:
    return CreateAATPoint(new MatCylinderZone(wp.location), wp);
  case TaskPointFactoryType::AAT_CYLINDER:
    return CreateAATPoint(new CylinderZone(wp.location, turnpoint_radius), wp);
  case TaskPointFactoryType::AAT_SEGMENT:
    return CreateAATPoint(new SectorZone(wp.location, turnpoint_radius), wp);
  case TaskPointFactoryType::AAT_ANNULAR_SECTOR:
    return CreateAATPoint(new AnnularSectorZone(wp.location, turnpoint_radius), wp);
  case TaskPointFactoryType::FINISH_SECTOR:
    return CreateFinish(new FAISectorZone(wp.location, false), wp);
  case TaskPointFactoryType::FINISH_LINE:
    return CreateFinish(new LineSectorZone(wp.location, finish_radius), wp);
  case TaskPointFactoryType::FINISH_CYLINDER:
    return CreateFinish(new CylinderZone(wp.location, finish_radius), wp);
  }

  assert(1);
  return NULL;
}

StartPoint* 
AbstractTaskFactory::CreateStart(const TaskPointFactoryType type,
                                 const Waypoint &wp) const
{
  if (!IsValidStartType(type))
    // error, invalid type!
    return NULL;

  return (StartPoint*)CreatePoint(type, wp);
}

IntermediateTaskPoint* 
AbstractTaskFactory::CreateIntermediate(const TaskPointFactoryType type,
                                        const Waypoint &wp) const
{
  if (!IsValidIntermediateType(type))
    return NULL;

  return (IntermediateTaskPoint*)CreatePoint(type, wp);
}

FinishPoint* 
AbstractTaskFactory::CreateFinish(const TaskPointFactoryType type,
                                  const Waypoint &wp) const
{
  if (!IsValidFinishType(type))
    return NULL;

  return (FinishPoint*)CreatePoint(type, wp);
}

bool 
AbstractTaskFactory::Append(const OrderedTaskPoint &new_tp,
                            const bool auto_mutate)
{
  if (task.IsFull())
    return false;

  if (auto_mutate) {
    if (!task.TaskSize()) {
      // empty task, so add as a start point
      if (IsValidType(new_tp, task.TaskSize())) {
        // candidate is ok, so add it
        return task.Append(new_tp);
      } else {
        // candidate must be transformed into a startpoint
        StartPoint* sp = CreateStart(new_tp.GetWaypoint());
        bool success = task.Append(*sp);
        delete sp;
        return success;
      }
    }

    if (task.TaskSize() > 1) {
      const OrderedTaskPoint &tp_before = task.GetTaskPoint(task.TaskSize() - 1);

      if (!IsValidIntermediateType(GetType(tp_before))) {
        // old finish must be mutated into an intermediate point
        IntermediateTaskPoint* sp =
          CreateIntermediate(task.GetTaskPoint(task.TaskSize() - 1).GetWaypoint());

        task.Replace(*sp, task.TaskSize()-1);
        delete sp;
      }
    }

    if (IsValidType(new_tp, task.TaskSize()))
      // ok to append directly
      return task.Append(new_tp);

    // this point must be mutated into a finish
    FinishPoint* sp = CreateFinish(new_tp.GetWaypoint());
    bool success = task.Append(*sp);
    delete sp;
    return success;
  }

  return task.Append(new_tp);
}

bool 
AbstractTaskFactory::Replace(const OrderedTaskPoint &new_tp,
                             const unsigned position,
                             const bool auto_mutate)
{
  if (auto_mutate) {
    if (IsValidType(new_tp, position))
      // ok to replace directly
      return task.Replace(new_tp, position);

    // will need to convert type of candidate
    OrderedTaskPoint *tp;
    if (position == 0) {
      // candidate must be transformed into a startpoint
      tp = CreateStart(new_tp.GetWaypoint());
    } else if (IsPositionFinish(position) &&
               position + 1 == task.TaskSize()) {
      // this point must be mutated into a finish
      tp = CreateFinish(new_tp.GetWaypoint());
    } else {
      // this point must be mutated into an intermediate
      tp = CreateIntermediate(new_tp.GetWaypoint());
    }

    bool success = task.Replace(*tp, position);
    delete tp;
    return success;
  }

  return task.Replace(new_tp, position);
}

bool 
AbstractTaskFactory::Insert(const OrderedTaskPoint &new_tp,
                            const unsigned position,
                            const bool auto_mutate)
{
  if (position >= task.TaskSize())
    return Append(new_tp, auto_mutate);

  if (auto_mutate) {
    if (position == 0) {
      if (task.HasStart()) {
        // old start must be mutated into an intermediate point
        IntermediateTaskPoint* sp =
          CreateIntermediate(task.GetTaskPoint(0).GetWaypoint());
        task.Replace(*sp, 0);
        delete sp;
      }


      if (IsValidType(new_tp, 0)) {
        return task.Insert(new_tp, 0);
      } else {
        // candidate must be transformed into a startpoint
        StartPoint* sp = CreateStart(new_tp.GetWaypoint());
        bool success = task.Insert(*sp, 0);
        delete sp;
        return success;
      }
    } else {
      if (new_tp.IsIntermediatePoint()) {
        // candidate ok for direct insertion
        return task.Insert(new_tp, position);
      } else {
        // candidate must be transformed into a intermediatepoint
        IntermediateTaskPoint* sp = CreateIntermediate(new_tp.GetWaypoint());
        bool success = task.Insert(*sp, position);
        delete sp;
        return success;
      }
    }
  }

  return task.Insert(new_tp, position);
}

bool 
AbstractTaskFactory::Remove(const unsigned position, 
                            const bool auto_mutate)
{
  if (position >= task.TaskSize())
    return false;

  if (auto_mutate) {
    if (position == 0) {
      // special case, remove start point..
      if (task.TaskSize() == 1) {
        return task.Remove(0);
      } else {
        // create new start point from next point
        StartPoint* sp = CreateStart(task.GetTaskPoint(1).GetWaypoint());
        bool success = task.Remove(0) && task.Replace(*sp, 0);
        delete sp;
        return success;
      }
    } else if (IsPositionFinish(position - 1) &&
               position + 1 == task.TaskSize()) {
      // create new finish from previous point
      FinishPoint *sp =
        CreateFinish(task.GetTaskPoint(position - 1).GetWaypoint());
      bool success = task.Remove(position) &&
        task.Replace(*sp, position - 1);
      delete sp;
      return success;
    } else {
      // intermediate point deleted, nothing special to do
      return task.Remove(position);
    }
  }

  return task.Remove(position);
}

bool 
AbstractTaskFactory::Swap(const unsigned position, const bool auto_mutate)
{
  if (task.TaskSize() <= 1)
    return false;
  if (position >= task.TaskSize() - 1)
    return false;

  const OrderedTaskPoint &orig = task.GetTaskPoint(position + 1);
  if (!Insert(orig, position, auto_mutate))
    return false;

  return Remove(position+2, auto_mutate);
}

const OrderedTaskPoint&
AbstractTaskFactory::Relocate(const unsigned position, 
                              const Waypoint& waypoint)
{
  task.Relocate(position, waypoint);
  return task.GetTaskPoint(position);
}

const OrderedTaskBehaviour &
AbstractTaskFactory::GetOrderedTaskBehaviour() const
{
  return task.GetOrderedTaskBehaviour();
}

void 
AbstractTaskFactory::UpdateOrderedTaskBehaviour(OrderedTaskBehaviour& to)
{
  to.fai_finish = constraints.fai_finish;
  to.start_max_speed = fixed_zero;
  to.start_max_height_ref = HeightReferenceType::MSL;
  to.finish_min_height_ref = HeightReferenceType::MSL;
}

bool 
AbstractTaskFactory::IsPositionIntermediate(const unsigned position) const
{
  if (IsPositionStart(position))
    return false;
  if (position >= constraints.max_points)
    return false;
  if (position + 1 < constraints.min_points)
    return true;

  if (constraints.IsFixedSize())
    return (position + 1 < constraints.max_points);
  else if (task.TaskSize() < constraints.min_points)
    return true;
  else
    return (position <= task.TaskSize());
}

bool 
AbstractTaskFactory::IsPositionFinish(const unsigned position) const
{
  if (IsPositionStart(position))
    return false;

  if (position + 1 < constraints.min_points)
    return false;
  if (position + 1 > constraints.max_points)
    return false;

  if (constraints.IsFixedSize())
    return (position + 1 == constraints.max_points);
  else
    return (position + 1 >= task.TaskSize());
}

bool
AbstractTaskFactory::ValidAbstractType(LegalAbstractPointType type, 
                                       const unsigned position) const
{
  const bool is_start = IsPositionStart(position);
  const bool is_finish = IsPositionFinish(position);
  const bool is_intermediate = IsPositionIntermediate(position);

  switch (type) {
  case POINT_START:
    return is_start;
  case POINT_FINISH:
    return is_finish;
  case POINT_AST:
    return is_intermediate &&
      (IsValidIntermediateType(TaskPointFactoryType::FAI_SECTOR) 
       || IsValidIntermediateType(TaskPointFactoryType::AST_CYLINDER)
       || IsValidIntermediateType(TaskPointFactoryType::KEYHOLE_SECTOR)
       || IsValidIntermediateType(TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR)
       || IsValidIntermediateType(TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR));
  case POINT_AAT:
    return is_intermediate &&
      (IsValidIntermediateType(TaskPointFactoryType::AAT_CYLINDER)
       || IsValidIntermediateType(TaskPointFactoryType::MAT_CYLINDER)
       || IsValidIntermediateType(TaskPointFactoryType::AAT_SEGMENT)
       || IsValidIntermediateType(TaskPointFactoryType::AAT_ANNULAR_SECTOR));
  };
  return false;
}

bool 
AbstractTaskFactory::IsValidType(const OrderedTaskPoint &new_tp,
                               unsigned position) const
{
  switch (new_tp.GetType()) {
  case TaskPoint::START:
    return ValidAbstractType(POINT_START, position) &&
        IsValidStartType(GetType(new_tp));

  case TaskPoint::AST:
    return ValidAbstractType(POINT_AST, position) &&
        IsValidIntermediateType(GetType(new_tp));

  case TaskPoint::AAT:
    return ValidAbstractType(POINT_AAT, position)&&
        IsValidIntermediateType(GetType(new_tp));

  case TaskPoint::FINISH:
    return ValidAbstractType(POINT_FINISH, position)&&
        IsValidFinishType(GetType(new_tp));

  case TaskPoint::UNORDERED:
  case TaskPoint::ROUTE:
    /* obviously, when we check the type of an OrderedTaskPoint, we
       should never get type==UNORDERED or ROUTE */
    assert(false);
    break;
  }

  return false;
}

bool
AbstractTaskFactory::IsValidIntermediateType(TaskPointFactoryType type) const
{
  return std::find(intermediate_types.begin(), intermediate_types.end(), type)
    != intermediate_types.end();
}

bool
AbstractTaskFactory::IsValidStartType(TaskPointFactoryType type) const
{
  return std::find(start_types.begin(), start_types.end(), type)
    != start_types.end();
}

bool
AbstractTaskFactory::IsValidFinishType(TaskPointFactoryType type) const
{
  return std::find(finish_types.begin(), finish_types.end(), type)
    != finish_types.end();
}

AbstractTaskFactory::LegalPointVector 
AbstractTaskFactory::GetValidTypes(unsigned position) const
{
  LegalPointVector v;
  if (ValidAbstractType(POINT_START, position))
    v.insert(v.end(), start_types.begin(), start_types.end());

  LegalPointVector i = GetValidIntermediateTypes(position);
  if (!i.empty())
    v.insert(v.end(), i.begin(), i.end());

  if (ValidAbstractType(POINT_FINISH, position))
    v.insert(v.end(), finish_types.begin(), finish_types.end());

  return v;
}

void
AbstractTaskFactory::AddValidationError(TaskValidationErrorType e)
{
  validation_errors.push_back(e);
}

void
AbstractTaskFactory::ClearValidationErrors()
{
  validation_errors.clear();
}

AbstractTaskFactory::TaskValidationErrorVector
AbstractTaskFactory::GetValidationErrors()
{
  return validation_errors;
}

bool
AbstractTaskFactory::CheckAddFinish()
{
 if (task.TaskSize() < 2)
   return false;

 if (task.HasFinish())
   return false;

 FinishPoint *fp = CreateFinish(task.GetPoint(task.TaskSize() - 1).GetWaypoint());
 assert(fp);
 Remove(task.TaskSize() - 1, false);
 Append(*fp, false);
 delete fp;

 return true;
}

bool
AbstractTaskFactory::ValidateFAIOZs()
{
  ClearValidationErrors();
  bool valid = true;

  for (unsigned i = 0; i < task.TaskSize() && valid; i++) {
    const OrderedTaskPoint &tp = task.GetPoint(i);
    const fixed ozsize = GetOZSize(tp.GetObservationZone());

    switch (GetType(tp)) {
    case TaskPointFactoryType::START_BGA:
    case TaskPointFactoryType::START_CYLINDER:
      valid = false;
      break;

    case TaskPointFactoryType::START_SECTOR:
      if (ozsize > fixed(1000.01))
        valid = false;

      break;
    case TaskPointFactoryType::START_LINE:
      if (ozsize > fixed(2000.01))
        valid = false;

      break;

    case TaskPointFactoryType::FAI_SECTOR:
      break;

    case TaskPointFactoryType::AST_CYLINDER:
      if (ozsize > fixed(500.01))
        valid = false;

      break;

    case TaskPointFactoryType::KEYHOLE_SECTOR:
    case TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR:
    case TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR:
    case TaskPointFactoryType::MAT_CYLINDER:
    case TaskPointFactoryType::AAT_CYLINDER:
    case TaskPointFactoryType::AAT_SEGMENT:
    case TaskPointFactoryType::AAT_ANNULAR_SECTOR:
      valid = false;
      break;

    case TaskPointFactoryType::FINISH_SECTOR:
      break;
    case TaskPointFactoryType::FINISH_LINE:
      if (ozsize > fixed(2000.01))
        valid = false;

      break;

    case TaskPointFactoryType::FINISH_CYLINDER:
      valid = false;
      break;
    }
  }

  if (!valid)
    AddValidationError(NON_FAI_OZS);

  return valid;
}

bool
AbstractTaskFactory::ValidateMATOZs()
{
  ClearValidationErrors();
  bool valid = true;

  for (unsigned i = 0; i < task.TaskSize() && valid; i++) {
    const OrderedTaskPoint &tp = task.GetPoint(i);

    switch (GetType(tp)) {
    case TaskPointFactoryType::START_CYLINDER:
    case TaskPointFactoryType::START_LINE:
    case TaskPointFactoryType::START_SECTOR:
      break;

    case TaskPointFactoryType::START_BGA:
      valid = false;
      break;

    case TaskPointFactoryType::MAT_CYLINDER:
      break;

    case TaskPointFactoryType::AAT_CYLINDER:
    case TaskPointFactoryType::FAI_SECTOR:
    case TaskPointFactoryType::AST_CYLINDER:
    case TaskPointFactoryType::KEYHOLE_SECTOR:
    case TaskPointFactoryType::BGAFIXEDCOURSE_SECTOR:
    case TaskPointFactoryType::BGAENHANCEDOPTION_SECTOR:
    case TaskPointFactoryType::AAT_SEGMENT:
    case TaskPointFactoryType::AAT_ANNULAR_SECTOR:
    case TaskPointFactoryType::FINISH_SECTOR:
      valid = false;
      break;

    case TaskPointFactoryType::FINISH_LINE:
    case TaskPointFactoryType::FINISH_CYLINDER:
      break;
    }
  }

  if (!valid)
    AddValidationError(NON_MAT_OZS);

  return valid;
}


bool
AbstractTaskFactory::Validate()
{
  ClearValidationErrors();
  bool valid = true;

  if (!task.HasStart()) {
    AddValidationError(NO_VALID_START);
    valid = false;
  }
  if (!task.HasFinish()) {
    AddValidationError(NO_VALID_FINISH);
    valid = false;
  }

  if (constraints.is_closed && !IsClosed()) {
    AddValidationError(TASK_NOT_CLOSED);
    valid = false;
  }

  if (constraints.IsFixedSize()) {
    if (task.TaskSize() != constraints.max_points) {
      AddValidationError(INCORRECT_NUMBER_TURNPOINTS);
      valid = false;
    }
  } else {
    if (task.TaskSize() < constraints.min_points) {
      AddValidationError(UNDER_MIN_TURNPOINTS);
      valid = false;
    }
    if (task.TaskSize() > constraints.max_points) {
      AddValidationError(EXCEEDS_MAX_TURNPOINTS);
      valid = false;
    }
  }

  if (constraints.homogeneous_tps && !IsHomogeneous()) {
    AddValidationError(TASK_NOT_HOMOGENEOUS);
    valid = false;
  }

  return valid;
}

AbstractTaskFactory::LegalPointVector 
AbstractTaskFactory::GetValidIntermediateTypes(unsigned position) const
{
  LegalPointVector v;

  if (!IsPositionIntermediate(position))
    return v;

  if (constraints.homogeneous_tps &&
      position > 1 && task.TaskSize() > 1) {
    TaskPointFactoryType type = GetType(task.GetPoint(1));
    if (IsValidIntermediateType(type)) {
      v.push_back(type);
      return v;
    }
  }

  if (ValidAbstractType(POINT_AAT, position) ||
      ValidAbstractType(POINT_AST, position))
    v.insert(v.end(), intermediate_types.begin(), intermediate_types.end());

  return v;
}

AbstractTaskFactory::LegalPointVector
AbstractTaskFactory::GetValidStartTypes() const
{
  LegalPointVector v;
  v.insert(v.end(), start_types.begin(), start_types.end());
  return v;
}

AbstractTaskFactory::LegalPointVector
AbstractTaskFactory::GetValidIntermediateTypes() const
{
  LegalPointVector v;
  v.insert(v.end(), intermediate_types.begin(), intermediate_types.end());
  return v;
}

AbstractTaskFactory::LegalPointVector
AbstractTaskFactory::GetValidFinishTypes() const
{
  LegalPointVector v;
  v.insert(v.end(), finish_types.begin(), finish_types.end());
  return v;
}

bool 
AbstractTaskFactory::IsClosed() const
{
  if (task.TaskSize() < 3)
    return false;

  const Waypoint &wp_start = task.GetPoint(0).GetWaypoint();
  const Waypoint& wp_finish =
    task.GetPoint(task.TaskSize() - 1).GetWaypoint();

  return (wp_start.location == wp_finish.location);
}

bool 
AbstractTaskFactory::IsUnique() const
{
  const unsigned size = task.TaskSize();
  for (unsigned i = 0; i + 1 < size; i++) {
    const Waypoint &wp_0 = task.GetPoint(i).GetWaypoint();

    for (unsigned j = i + 1; j < size; j++) {
      if (i == 0 && j + 1 == size) {
        // start point can be similar to finish point
      } else {
        const Waypoint &wp_1 = task.GetPoint(j).GetWaypoint();
        if (wp_1 == wp_0)
          return false;
      }
    }
  }
  return true;
}

bool
AbstractTaskFactory::IsHomogeneous() const
{
  bool valid = true;

  const unsigned size = task.TaskSize();

  if (size > 2) {
    TaskPointFactoryType homogtype = GetType(task.GetPoint(1));

    for (unsigned i = 2; i < size; i++) {
      const OrderedTaskPoint &tp = task.GetPoint(i);
      if (tp.GetType() == TaskPoint::FINISH) {
        ; // don't check a valid finish point
      } else {
        if (GetType(tp) != homogtype) {
          valid = false;
          break;
        }
      }
    }
  }

  return valid;
}

bool
AbstractTaskFactory::RemoveExcessTPsPerTaskType()
{
  bool changed = false;
  unsigned maxtp = constraints.max_points;
  while (maxtp < task.TaskSize()) {
    Remove(maxtp, false);
    changed = true;
  }
  return changed;
}

bool
AbstractTaskFactory::MutateTPsToTaskType()
{
  bool changed = RemoveExcessTPsPerTaskType();

  for (unsigned int i = 0; i < task.TaskSize(); i++) {
    const OrderedTaskPoint &tp = task.GetPoint(i);
    if (!IsValidType(tp, i) ||
        task.GetFactoryType() == TaskFactoryType::FAI_GENERAL ||
        task.GetFactoryType() == TaskFactoryType::MAT ) {

      TaskPointFactoryType newtype = GetMutatedPointType(tp);
      if (IsPositionFinish(i)) {

        if (!IsValidFinishType(newtype)) {
          newtype = behaviour.sector_defaults.finish_type;
          if (!IsValidFinishType(newtype))
            newtype = *finish_types.begin();
        }

        FinishPoint *fp = (FinishPoint*)CreateMutatedPoint(tp, newtype);
        assert(fp);
        if (Replace(*fp, i, true))
          changed = true;
        delete fp;

      } else if (i == 0) {
        if (!IsValidStartType(newtype)) {
          newtype = behaviour.sector_defaults.start_type;
          if (!IsValidStartType(newtype))
            newtype = *start_types.begin();
        }

        StartPoint *sp = (StartPoint*)CreateMutatedPoint(tp, newtype);
        assert(sp);
        if (Replace(*sp, i, true))
          changed = true;
        delete sp;

      } else {

        if (!IsValidIntermediateType(newtype)) {
          newtype = behaviour.sector_defaults.turnpoint_type;
          if (!IsValidIntermediateType(newtype))
            newtype = *intermediate_types.begin();
        }
        OrderedTaskPoint *tpnew = (OrderedTaskPoint*)CreateMutatedPoint(tp, newtype);
        if (Replace(*tpnew, i, true))
          changed = true;
        delete tpnew;
      }
    }
  }

  changed |= MutateClosedFinishPerTaskType();
  return changed;
}

bool
AbstractTaskFactory::MutateClosedFinishPerTaskType()
{
  if (task.TaskSize() < 2)
    return false;

  if (!IsPositionFinish(task.TaskSize() - 1))
    return false;

  bool changed = false;

  if (constraints.is_closed) {
    if (!IsClosed()) {
      const OrderedTaskPoint &tp = task.GetPoint(task.TaskSize() - 1);
      if (tp.GetType() == TaskPoint::FINISH) {
        FinishPoint *fp = CreateFinish(task.GetPoint(0).GetWaypoint());
        assert(fp);
        Remove(task.TaskSize() - 1, false);
        Append(*fp, false);
        delete fp;
        changed = true;
      }
    }
  }
  return changed;
}

bool 
AbstractTaskFactory::AppendOptionalStart(const Waypoint& wp)
{
  OrderedTaskPoint* tp = NULL;
  if (task.TaskSize())
    tp = task.GetPoint(0).Clone(behaviour,
                                  GetOrderedTaskBehaviour(), &wp);
  else
    tp = CreateStart(wp);

  if (!tp)
    return false; // should never happen

  bool success = task.AppendOptionalStart(*tp);
  delete tp;
  return success;
}

bool
AbstractTaskFactory::AppendOptionalStart(const OrderedTaskPoint &new_tp,
                                           const bool auto_mutate)
{
  if (auto_mutate && !IsValidType(new_tp, 0)) {
    // candidate must be transformed into a startpoint of appropriate type
    StartPoint* sp = CreateStart(new_tp.GetWaypoint());
    bool success = task.AppendOptionalStart(*sp);
    delete sp;
    return success;
  }
  // ok to add directly
  return task.AppendOptionalStart(new_tp);
}
