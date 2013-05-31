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


#ifndef WAYPOINT_READER_HPP
#define WAYPOINT_READER_HPP

#include "WaypointReaderBase.hpp"

#include <algorithm>
#include <tchar.h>
#include <windef.h> /* for MAX_PATH */

class Waypoints;
class RasterTerrain;
class OperationEnvironment;

class WaypointReader
{
  /** The internal reader implementation depending on the file format */
  WaypointReaderBase* reader;

  TCHAR path[MAX_PATH];

public:
  /** Non-initializing constructor */
  WaypointReader():reader(NULL) {}

  /** Initializing constructor. Loads the specified waypoint file */
  WaypointReader(const TCHAR* filename, int filenum = 0)
    :reader(NULL) {
    Open(filename, filenum);
  }

  WaypointReader(const WaypointReader &other) = delete;

  WaypointReader(WaypointReader &&other)
    :reader(other.reader) {
    other.reader = NULL;
  }

  /** Destroys the internal reader */
  ~WaypointReader() {
    delete reader;
  }

  WaypointReader &operator=(const WaypointReader &other) = delete;

  WaypointReader &operator=(WaypointReader &&other) {
    std::swap(reader, other.reader);
    return *this;
  }

  /**
   * Opens the given file, tries to guess the file format and
   * initializes the internal reader
   * @param filename The file that should be opened
   * @param filenum The filenum parameter that is saved into the parsed waypoints
   */
  void Open(const TCHAR* filename, int filenum = 0);

  /** Sets the terrain that should be used for waypoint elevation detection */
  void SetTerrain(const RasterTerrain* _terrain);

  /**
   * Parses the waypoint file into the given Waypoints instance
   * @param way_points A Waypoints instance that will hold the parsed waypoints
   * @return True if the file was parsed successfully
   */
  bool Parse(Waypoints &way_points, OperationEnvironment &operation);

  /**
   * Returns whether there is a valid internal reader
   * that can be used for parsing the waypoint file.
   * @return True if Parse() can be used
   */
  bool Error() const {
    return reader == NULL;
  }
};

#endif
