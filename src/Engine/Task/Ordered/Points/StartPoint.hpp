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


#ifndef STARTPOINT_HPP
#define STARTPOINT_HPP

#include "OrderedTaskPoint.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Task/Ordered/StartConstraints.hpp"

/**
 * A StartPoint is an abstract OrderedTaskPoint,
 * can manage start transitions
 * but does not yet have an observation zone.
 * No taskpoints shall be present preceding a StartPoint.
 *
 * \todo
 * - gate start time?
 * - enabled/disabled for multiple start points
 */
class StartPoint final : public OrderedTaskPoint {
  fixed safety_height;

  TaskStartMargins margins;

  /**
   * A copy of OrderedTaskSettings::start_constraints, managed by
   * SetOrderedTaskSettings().
   */
  StartConstraints constraints;

public:
  /**
   * Constructor.  Sets task area to non-scorable; distances
   * are relative to crossing point or origin.
   *
   * @param _oz Observation zone for this task point
   * @param wp Waypoint origin of turnpoint
   * @param tb Task Behaviour defining options (esp safety heights)
   * @param to OrderedTask Behaviour defining options
   * @param is the start scored by the real location of the boundary exit
   *
   * @return Partially-initialised object
   */
  StartPoint(ObservationZonePoint *_oz,
             const Waypoint &wp,
             const TaskBehaviour &tb,
             const StartConstraints &constraints,
             bool boundary_scored = false);

  bool DoesRequireArm() const {
    return constraints.require_arm;
  }

  /**
   * Manually sets the start time
   */
  void SetHasExitedOverride () {
    ScoredTaskPoint::SetHasExited(true);
  }

  /**
   * Find correct start point for either FAI or US contest
   *   - for US, use actual start location (or center of cylinder if actual
   *     start is in back half)
   *   - for FAI use closest point on boundary of cylinder
   * Updates stats, samples and states for start, intermediate and finish transitions
   * Should only be performed when the aircraft state is inside the sector
   * Also saves subtract_start_radius as member property
   *
   * @param state Current aircraft state
   * @param next Next task point following the start
   * @param
   */
  void find_best_start(const AircraftState &state,
                       const OrderedTaskPoint &next,
                       const FlatProjection &projection);

  /* virtual methods from class TaskPoint */
  fixed GetElevation() const override;
  fixed GetRequiredElevation(fixed safety_limit) const override;


  /* virtual methods from class ScoredTaskPoint */
  bool CheckExitTransition(const AircraftState &ref_now,
                           const AircraftState &ref_last) const override;

  /* virtual methods from class OrderedTaskPoint */
  void SetTaskBehaviour(const TaskBehaviour &tb) override;
  void SetOrderedTaskSettings(const OrderedTaskSettings &s) override;
  void SetNeighbours(OrderedTaskPoint *prev,
                     OrderedTaskPoint *next) override;
  bool IsInSector(const AircraftState &ref) const override;

  /* virtual methods from class ObservationZoneClient */
  fixed ScoreAdjustment() const override;

private:
  /* virtual methods from class ScoredTaskPoint */
  bool ScoreLastExit() const override {
    return true;
  }
};

#endif
