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


#ifndef WAYPOINTFILESEEYOU_HPP
#define WAYPOINTFILESEEYOU_HPP

#include "WaypointReaderBase.hpp"

/**
 * Parses a SeeYou waypoint file.
 *
 * @see http://data.naviter.si/docs/cup_format.pdf
 */
class WaypointReaderSeeYou final : public WaypointReaderBase {
  bool first;
  bool pre_parse_first;

  bool ignore_following;
  bool pre_parse_ignore_following;
  /** do we flag all waypoints as turnpoints? */
  bool all_waypoints_are_turnpoints;

public:
  explicit WaypointReaderSeeYou(WaypointFactory _factory)
    :WaypointReaderBase(_factory),
     first(true), pre_parse_first(true),
     ignore_following(false), pre_parse_ignore_following(false),
     all_waypoints_are_turnpoints(true) {}

protected:
  /* virtual methods from class WaypointReaderBase */
  bool ParseLine(const TCHAR* line, Waypoints &way_points) override;

  /**
   * scans line description for "Turn Point"
   * @return True if text is found, else false
   */
  bool PreParseLine(const TCHAR* line, Waypoints &way_points);

  /**
   * detects whether Description field contains "Turn point" for any waypoints.
   * Sets all_waypoints_are_turnpoints = false if text is found
   */
  virtual void PreParse(Waypoints &way_points, TLineReader &reader,
                        OperationEnvironment &operation) override;
};

#endif
