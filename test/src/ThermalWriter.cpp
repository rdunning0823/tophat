/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "ThermalWriter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Util/StaticString.hpp"
#include "IO/TextWriter.hpp"


/**
 * Writer for a "null" value.
 */
static inline void WriteNull(TextWriter &writer) {
  writer.Write("null\t");
}

/**
 * Writer for a integer value.
 */
static inline void WriteInteger(TextWriter &writer, int value) {
  writer.Format("%d\t", value);
}

/**
 * Writer for a integer value.
 */
static inline void WriteUnsigned(TextWriter &writer, unsigned value) {
  writer.Format("%u\t", value);
}

/**
 * Writer for a integer value.
 */
static inline void WriteLong(TextWriter &writer, long value) {
  writer.Format("%ld\t", value);
}

/**
 * Writer for a string value.
 */
static inline void WriteString(TextWriter &writer, const char *value) {
  writer.Format("%s\t", value);
}

static inline void WriteFixed(TextWriter &writer, fixed value) {
  writer.Format("%f\t", (double)value);
}

static inline void WriteAngle(TextWriter &writer, Angle value) {
  WriteFixed(writer, value.Degrees());
}

void
ThermalWriter::WriteHeaderRecord()
{
  WriteString(writer,
              "Start_time\t"
              "lat\t"
              "lon\t"
              "Alt_diff\t"
              "Start_alt\t"
              "End_alt\t"
              "Vario\t"
              "Duration\t"
              "Wind_speed\t"
              "Wind_direction\t"
              "Pilot\t"
              "Glider_type");
  writer.NewLine();
}

void
ThermalWriter::WriteThermalList(const PhaseList &phases,
                                const LoggerSettings &logger_settings,
                                const GliderType &glider_type,
                                bool append)
{
  if (!append)
    WriteHeaderRecord();
  for (Phase phase : phases) {
    if (phase.phase_type == Phase::CIRCLING)
      WriteRecord(phase, logger_settings, glider_type);
  }
}

void
ThermalWriter::WriteRecord(const Phase &phase,
                           const LoggerSettings &logger_settings,
                           const GliderType &glider_type)
{
  NarrowString<64> buffer;

  FormatISO8601(buffer.buffer(), phase.start_datetime);
  WriteString(writer, buffer.c_str());

  WriteAngle(writer, phase.start_loc.latitude);
  WriteAngle(writer, phase.start_loc.longitude);
  WriteInteger(writer, (int)phase.alt_diff);
  WriteInteger(writer, (int)phase.start_alt);
  WriteInteger(writer, (int)phase.end_alt);
  WriteFixed(writer, phase.GetVario());
  WriteInteger(writer, (int)phase.duration);
  WriteLong(writer, (double)0); // wind speed
  WriteAngle(writer, Angle()); // wind speed

  WriteString(writer, logger_settings.pilot_name.c_str());
  WriteString(writer, glider_type.c_str());

  writer.NewLine();

}
