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

#ifndef GOTOTASK_H
#define GOTOTASK_H

#include "UnorderedTask.hpp"
#include "Waypoint/Waypoints.hpp"
#include "Compiler.h"

/**
 * Class providing ability to go to a single task point
 */
class GotoTask : 
  public UnorderedTask 
{
  friend class PrintHelper;

  TaskWaypoint* tp;
  const Waypoints &waypoints;

public:
  /** 
   * Base constructor.
   * 
   * @param tb Global task behaviour settings
   * @param wps Waypoints container to be scanned for takeoff
   * 
   * @return Initialised object (with no waypoint to go to)
   */

  GotoTask(const TaskBehaviour &tb,
           const Waypoints &wps);
  ~GotoTask();

/** 
 * Sets go to task point to specified waypoint. 
 * Obeys TaskBehaviour.goto_nonlandable, won't do anything
 * if destination is not landable.
 * 
 * @param wp Waypoint to Go To
 * @return True if successful
 */
  bool DoGoto(const Waypoint& wp);

  /**
   * When called on takeoff, creates a default goto task
   *
   * @param loc Location of takeoff point
   * @param terrain_alt Terrain height at takeoff point
   *
   * @return True if default task was created
   */
  bool TakeoffAutotask(const GeoPoint& loc, const fixed terrain_alt);

public:
  /* virtual methods from class TaskInterface */
  virtual void SetTaskBehaviour(const TaskBehaviour &tb);
  virtual unsigned TaskSize() const;
  virtual TaskWaypoint *GetActiveTaskPoint() const;
  virtual void SetActiveTaskPoint(unsigned index);
  virtual bool IsValidTaskPoint(const int index_offset) const;

  /* virtual methods from class AbstractTask */
  virtual void Reset();
protected:
  virtual bool UpdateSample(const AircraftState &state_now,
                            const GlidePolar &glide_polar,
                            const bool full_update);
  virtual bool CheckTransitions(const AircraftState &state_now,
                                const AircraftState &state_last);
public:
  virtual void AcceptTaskPointVisitor(TaskPointConstVisitor& visitor) const gcc_override;
};

#endif //GOTOTASK_H
