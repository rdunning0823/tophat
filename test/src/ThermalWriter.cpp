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
#include "Util/StaticString.hxx"
#include "IO/TextWriter.hpp"


NarrowString<2> delimiter;
/**
 * Writer for a "null" value.
 */
static inline void WriteNull(TextWriter &writer) {
  writer.Write("null");
  writer.Write(delimiter.c_str());
}

/**
 * Writer for a integer value.
 */
static inline void WriteInteger(TextWriter &writer, int value) {
  writer.Format("%d", value);
  writer.Write(delimiter.c_str());
}

/**
 * Writer for a integer value.
 */
static inline void WriteUnsigned(TextWriter &writer, unsigned value) {
  writer.Format("%u", value);
  writer.Write(delimiter.c_str());
}

/**
 * Writer for a integer value.
 */
static inline void WriteLong(TextWriter &writer, long value) {
  writer.Format("%ld", value);
  writer.Write(delimiter.c_str());
}

/**
 * Writer for a string value.
 */
static inline void WriteString(TextWriter &writer, const char *value) {
  writer.Format("%s", value);
  writer.Write(delimiter.c_str());
}

static inline void WriteFixed(TextWriter &writer, fixed value) {
  writer.Format("%f", (double)value);
  writer.Write(delimiter.c_str());
}

static inline void WriteAngle(TextWriter &writer, Angle value) {
  WriteFixed(writer, value.Degrees());
}

static inline void WriteUSLatitude(TextWriter &writer, Angle value) {
  writer.Format("%.3fN", (double)(value.Degrees() * fixed(100)));
  writer.Write(delimiter.c_str());
}
static inline void WriteUSLongitude(TextWriter &writer, Angle value) {
  writer.Format("0%.3fW", (double)(value.Degrees() * fixed(-100)));
  writer.Write(delimiter.c_str());
}

void
ThermalWriter::WriteHeaderRecord(bool cup_file)
{
  if (cup_file) {
    //name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc,userdata,pics
    //"44 (2.8) moderate thermal",44(2.mal,,4229.949N,07749.351W,528.2m,11,,,,,,
    WriteString(writer,"name");
    WriteString(writer,"code");
    WriteString(writer,"country");
    WriteString(writer,"lat");
    WriteString(writer,"lon");
    WriteString(writer,"elev");
    WriteString(writer,"style");
    WriteString(writer,"rwdir");
    WriteString(writer,"rwlen");
    WriteString(writer,"freq");
    WriteString(writer,"desc");
    WriteString(writer,"userdata");
    WriteString(writer,"pics");

  } else {
    WriteString(writer,"Start_time");
    WriteString(writer,"lat");
    WriteString(writer,"lon");
    WriteString(writer,"Alt_diff");
    WriteString(writer,"Start_alt");
    WriteString(writer,"End_alt");
    WriteString(writer,"Vario");
    WriteString(writer,"Duration");
    WriteString(writer,"Wind_speed");
    WriteString(writer,"Wind_direction");
    WriteString(writer,"Pilot");
    WriteString(writer,"Glider_type");
  }
  writer.NewLine();
}

void
ThermalWriter::WriteThermalList(const PhaseList &phases,
                                const LoggerSettings &logger_settings,
                                const GliderType &glider_type,
                                bool append, bool cup_file)
{
  delimiter = (cup_file) ? "," : "\t";
  if (!append)
    WriteHeaderRecord(cup_file);
  for (Phase phase : phases) {
    if (phase.phase_type == Phase::CIRCLING)
      WriteRecord(phase, logger_settings, glider_type, cup_file);
  }
}

void
ThermalWriter::WriteRecord(const Phase &phase,
                           const LoggerSettings &logger_settings,
                           const GliderType &glider_type, bool cup_file)
{
  NarrowString<64> buffer;

  if (!positive(phase.alt_diff) || phase.GetVario() < fixed(1))
    return;

  if (cup_file) {
    buffer.Format("%s %.1f", logger_settings.GetCompetitionID(),
                  (double)(phase.GetVario() * phase.alt_diff / fixed(300)));
    WriteString(writer,buffer.c_str());
    buffer.Format("v%.1f", (double)phase.GetVario());
    WriteString(writer,buffer.c_str());
    WriteString(writer,""); //country
    WriteUSLatitude(writer, phase.start_loc.latitude);
    WriteUSLongitude(writer, phase.start_loc.longitude);
    WriteInteger(writer, (int)phase.alt_diff); // elev
    WriteInteger(writer, 11);
    WriteString(writer,"");
    WriteString(writer,"");
    WriteString(writer,"");
    buffer.Format("wind %.0f / %.0fkts",
                  (double)phase.wind.bearing.AbsoluteDegrees(),
                  (double)(phase.wind.norm * fixed(1.94384)));
    WriteString(writer,buffer.c_str());
    WriteString(writer,"");
    WriteString(writer,"");

  } else {
    FormatISO8601(buffer.buffer(), phase.start_datetime);
    WriteString(writer, buffer.c_str());

    WriteAngle(writer, phase.start_loc.latitude);
    WriteAngle(writer, phase.start_loc.longitude);
    WriteInteger(writer, (int)phase.alt_diff);
    WriteInteger(writer, (int)phase.start_alt);
    WriteInteger(writer, (int)phase.end_alt);
    WriteFixed(writer, phase.GetVario());
    WriteInteger(writer, (int)phase.duration);
    WriteInteger(writer, (int)(phase.wind.norm * fixed(1.94384))); // knots
    WriteInteger(writer, (int)phase.wind.bearing.AbsoluteDegrees());

    WriteString(writer, logger_settings.pilot_name.c_str());
    WriteString(writer, glider_type.c_str());
  }
  writer.NewLine();

}
