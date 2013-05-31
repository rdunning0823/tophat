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

#ifndef XCSOAR_IO_LINE_SOURCE_HPP
#define XCSOAR_IO_LINE_SOURCE_HPP

#include "LineReader.hpp"
#include "Source.hpp"
#include "Util/ReusableArray.hpp"

#include <utility>

/**
 * An adapter for a Source object which reads line-by-line.  It is
 * limited to narrow characters, but may work with multi-byte
 * characters.  It assumes that lines are delimited by a linefeed
 * character, and deletes carriage returns.
 *
 * The maximum length of a line is limited by the buffer size of the
 * Source object.
 */
class LineSplitter : public NLineReader {
protected:
  Source<char> &source;

  /**
   * The remaining range of the buffer returned by the Source.
   */
  Source<char>::Range remaining;

  /** a buffer for guaranteeing that the last line is
      null-terminated */
  ReusableArray<char> last;

public:
  LineSplitter(Source<char> &_source)
    :source(_source), remaining((char *)NULL, 0) {}

  /**
   * Discards the buffer that holds the data after the last line.
   * This can be used to implement Source::Rewind(): after rewinding
   * the Source object, the LineSplitter must forget the buffer.
   */
  void ResetBuffer() {
    remaining.length = 0;
  }

  virtual char *read();
  virtual long size() const;
  virtual long tell() const;
};

#endif
