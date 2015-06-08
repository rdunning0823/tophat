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

#ifndef XCSOAR_TASK_NAV_DATA_CACHE_HPP
#define XCSOAR_TASK_NAV_DATA_CACHE_HPP

#include "Task/AbstractTask.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Util/StaticString.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Engine/Task/Factory/TaskFactoryType.hpp"

#include <assert.h>


struct ComputerSettings;
struct NMEAInfo;
class TaskStats;
class GotoTask;

/**
 * a class for calculating glide solutions ordered task points and storing them
 * This class will never lock the task manager
 */
class TaskNavDataCache
{
public:
  enum {
    MAX_TURNPOINTS = 50,
    CACHE_LATENCY = 1000
  };

  struct tp_info {
    unsigned timestamp;

    /**
     * The waypoint of the task point
     */
    const Waypoint* waypoint;

    /**
     * the location of the target if available
     * if nullptr, then no target is available, else target is available
     * and used for distance_remaining and delta_bearing_remaining calculations
     */
    const GeoPoint* target;

    fixed distance;

    /**
     * true if distance was computable
     */
    bool distance_valid;
    Angle delta_bearing;

    /**
     * true if delta_bearing was computable
     */
    bool bearing_valid;
    fixed altitude_difference;

    /**
     * true if altidude difference was computable
     */
    bool altitude_difference_valid;

    /**
     * altitude difference to target
     */
    fixed altitude_difference_remaining;

    /**
     * true if altitude difference to target was computable
     */
    bool altitude_difference_remaining_valid;

    /**
     * distance to target
     */
    fixed distance_remaining;
    bool distance_remaining_valid;

    /**
     *  bearing to the target
     */
    Angle delta_bearing_remaining;
    bool delta_bearing_remaining_valid;

    /*
     * For ordered, true if the ScoredTaskPoint has been entered
     * For non-ordered, undefined.
     */
    bool has_entered;

    /*
     * For ordered, true if the ScoredTaskPoint has been exited
     * For non-ordered, undefined.
     */
    bool has_exited;

    tp_info() : timestamp(0), waypoint(nullptr), target(nullptr) {};

    bool IsValid() {
      return timestamp > 0u;
    };

    void Invalidate() {
      timestamp = 0u;
    }

    bool IsCurrent() {
      return MonotonicClockMS() < timestamp + CACHE_LATENCY;
    }

    /**
     * has the plane entered the sector?
     */
    bool GetHasEntered() {
      return has_entered;
    };

    /**
     * has the plane exited the sector?
     */
    bool GetHasExited() {
      return has_exited;
    };
  };

private:
  const ComputerSettings &settings;
  const NMEAInfo &basic;

protected:
  tp_info tps[MAX_TURNPOINTS];

  /**
   * tp info for the active turnpoint.  For Goto task, abort mode and home task
   */
  tp_info active_tp;

  /**
   * size of the ordered_task
   */
  unsigned ordered_task_size;

  /**
   * timestamp from the task manager when task was last read
   */
  unsigned task_manager_time_stamp;

  /**
   * the mode
   */
  TaskType mode;

  /**
   * the type of task (AAT, MAT etc)
   */
  TaskFactoryType task_factory_type;

  /**
   * the active task point index from the task manager
   */
  unsigned active_task_point_index;

  /**
   * the timestamp the transitions were last updated
   */
  unsigned transition_time_stamp;

public:
  TaskNavDataCache(const ComputerSettings &_settings,
                   const NMEAInfo &_basic);

  void UpdateOrderedTask(const OrderedTask &ordered_task,
                         TaskType _mode,
                         TaskFactoryType _task_factory_type,
                         unsigned _active_task_point_index,
                         int _task_manager_time_stamp);

  void SetActiveWaypoint(TaskWaypoint* twp) {
    if (twp == nullptr)
      active_tp.waypoint = nullptr;
    else
      active_tp.waypoint = &twp->GetWaypoint();
    active_tp.Invalidate();
  }

  void SetActiveTaskPointIndex(unsigned i) {
    active_task_point_index = i;
  }

  unsigned GetActiveTaskPointIndex() {
      return active_task_point_index;
  }

  /**
   * returns timestamp that task manager provided when task was read
   */
  unsigned GetTaskManagerTimeStamp() {
      return task_manager_time_stamp;
  }

  unsigned GetOrderedTaskSize() {
    return ordered_task_size;
  }

  /**
   * returns the mode
   */
  TaskType GetTaskMode() {
    return mode;
  }

  /**
   * returns task factory type
   */
  TaskFactoryType GetTaskFactoryType() {
    return task_factory_type;
  }

  tp_info &GetPoint(unsigned i);

  /**
   * invalidates all items in cache
   */
  void SetDirty();

  /**
   * Calcs the active tp cache from the waypoint in active_tp
   * @return active_tp info
   */
  tp_info &CalcActivePoint();

  /**
   * caches the waypoint for the task point
   * @param ordered_task.  the current ordered_task
   * @param idx. the index of the point to be cached
   */
  void SetTaskPoint(const OrderedTask &ordered_task, unsigned idx);

  /**
   * updates the has_entered transition value for the ordered task points
   * assumes the task is current and SetTaskPoint has been called for each
   * task point
   */
  void UpdateTransitions(const OrderedTask &ordered_task);

  /**
   * updates the target positions of the ordered task points
   */
  void UpdateTargets(const OrderedTask &ordered_task);

  /**
   * returns true if the transitions and targets are current (with delay)
   */
  bool AreTransistionsAndTargetsCurrent() {
    return NowStamp() < transition_time_stamp + 2000;
  }

  /**
   * Calcs the glide solution of the ith Task TP cache if it's old.
   * Does not calculate the has_entered status.
   * Must only be called after SetTaskPoint is called.
   * Note: for Target, elevation of TP is used
   * @param i.  index of ordered task
   * @param ordered_task.  The ordered task.
   * @return The udpated ith Task TP info
   */
  tp_info &CalcTaskPoint(unsigned i);

  /**
   * Calcs the ith Task TP cache if it's old
   * @param tp_data. the tp_data item to be calc'd
   * @param wp. The waypoint from which to calc the tp_data
   * @return the calcd tp_data item
   */
  tp_info &CalcPoint(TaskNavDataCache::tp_info &tp_data,
                       const Waypoint &wp);

  /**
   * calculates the altitude differential needed to get to the point
   * using the current flight parameters and settings
   */
  fixed CalcAltitudeDifferential(const GeoPoint &point, fixed point_elevation);

  /**
   * calculates the target bearing, distance and altitude difference
   * for ordered tasks
   * using "distance remaining"
   */
  void CalcTarget(tp_info &tp_data, fixed target_altitude);

  /**
   * validates or invalidates the target-specific information
   * @param valid
   */
  tp_info &ValidateTarget(tp_info &tp_data, bool valid);

  unsigned NowStamp();
};
#endif
