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

#include "Settings.hpp"

void
LoggerSettings::SetDefaults()
{
  time_step_cruise = 5;
  time_step_circling = 1;
  auto_logger = AutoLogger::ON;
  logger_id.clear();
  pilot_name.clear();
  competition_id.clear();
  glider_id.clear();

  /* XXX disabled by default for now, until the FlightLogger
     implementation is finished */
  enable_flight_logger = false;

  enable_nmea_logger = false;
}

const TCHAR*
LoggerSettings::GetCompetitionID() const
{
  if (competition_id.empty())
    return glider_id.c_str();
  else
    return competition_id.c_str();
}

const TCHAR*
LoggerSettings::GetLoggerID() const
{
  if (logger_id.empty()) {
    return pilot_name.c_str();
  } else {
    return logger_id.c_str();
  }
}
