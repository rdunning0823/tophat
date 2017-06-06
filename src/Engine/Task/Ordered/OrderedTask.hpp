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

#ifndef ORDEREDTASK_H
#define ORDEREDTASK_H

#include "Geo/Flat/TaskProjection.hpp"
#include "Task/AbstractTask.hpp"
#include "SmartTaskAdvance.hpp"
#include "Util/DereferenceIterator.hpp"
#include "Util/StaticString.hxx"
#include "Task/Stats/StartStats.hpp"
#include "Navigation/Aircraft.hpp"

#include <assert.h>
#include <vector>
#include <tchar.h>

class SearchPoint;
class SearchPointVector;
class OrderedTaskPoint;
class StartPoint;
class FinishPoint;
class AbstractTaskFactory;
class TaskDijkstraMin;
class TaskDijkstraMax;
struct Waypoint;
class Waypoints;
class AATPoint;
class FlatBoundingBox;
class GeoBounds;
struct TaskSummary;
struct TaskFactoryConstraints;

/**
 * A task comprising an ordered sequence of task points, each with
 * observation zones.  A valid OrderedTask has a StartPoint, zero or more
 * IntermediatePoints and a FinishPoint.
 *
 * \todo
 * - better handling of removal of start/finish point
 * - allow for TakeOffPoint and LandingPoint
 * - have a method to check if a potential taskpoint is distinct from its neighbours?
 * - multiple start points
 */
class OrderedTask final : public AbstractTask
{
  friend class PointStateDeserialiser;

public:
  /** Storage type of task points */
  typedef std::vector<OrderedTaskPoint*> OrderedTaskPointVector;

  typedef DereferenceContainerAdapter<const OrderedTaskPointVector,
                                      const OrderedTaskPoint> ConstTaskPointList;

private:
  OrderedTaskPointVector task_points;
  OrderedTaskPointVector optional_start_points;

  StartPoint *taskpoint_start;
  FinishPoint *taskpoint_finish;

  TaskProjection task_projection;

  GeoPoint last_min_location;

  TaskFactoryType factory_mode;
  AbstractTaskFactory* active_factory;
  OrderedTaskSettings ordered_settings;
  SmartTaskAdvance task_advance;
  TaskDijkstraMin *dijkstra_min;
  TaskDijkstraMax *dijkstra_max;

  /* state that triggered the start prior to the most recent start */
  StartStats saved_start_stats_pushed;
  AircraftState saved_start_state_pushed;

  /* is the saved start valid? */
  bool saved_start_pushed_valid;

  /* the last speed used to calculate the task_mc */
  fixed last_task_mc_speed;

    /** name of task */
  StaticString<64> name;

  bool is_glider_close_to_start_cylinder;

public:
  /**
   * Constructor.
   *
   * \todo
   * - default values in constructor
   *
   * @param tb Task behaviour
   *
   * @return Initialised object
   */
  explicit OrderedTask(const TaskBehaviour &tb);
  ~OrderedTask();

  /**
   * Accessor for factory system for constructing tasks
   *
   * @return Factory
   */
  gcc_pure
  AbstractTaskFactory& GetFactory() const {
    return *active_factory;
  }

  gcc_pure
  const TaskFactoryConstraints &GetFactoryConstraints() const;

  /**
   * Set type of task factory to be used for constructing tasks
   *
   * @param _factory Type of task
   */
  void SetFactory(const TaskFactoryType _factory);

  /** 
   * Return list of factory types
   *
   * @param all If true, return all types, otherwise only valid transformable ones
   *
   * @return Vector of factory types
   */
  gcc_pure
  std::vector<TaskFactoryType> GetFactoryTypes(bool all = true) const;

  void SetTaskBehaviour(const TaskBehaviour &tb);

  /**
   * Removes all task points.
   */
  void RemoveAllPoints();

  /**
   * Clear all points and restore default ordered task behaviour
   * for the active factory
   */
  void Clear();

  /**
   * Create a clone of the task.
   * Caller is responsible for destruction.
   *
   * @param te Task events
   * @param tb Task behaviour
   *
   * @return Initialised object
   */
  gcc_malloc
  OrderedTask *Clone(const TaskBehaviour &tb) const;

  /**
   * Copy task into this task
   *
   * @param other OrderedTask to copy
   * @param waypoints.  const reference to the waypoint file
   * @return True if this task changed
   */
  bool Commit(const OrderedTask& other);

  /**
   * Retrieves the active task point index.
   *
   * @return Index of active task point sequence
   */
  gcc_pure
  unsigned GetActiveIndex() const {
    return active_task_point;
  }

  /**
   * Retrieve task point by sequence index
   *
   * @param index Index of task point sequence
   *
   * @return OrderedTaskPoint at index
   */
  gcc_pure
  const OrderedTaskPoint &GetTaskPoint(const unsigned index) const {
    assert(index < task_points.size());

    return *task_points[index];
  }

  /**
   * Check if task has a single StartPoint
   *
   * @return True if task has start
   */
  gcc_pure
  bool HasStart() const {
    return taskpoint_start != nullptr;
  }

  /**
   * Check if task has a single FinishPoint
   *
   * @return True if task has finish
   */
  gcc_pure
  bool HasFinish() const {
    return taskpoint_finish != nullptr;
  }

  /**
   * Cycle through optional start points, replacing actual task start point
   * with top item in optional starts.
   */
  void RotateOptionalStarts();

  /**
   * Returns true if there are optional start points.
   */
  gcc_pure
  bool HasOptionalStarts() const {
    return !optional_start_points.empty();
  }

  /* returns true if the task start point radius should be subtracted from
   * distance calculation
   */
  gcc_pure
  bool ScoredAdjustmentStart() const;

  /**
   * Insert taskpoint before specified index in task.  May fail if the candidate
   * is the wrong type (e.g. if it is a StartPoint and the task already
   * has one).
   * Ownership is transferred to this object.
   *
   * @param tp Taskpoint to insert
   * @param position Index in task sequence, before which to insert
   *
   * @return True on success
   */
  bool Insert(const OrderedTaskPoint &tp, const unsigned position);

  /**
   * Replace taskpoint.
   * May fail if the candidate is the wrong type.
   * Does nothing (but returns true) if replacement is equivalent
   * Ownership is transferred to this object.
   *
   * @param tp Taskpoint to become replacement
   * @param position Index in task sequence of task point to replace
   *
   * @return True on success
   */
  bool Replace(const OrderedTaskPoint &tp, const unsigned position);

  /**
   * Replace optional start point.
   * May fail if the candidate is the wrong type.
   * Does nothing (but returns true) if replacement is equivalent
   * Ownership is transferred to this object.
   *
   * @param tp Taskpoint to become replacement
   * @param position Index in task sequence of task point to replace
   *
   * @return True on success
   */
  bool ReplaceOptionalStart(const OrderedTaskPoint &tp, const unsigned position);

  /**
   * Append taskpoint to end of task.  May fail if the candidate
   * is the wrong type (e.g. if it is a StartPoint and the task already
   * has one).
   * Ownership is transferred to this object.
   *
   * @param tp Taskpoint to append to task
   *
   * @return True on success
   */
  bool Append(const OrderedTaskPoint &tp);

  /**
   * Append optional start point.  May fail if the candidate
   * is the wrong type.
   * Ownership is transferred to this object.
   *
   * @param tp Taskpoint to append to task
   *
   * @return True on success
   */
  bool AppendOptionalStart(const OrderedTaskPoint &tp);

  /**
   * Remove task point at specified position.  Note that
   * currently start/finish points can't be removed.
   *
   * @param position Index in task sequence of task point to remove
   *
   * @return True on success
   */
  bool Remove(const unsigned position);

  /**
   * Remove optional start point at specified position.
   *
   * @param position Index in sequence of optional start point to remove
   *
   * @return True on success
   */
  bool RemoveOptionalStart(const unsigned position);

  /**
   * Change the waypoint of an optional start point
   * @param position valid index to optional start point
   * @param waypoint
   * @return true if succeeded
   */
  bool RelocateOptionalStart(const unsigned position, const Waypoint& waypoint);

  /**
   * Relocate a task point to a new location
   *
   * @param position Index in task sequence of task point to replace
   * @param waypoint Waypoint of replacement
   *
   * @return True on success
   */
  bool Relocate(const unsigned position, const Waypoint& waypoint);

 /**
  * returns pointer to AATPoint accessed via TPIndex if exist
  *
  * @param TPindex index of taskpoint
  *
  * @return pointer to tp if valid, else nullptr
  */
 AATPoint* GetAATTaskPoint(unsigned index) const;

  /**
   * Check whether the task point with the specified index exists.
   */
  gcc_pure
  bool IsValidIndex(unsigned i) const {
    return i < task_points.size();
  }

  /**
   * Determine whether the task is full according to the factory in use
   *
   * @return True if task is full
   */
  gcc_pure
  bool IsFull() const;

  /**
   * Accessor for task projection, for use when creating task points
   *
   * @return Task global projection
   */
  gcc_pure
  const TaskProjection&
  GetTaskProjection() const {
    return task_projection;
  }

  void CheckDuplicateWaypoints(Waypoints& waypoints);

  /**
   * Update TaskStats::{task_valid, has_targets, is_mat, has_optional_starts}.
   */
  void UpdateStatsGeometry();

  /**
   * Update internal geometric state of task points.
   * Typically called after task geometry or observation zones are modified.
   *
   *
   * This also updates planned/nominal distances so clients can use that
   * data during task construction.
   */
  void UpdateGeometry();

  /**
   * Convert a GeoBounds into a flat bounding box projected
   * according to the task projection.
   */
  gcc_pure
  FlatBoundingBox GetBoundingBox(const GeoBounds &bounds) const;

  /**
   * Update summary task statistics (progress along path)
   */
  void UpdateSummary(TaskSummary &summary) const;

public:
  /**
   * Retrieve vector of search points to be used in max/min distance
   * scans (by TaskDijkstra).
   *
   * @param tp Index of task point of query
   *
   * @return Vector of search point candidates
   */
  const SearchPointVector &GetPointSearchPoints(unsigned tp) const;

protected:
  /**
   * Set task point's minimum distance value (by TaskDijkstra).
   *
   * @param tp Index of task point to set min
   * @param sol Search point found to be minimum distance
   */
  void SetPointSearchMin(unsigned tp, const SearchPoint &sol);

  /**
   * Set task point's max achieved distance value (by TaskDijkstra).
   *
   * @param tp Index of task point to set
   * @param sol Search point to be set for task point's max achieved distance
   */
  void SetPointSearchMaxAchieved(unsigned tp, const SearchPoint &sol);

  /**
   * Set task point's maximum distance value (by TaskDijkstra).
   *
   * @param tp Index of task point to set max
   * @param sol Search point found to be maximum distance
   */
  void SetPointSearchMax(unsigned tp, const SearchPoint &sol);

  /**
   * Set task point's minimum distance achieved value
   *
   * @param tp Index of task point to set min
   * @param sol Search point found to be minimum distance
   */
  void set_tp_search_achieved(unsigned tp, const SearchPoint &sol);

public:
  /**
   * Scan task for valid start/finish points
   *
   * @return True if start and finish found
   */
  bool ScanStartFinish();

  /**
   * Saves the stats of the last start if state.location & time are valid
   *
   * @param stats.  The task stats of the aircraft at the time of the last start
   * @param state.  The state of the aircraft at the time of the last start
   * @return. true if the prior state was valid and saved
   */
  bool SavedStartSave(const StartStats &stats, const AircraftState &state);

  /**
   * Restores the last pushed start data: time, location
   * TODO: update samples around start
   */
  void SavedStartRestore();

  /**
   * Invalidates the saved start
   */
  void SavedStartInvalidate();

  /**
   * Are the saved start valid?
   */
  bool SavedStartIsValid() const;

  /** restores state of task from saved file */
  void RestoreTaskState();

private:

  /**
   * calculate angles for calculating landout distances for speed calculations
   */
  void SetLandoutDistanceGeometry(bool force);

  /**
   * Computes the max achieved distance for the tps in the task
   * that have been sampled or up to the current index whichever is greater
   *
   * @return true if a solution was found (and applied)
   */
  bool RunDijsktraMaxAchieved();

  /**
   * Sets the SearchMin points for each task point from the current to
   * the end.
   * shortest remaining solution. In other words, the current OZ will be
   * the point that is closest to finish.
   * @return true if a solution was found (and applied)
   */
  bool RunDijsktraMin(const GeoPoint &location);


  fixed ScanDistanceMin(const GeoPoint &ref, bool full);

  /**
   * @return true if a solution was found (and applied)
   * Updates search point max for each tp in task
   * Updates search point achieved for prior tps in task
   */
  bool RunDijsktraMax();

  fixed ScanDistanceMax();

  /**
   * are we close enough to the start so that achieved task speed is unreliable?
   * @return: ratio [0..1].  0 means use average speed only,
   *                         1 means use safety MC equivalent speed only
   */
  fixed GetBlendRatioWhenTooCloseToStartForAverageSpeedCalc() const;

  /**
   * @param achieved_speed
   * @param blend_ratio
   * @param glide polar for safety mc
   * @return: speed blended from SafetyMC equivalent speed and VTaskAchieved
   */
  fixed BlendAchievedTaskSpeedWithSafetyMCSpeed(fixed achieved_speed,
                                                fixed blend_ratio,
                                                const GlidePolar &safety_polar) const;

  /**
   * Updates the TaskStats.task_mc value based on the task planning behaviour
   * of the current task
   * @param glide_polar_task.  The task polar.  The mc setting is irrelevant
   * @param glide_polar_safety.  The safety MC setting.
   */
  void UpdateTaskMC(const GlidePolar &_glide_polar,
                    const GlidePolar &glide_polar_safety) override;

  /**
   * Optimise target ranges (for adjustable tasks) to produce an estimated
   * time remaining with the current glide polar, equal to a target value.
   *
   * @param state_now Aircraft state
   * @param glide_polar Glide polar to be used
   * @param t_target Desired time for remainder of task (s)
   *
   * @return Target range parameter (0-1)
   */
  fixed CalcMinTarget(const AircraftState &state_now,
                      const GlidePolar &glide_polar,
                      const fixed t_target);

  /**
   * Sets previous/next taskpoint pointers for task point at specified
   * index in sequence.
   *
   * @param position Index of task point
   */
  void SetNeighbours(unsigned position);

  /**
   * Erase taskpoint in sequence (for internal use)
   *
   * @param i index of task point in sequence
   */
  void ErasePoint(unsigned i);

  /**
   * Erase optional start point (for internal use)
   *
   * @param i index of optional start point in sequence
   */
  void EraseOptionalStartPoint(unsigned i);

  void UpdateStartTransition(const AircraftState &state,
                             OrderedTaskPoint &start);

  gcc_pure
  bool DistanceIsSignificant(const GeoPoint &location,
                             const GeoPoint &location_last) const;

  gcc_pure
  bool AllowIncrementalBoundaryStats(const AircraftState &state) const;

  /**
   * Calls task_events for appropriate transition events
   * Updates sampled points for tp.
   * Updates state_entered for enter or exit transition
   * Sets has_exited = true if exit transition.
   * @param point
   * @param state_now
   * @param state_last
   * @param bb_now
   * @param bb_last
   * @param transition_enter.  Sets to true if just entered
   * @param transition_exit.  Sets to true if just exited
   * @param last_started.  Sets to false if value was true but we just exited start oz
   * @param is_start.  True if this is the start point
   * @return.  True if sampled points change for tp
   */
  bool CheckTransitionPoint(OrderedTaskPoint &point,
                            const AircraftState &state_now,
                            const AircraftState &state_last,
                            const FlatBoundingBox &bb_now,
                            const FlatBoundingBox &bb_last,
                            bool &transition_enter, bool &transition_exit,
                            bool &last_started,
                            const bool is_start);

  bool CheckTransitionOptionalStart(const AircraftState &state_now,
                                    const AircraftState &state_last,
                                    const FlatBoundingBox& bb_now,
                                    const FlatBoundingBox& bb_last,
                                    bool &transition_enter,
                                    bool &transition_exit,
                                    bool &last_started);

  /**
   * @param waypoints Active waypoint database
   * @param points Vector of points to confirm in active waypoint database
   * @param is_task True if task point.  False if optional start point
   */
  void CheckDuplicateWaypoints(Waypoints& waypoints,
                               OrderedTaskPointVector& points,
                               const bool is_task);

  void SelectOptionalStart(unsigned pos);

public:
  /**
   * Retrieve TaskAdvance mechanism
   *
   * @return Reference to TaskAdvance used by this task
   */
  const TaskAdvance &GetTaskAdvance() const {
    return task_advance;
  }

  /**
   * Retrieve TaskAdvance mechanism
   *
   * @return Reference to TaskAdvance used by this task
   */
  TaskAdvance &SetTaskAdvance() {
    return task_advance;
  }

  /**
   * Retrieve the factory type used by this task
   *
   * @return Factory type
   */
  TaskFactoryType GetFactoryType() const {
    return factory_mode;
  }

  /**
   * Retrieve (const) the #OrderedTaskSettings used by this task
   *
   * @return Read-only #OrderedTaskSettings
   */
  const OrderedTaskSettings &GetOrderedTaskSettings() const {
    return ordered_settings;
  }

  /**
   * Copy #OrderedTaskSettings to this task
   *
   * @param ob Value to set
   */
  void SetOrderedTaskSettings(const OrderedTaskSettings &ob);

protected:
  /**
   * Propagate a change to the #OrderedTaskSettings to all interested
   * child objects.
   */
  void PropagateOrderedTaskSettings();

public:
  ConstTaskPointList GetPoints() const {
    return task_points;
  }

  gcc_pure
  OrderedTaskPoint &GetPoint(const unsigned i) {
    assert(i < task_points.size());
    assert(task_points[i] != nullptr);

    return *task_points[i];
  }

  gcc_pure
  const OrderedTaskPoint &GetPoint(const unsigned i) const {
    assert(i < task_points.size());
    assert(task_points[i] != nullptr);

    return *task_points[i];
  }

  ConstTaskPointList GetOptionalStartPoints() const {
    return optional_start_points;
  }

  /**
   * @return number of optional start poitns
   */
  gcc_pure
  unsigned GetOptionalStartPointCount() const {
    return optional_start_points.size();
  }

  /**
   * returns optional start point
   *
   * @param pos optional start point index
   * @return nullptr if index out of range, else optional start point
   */
  gcc_pure
  const OrderedTaskPoint &GetOptionalStartPoint(unsigned i) const {
    assert(i < optional_start_points.size());

    return *optional_start_points[i];
  }

  /** Determines whether the task has adjustable targets */
  gcc_pure
  bool HasTargets() const;

  /**
   * Find location of center of task (for rendering purposes)
   *
   * @return Location of center of task or GeoPoint::Invalid()
   */
  gcc_pure
  GeoPoint GetTaskCenter() const;

  /**
   * Find approximate radius of task from center to edge (for rendering purposes)
   *
   * @return Radius (m) from center to edge of task
   */
  gcc_pure
  fixed GetTaskRadius() const;

  /**
   * returns the index of the highest intermediate TP that has been entered.
   * if none have been entered, returns zero
   * If start has exited, returns zero
   * Does not consider whether Finish has been achieved
   * @return index of last intermediate point achieved or 0 if none
   */
  unsigned GetLastIntermediateAchieved() const;

  /**
   * Should we add this WP to the Mat
   * after the last achieved Intermediate point?
   * @param mat_wp the wp to test
   * @return true if this should be added after the last achieved intermediate tp
   */
  bool ShouldAddToMat(const Waypoint &mat_wp) const;

  gcc_pure
  const StaticString<64> &GetName() const {
    return name;
  }

  bool GetNameIsBlank() const {
    return name.empty();
  }

  /** name of task, no file extension */
  void SetName(const StaticString<64> &name_) {
    name = name_;
  }
  /** name of task, no file extension */
  void SetName(const TCHAR *name_) {
    name = name_;
  }

  void ClearName() {
    name.clear();
  }

public:
  /* virtual methods from class TaskInterface */
  unsigned TaskSize() const override {
    return task_points.size();
  }

  void SetActiveTaskPoint(unsigned desired) override;
  TaskWaypoint *GetActiveTaskPoint() const override;
  bool IsValidTaskPoint(const int index_offset=0) const override;
  bool UpdateIdle(const AircraftState& state_now,
                  const GlidePolar &glide_polar) override;
  /**
   * Change the time the task was started.
   * If task was already started, do not update aircraft info.
   * If task was not started, update task_started to true
   * and set aircraft information and set the HasEntered() property
   * of the start point
   * @AircraftState used for start stats if none already present
   * @time of start
   */
  void OverrideStartTime(const AircraftState state, fixed time);

  /* virtual methods from class AbstractTask */
  void Reset() override;
  bool TaskStarted(bool soft=false) const override;
  bool CheckTask() const override;

  /**
   * is this type of task optimizable?
   */
  virtual bool IsOptimizable() const override;

protected:
  /* virtual methods from class AbstractTask */
  bool UpdateSample(const AircraftState &state_now,
                    const GlidePolar &glide_polar,
                    const bool full_update) override;

  /**
   * Updates ActiveState for each point (BEFORE/CURRENT/AFTER)
   * Run CheckTransitionPoint for each point and optional start.
   * check "arm" state (XCSoar only)
   * calls task_events->ActiveAdvanced, TaskStart and TaskFinish
   * calls find_best_start to calc correct start for US/FAI tasks
   * sets FAI finish altitudes
   * Updates stats, samples and states for start, intermediate and finish transitions
   */
  bool CheckTransitions(const AircraftState &state_now,
                        const AircraftState &state_last) override;
  bool CalcBestMC(const AircraftState &state_now,
                  const GlidePolar &glide_polar,
                  fixed& best) const override;
  fixed CalcRequiredGlide(const AircraftState &state_now,
                          const GlidePolar &glide_polar) const override;
  bool CalcCruiseEfficiency(const AircraftState &state_now,
                            const GlidePolar &glide_polar,
                            fixed &value) const override;
  bool CalcEffectiveMC(const AircraftState &state_now,
                       const GlidePolar &glide_polar,
                       fixed &value) const override;
  fixed CalcGradient(const AircraftState &state_now) const override;
  fixed ScanTotalStartTime() override;
  fixed ScanLegStartTime() override;
  fixed ScanDistanceNominal() override;
  fixed ScanDistancePlanned() override;
  fixed ScanDistanceRemaining(const GeoPoint &ref) override;
  fixed ScanDistanceScored(const GeoPoint &ref, bool full_update) override;
  fixed ScanDistanceTravelled(const GeoPoint &ref) override;
  void ScanDistanceMinMax(const GeoPoint &ref, bool full,
                          fixed *dmin, fixed *dmax) override;
  void GlideSolutionRemaining(const AircraftState &state_now,
                              const GlidePolar &polar,
                              GlideResult &total, GlideResult &leg) override;
  void GlideSolutionTravelled(const AircraftState &state_now,
                              const GlidePolar &glide_polar,
                              GlideResult &total, GlideResult &leg) override;
  void GlideSolutionPlanned(const AircraftState &state_now,
                            const GlidePolar &glide_polar,
                            GlideResult &total,
                            GlideResult &leg,
                            DistanceStat &total_remaining_effective,
                            DistanceStat &leg_remaining_effective,
                            const GlideResult &solution_remaining_total,
                            const GlideResult &solution_remaining_leg) override;

  void UpdateNavBarStatistics(const AircraftState &aircraft,
                              const GlidePolar &polar) override;

protected:
  bool IsScored() const override;

public:
  void AcceptTaskPointVisitor(TaskPointConstVisitor &visitor) const override;

  /**
   * @param ref.  Location of glider
   * checks whether the aircraft is within 3 miles of the edge of the start
   * cylinder, or inside it
   */
  void UpdateGliderStartCylinderProximity(const GeoPoint &ref) override;
  gcc_pure
  bool CheckGliderStartCylinderProximity() const override {
    return is_glider_close_to_start_cylinder;
  }

};

#endif //ORDEREDTASK_H
