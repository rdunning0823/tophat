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

#ifndef ORDEREDTASK_H
#define ORDEREDTASK_H

#include "Geo/Flat/TaskProjection.hpp"
#include "Geo/SearchPointVector.hpp"
#include "Task/AbstractTask.hpp"
#include "Task/TaskBehaviour.hpp"
#include "TaskAdvanceSmart.hpp"
#include "MatPoints.hpp"

#include <assert.h>
#include <vector>

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
struct GeoBounds;
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
class OrderedTask:
  public AbstractTask
{
public:
  friend class Serialiser;
  friend class PrintHelper;

  typedef std::vector<OrderedTaskPoint*> OrderedTaskPointVector; /**< Storage type of task points */ 

private:
  OrderedTaskPointVector task_points;
  OrderedTaskPointVector optional_start_points;

  StartPoint *taskpoint_start;
  FinishPoint *taskpoint_finish;

  TaskProjection task_projection;

  GeoPoint last_min_location;

  TaskFactoryType factory_mode;
  AbstractTaskFactory* active_factory;
  OrderedTaskBehaviour ordered_behaviour;
  TaskAdvanceSmart task_advance;
  TaskDijkstraMin *dijkstra_min;
  TaskDijkstraMax *dijkstra_max;

  /**
   * maintains a list of Turnpoints from the waypoint file for MAT tasks
   */
  MatPoints mat_points;

public:
  /**
   * returns const reference to mat_points
   */
  const MatPoints::MatVector& GetMatPoints() const {
    return mat_points.GetMatPoints();
  }

  /**
   * returns reference mat_points
   */
  MatPoints::MatVector& SetMatPoints() {
    return mat_points.SetMatPoints();
  }

  /**
   * this is called automatically by Commit
   * or if the waypoint file changes.
   * If a client wants to use the Mat Points on an Uncommitted task,
   * he must Call FillMatPoints himself
   * The MatPoints will be deleted when the task is deleted
   * It is the client's responsibility to lock the task manager if
   * this is called on the active task
   * @param wps.   Reference to the waypoint file
   * @param update_geometry.  If true (default) will update the task's geometry
   */
  void FillMatPoints(const Waypoints &wps, bool update_geometry = true);

  /**
   * removes all points from mat_points
   */
  void ClearMatPoints();

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
  OrderedTask(const TaskBehaviour &tb);
  virtual ~OrderedTask();

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
   *
   * @return Type of task
   */
  TaskFactoryType SetFactory(const TaskFactoryType _factory);

  /** 
   * Return list of factory types
   * 
   * @param all If true, return all types, otherwise only valid transformable ones
   * 
   * @return Vector of factory types
   */
  gcc_pure
  std::vector<TaskFactoryType> GetFactoryTypes(bool all = true) const;

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
  bool Commit(const OrderedTask& other, const Waypoints &waypoints);

  /**
   * Retrieves the active task point index.
   *
   * @return Index of active task point sequence
   */
  gcc_pure
  unsigned GetActiveIndex() const;

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
    return taskpoint_start != NULL;
  }

  /**
   * Check if task has a single FinishPoint
   *
   * @return True if task has finish
   */
  gcc_pure
  bool HasFinish() const {
    return taskpoint_finish != NULL;
  }

  /**
   * Cycle through optional start points, replacing actual task start point
   * with top item in optional starts.
   */
  void RotateOptionalStarts();

  /**
   * Return number of optional start points
   */
  unsigned OptionalStartsSize() const;

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
  * @return pointer to tp if valid, else NULL
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

  /**
   * Accesses task start state
   *
   * @return State at task start (or null state if not started)
   */
  gcc_pure
  AircraftState GetStartState() const;

  /**
   * Accesses task finish state
   *
   * @return State at task finish (or null state if not finished)
   */
  gcc_pure
  AircraftState GetFinishState() const;

  void CheckDuplicateWaypoints(Waypoints& waypoints);

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

private:

  fixed ScanDistanceMin(const GeoPoint &ref, bool full);
  fixed ScanDistanceMax();

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
  TaskAdvance &GetTaskAdvance() {
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
   * Retrieve (const) the OrderedTaskBehaviour used by this task
   * 
   * @return Read-only OrderedTaskBehaviour
   */
  const OrderedTaskBehaviour &GetOrderedTaskBehaviour() const {
    return ordered_behaviour;
  }

  /** 
   * Retrieve the OrderedTaskBehaviour used by this task
   * 
   * @return Reference to OrderedTaskBehaviour
   */
  OrderedTaskBehaviour &GetOrderedTaskBehaviour() {
    return ordered_behaviour;
  }

  /** 
   * Copy OrderedTaskBehaviour to this task
   * 
   * @param ob Value to set
   */
  void SetOrderedTaskBehaviour(const OrderedTaskBehaviour &ob);

  gcc_pure
  OrderedTaskPoint &GetPoint(const unsigned i) {
    assert(i < task_points.size());
    assert(task_points[i] != NULL);

    return *task_points[i];
  }

  gcc_pure
  const OrderedTaskPoint &GetPoint(const unsigned i) const {
    assert(i < task_points.size());
    assert(task_points[i] != NULL);

    return *task_points[i];
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
   * @return NULL if index out of range, else optional start point
   */
  gcc_pure
  const OrderedTaskPoint &GetOptionalStartPoint(unsigned i) const {
    assert(i < optional_start_points.size());

    return *optional_start_points[i];
  }

public:
  /* virtual methods from class TaskInterface */
  virtual void SetTaskBehaviour(const TaskBehaviour &tb);
  virtual unsigned TaskSize() const;
  virtual void SetActiveTaskPoint(unsigned desired);
  virtual TaskWaypoint *GetActiveTaskPoint() const;
  virtual bool IsValidTaskPoint(const int index_offset=0) const;
  virtual bool UpdateIdle(const AircraftState& state_now,
                          const GlidePolar &glide_polar);

  /* virtual methods from class AbstractTask */
  virtual void Reset();
  virtual bool TaskFinished() const;
  virtual bool TaskStarted(bool soft=false) const;
  virtual bool CheckTask() const;
  virtual fixed GetFinishHeight() const;
  virtual GeoPoint GetTaskCenter(const GeoPoint &fallback_location) const;
  virtual fixed GetTaskRadius(const GeoPoint &fallback_location) const;

  /**
   * returns the index of the highest intermediate TP that has been entered.
   * if none have been entered, returns zero
   * If start has exited, returns zero
   * Does not consider whether Finish has been achieved
   * @return index of last intermediate point achieved or 0 if none
   */
  virtual unsigned GetLastIntermediateAchieved() const;

  /**
   * Should we add this WP to the Mat
   * after the last achieved Intermediate point?
   * @param mat_wp the wp to test
   * @return true if this should be added after the last achieved intermediate tp
   */
  bool ShouldAddToMat(const Waypoint &mat_wp) const;

protected:
  /* virtual methods from class AbstractTask */
  virtual bool UpdateSample(const AircraftState &state_now,
                            const GlidePolar &glide_polar,
                            const bool full_update);

  virtual bool CheckTransitions(const AircraftState &state_now,
                                const AircraftState &state_last);
  virtual bool CalcBestMC(const AircraftState &state_now,
                          const GlidePolar &glide_polar,
                          fixed& best) const;
  virtual fixed CalcRequiredGlide(const AircraftState &state_now,
                                  const GlidePolar &glide_polar) const;
  virtual bool CalcCruiseEfficiency(const AircraftState &state_now,
                                    const GlidePolar &glide_polar,
                                    fixed &value) const;
  virtual bool CalcEffectiveMC(const AircraftState &state_now,
                               const GlidePolar &glide_polar,
                               fixed &value) const;
  virtual fixed CalcMinTarget(const AircraftState &state_now,
                              const GlidePolar &glide_polar,
                              const fixed t_target);
  virtual fixed CalcGradient(const AircraftState &state_now) const;
  virtual fixed ScanTotalStartTime(const AircraftState &state_now);
  virtual fixed ScanLegStartTime(const AircraftState &state_now);
  virtual fixed ScanDistanceNominal();
  virtual fixed ScanDistancePlanned();
  virtual fixed ScanDistanceRemaining(const GeoPoint &ref);
  virtual fixed ScanDistanceScored(const GeoPoint &ref);
  virtual fixed ScanDistanceTravelled(const GeoPoint &ref);
  virtual void ScanDistanceMinMax(const GeoPoint &ref, bool full,
                                  fixed *dmin, fixed *dmax);
  virtual void GlideSolutionRemaining(const AircraftState &state_now,
                                      const GlidePolar &polar,
                                      GlideResult &total, GlideResult &leg);
  virtual void GlideSolutionTravelled(const AircraftState &state_now,
                                      const GlidePolar &glide_polar,
                                      GlideResult &total, GlideResult &leg);
  virtual void GlideSolutionPlanned(const AircraftState &state_now,
                                    const GlidePolar &glide_polar,
                                    GlideResult &total,
                                    GlideResult &leg,
                                    DistanceStat &total_remaining_effective,
                                    DistanceStat &leg_remaining_effective,
                                    const GlideResult &solution_remaining_total,
                                    const GlideResult &solution_remaining_leg);
public:
  virtual bool HasTargets() const;
  /**
   * is this type of task optimizable?
   */
  virtual bool IsOptimizable() const;
protected:
  virtual bool IsScored() const;
public:
  virtual void AcceptTaskPointVisitor(TaskPointConstVisitor &visitor) const gcc_override;
  virtual void AcceptStartPointVisitor(TaskPointConstVisitor &visitor) const gcc_override;
};

#endif //ORDEREDTASK_H
