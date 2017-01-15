/* Copyright_License {

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
#ifndef TASKSTATS_HPP
#define TASKSTATS_HPP

#include "ElementStat.hpp"
#include "StartStats.hpp"
#include "WindowStats.hpp"
#include "Task/Computer/WindowStatsComputer.hpp"
#include "Util/StaticArray.hpp"

#include <type_traits>

struct TaskBehaviour;
enum {
  /* max tps is 30 for MAT task */
  max_glide_results = 30,
};

/** Container for common task statistics */
class TaskStats 
{
public:
  /** Total task statistics */
  ElementStat total;
  /** Current (active) leg statistics */
  ElementStat current_leg;

  /** Calculated glide angle required */
  fixed glide_required;
  /** Calculated cruise efficiency ratio */
  fixed cruise_efficiency;
  /** Calculated effective MC (m/s) */
  fixed effective_mc;
  /** Best MacCready setting calculated for final glide (m/s) */
  fixed mc_best;
  /** Used for planning the task, setting target, estimating completion time. */
  fixed task_mc;
  /** The effective speed for the task calculated using the task_mc */
  fixed task_mc_effective_speed;

  /** Nominal task distance (m) */
  fixed distance_nominal;
  /** Maximum achievable task distance (m) */
  fixed distance_max;
  /** Minimum achievable task distance (m) */
  fixed distance_min;
  /** Scored distance (m) */
  fixed distance_scored;

  /**
   * Glide solution to any tp in task
   * In reality, only prev and next are accurate
   */
  GlideResult glide_results[max_glide_results];
  /**
   * Glide result for goto tasks
   */
  GlideResult glide_result_goto;

  /**
   * Index of the active task point.
   */
  unsigned active_index;

  /** Whether the task is navigable */
  bool task_valid;

  /** Whether ordered task has AAT areas */
  bool has_targets;

  /**
   * Is this a MAT task?
   */
  bool is_mat;

  /** Whether ordered task has optional starts */
  bool has_optional_starts;

  /** Whether the task is finished */
  bool task_finished;

  /**
   * Is the aircraft currently inside the current task point's
   * observation zone?
   */
  bool inside_oz;

  /**
   * Does the current task point need to be armed?
   */
  bool need_to_arm;

  /** Whether the task is appoximately in final glide */
  bool flight_mode_final_glide;

  /** syncs time w/ distance in scored speed calc if distance is behind
   * a second */
  mutable fixed scored_speed_sync_offset;

  StartStats start;

  WindowStats last_hour;

  fixed GetEstimatedTotalTime() const {
    return total.time_elapsed + total.time_remaining_start;
  }

  /**
   * Check whether get_pirker_speed() is available.
   */
  bool IsPirkerSpeedAvailable() const {
    return total.pirker.IsDefined();
  }

  /** Incremental task speed adjusted for mc, target changes */
  fixed get_pirker_speed() const {
    return total.pirker.GetSpeedIncremental();
  }

  fixed GetScoredSpeed() const {
    // hack. distance stat is one second behind time stat so synchronize here
    if (positive(distance_scored) && !positive(total.time_elapsed))
      scored_speed_sync_offset = fixed(1);

    if (positive(total.time_elapsed))
      return distance_scored / (total.time_elapsed + scored_speed_sync_offset);
    else
      return fixed(0);
  }

  /** Reset each element (for incremental speeds). */
  void reset();

  /**
   * Convenience function, determines if remaining task is in final glide
   *
   * @return True if is mode changed
   */
  bool calc_flight_mode(const TaskBehaviour &settings);
};

static_assert(std::is_trivial<TaskStats>::value, "type is not trivial");

#endif
