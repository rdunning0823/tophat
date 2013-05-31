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

#ifndef XCSOAR_MARKS_HPP
#define XCSOAR_MARKS_HPP

#include "Geo/GeoPoint.hpp"
#include "DateTime.hpp"

#include <vector>

class WindowProjection;
class Canvas;
struct MarkerLook;

class Markers
{
public:
  struct Marker {
    GeoPoint location;
    BrokenDateTime time;
  };

  typedef std::vector<Marker>::const_iterator const_iterator;

private:
  std::vector<Marker> marker_store;

public:
  Markers();
  ~Markers();

  void Reset();
  void Draw(Canvas &canvas, const WindowProjection &projection,
            const MarkerLook &look) const;
  void MarkLocation(const GeoPoint &loc, const BrokenDateTime &time);

  const_iterator begin() const {
    return marker_store.begin();
  }
  const_iterator end() const {
    return marker_store.end();
  }
};

#endif
