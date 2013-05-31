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
#ifndef UNORDEREDTASK_H
#define UNORDEREDTASK_H

#include "Task/AbstractTask.hpp"

/**
 *  Common class for all unordered task types
 */
class UnorderedTask: 
  public AbstractTask
{
public:
  /** 
   * Base constructor.
   * 
   * @param tb Global task behaviour settings
   * 
   */
  UnorderedTask(const enum Type _type,
                const TaskBehaviour &tb);

public:
  /* virtual methods from class AbstractTask */
  virtual bool CheckTask() const;
  virtual fixed GetFinishHeight() const;
  virtual GeoPoint GetTaskCenter(const GeoPoint &fallback_location) const;
  virtual fixed GetTaskRadius(const GeoPoint &fallback_location) const;
  virtual bool CalcBestMC(const AircraftState &state_now,
                          const GlidePolar &glide_polar,
                          fixed& best) const;
  virtual fixed CalcRequiredGlide(const AircraftState &state_now,
                                  const GlidePolar &glide_polar) const;
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
  virtual bool HasTargets() const { return false; }
  virtual bool IsOptimizable() const { return false; }
  virtual bool IsScored() const { return false; }
  virtual void AcceptStartPointVisitor(TaskPointConstVisitor& visitor) const gcc_override;
};

#endif
