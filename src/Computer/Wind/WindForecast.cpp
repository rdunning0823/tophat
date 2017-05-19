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

#include "WindForecast.hpp"
#include "NMEA/Derived.hpp"
#include "NMEA/MoreData.hpp"
#include "LogFile.hpp"
#include "Net/HTTP/Session.hpp"
#include "Net/HTTP/ToBuffer.hpp"
#include "Net/HTTP/Request.hpp"
#include "Util/StaticString.hxx"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Units/System.hpp"

#define WIND_FC_QUALITY 5
// how far should data be updated
#define WIND_FC_UPDATE_DISTANCE_NM 5
// how often should data be updated
#define WIND_FC_UPDATE_INTERVAL_MIN 60
// how long should it wait before retrying on fail
#define WIND_FC_RETRY_MIN 1

/**
 * Parses a single line of the sounding
 */
bool
WindForecast::ReadLine(const std::string& line, Data &data)
{
  int num(0);
  std::istringstream strm(line);
  if (!(strm >> num)) {
    return false;
  }
  // we're interested only in lines starting from numbers between 4 and 9
  if (4 <= num && num <= 9) {
    int pres, alt, temp, dewpt, wdir, wspd;
    strm >> pres >> alt >> temp >> dewpt >> wdir >> wspd;
    if (alt == 99999 || wdir == 99999 || wspd == 99999) {
      // no data
      return false;
    }
    data.pres = fixed(pres / 10.0);
    data.alt = alt;
    data.temp = fixed(temp / 10.0);
    data.dewpt = fixed(dewpt / 10.0);
    data.wspd = wspd;
    data.wdir = wdir;
    return true;
  } else {
    return false;
  }
}

void
WindForecast::Tick()
{
  mutex.Unlock();

  last_update_position = last_position;
  last_update_time = last_time;
  first_run = false;
  data.clear();

  // Build file url
  NarrowString<1024> url;
  url.Format(
      "https://rucsoundings.noaa.gov/get_soundings.cgi?data_source=Op40&airport=%f,%f&start=latest",
      (double)last_position.latitude.Degrees(),
      (double)last_position.longitude.Degrees());

  LogFormat(_T("Updating winds aloft forecast from %s"), url.c_str());

  // Open download session
  Net::Session session;
  if (session.Error()) {
    mutex.Lock();
    return;
  }

  // 10kb should be enough
  char buffer[1024 * 10];
  Net::Request request(session, url, 3000);
  if (!request.Send(10000)) {
    mutex.Lock();
    return;
  }

  ssize_t size = request.Read(buffer, sizeof(buffer), 10000);
  if (size <= 0) {
    mutex.Lock();
    return;
  }

  buffer[size] = 0;

  // Format description: https://rucsoundings.noaa.gov/raob_format.html
  std::stringstream strm(buffer);
  std::string line;
  while (getline(strm, line)) {
    WindForecast::Data row;
    if (WindForecast::ReadLine(line, row)) {
      data.push_back(row);
    }
  }

  mutex.Lock();
}

void
WindForecast::Reset()
{
  data.clear();
  first_run = true;
}

WindForecast::Result
WindForecast::Update(const MoreData &basic, const DerivedInfo &derived)
{
  if (!basic.NavAltitudeAvailable() || !basic.location_available) {
    return WindForecast::Result(0);
  }

  ScopeLock protect(mutex);

  if (IsBusy())
    /* still running, skip this submission */
    return WindForecast::Result(0);

  last_position = basic.location;
  last_time = basic.date_time_utc;

  if (NeedUpdate()) {
    Trigger();
    return WindForecast::Result(0);
  }

  WindForecast::Result result(0);

  Data *previous = NULL;
  for (unsigned int i = 0; i < data.size(); i++) {
    // data is sorted from lowest to highest altitudes
    Data *row = &data[i];
    if (Units::ToSysUnit(fixed(row->alt), Unit::METER) > basic.nav_altitude) {
      // pick first altitude which is higher than current
      SpeedVector wind = WindForecast::LinearApprox(basic.nav_altitude,
                                                    previous, row);
      return WindForecast::Result(WIND_FC_QUALITY, wind);
    } else {
      previous = row;
    }
  }

  return WindForecast::Result(0);
}

/**
 * Determines if update is needed
 */
bool
WindForecast::NeedUpdate()
{
  if (first_run)
    return true;
  if (data.empty() && (last_time - last_update_time) / 60 > WIND_FC_RETRY_MIN)
    return true;
  if (last_position.Distance(last_update_position)
      > Units::ToSysUnit(fixed(WIND_FC_UPDATE_DISTANCE_NM),
                         Unit::NAUTICAL_MILES))
    return true;
  if ((last_time - last_update_time) / 60 > WIND_FC_UPDATE_INTERVAL_MIN)
    return true;
  return false;
}

/**
 * Approximates wind between two altitudes
 */
SpeedVector
WindForecast::LinearApprox(fixed altitude, const WindForecast::Data *prev,
                           const WindForecast::Data *next)
{
  assert(next != NULL);

  if (prev == NULL) {
    return SpeedVector(Angle::Degrees(next->wdir),
                       Units::ToSysUnit(fixed(next->wspd), Unit::KNOTS));
  }
  // 0.0 <= factor < 1.0
  fixed factor = (altitude - fixed(prev->alt)) / fixed(next->alt - prev->alt);
  fixed speed = fixed(prev->wspd) + fixed(next->wspd - prev->wspd) * factor;
  return SpeedVector(
      Angle::Degrees(prev->wdir).Fraction(Angle::Degrees(next->wdir), factor),
      Units::ToSysUnit(speed, Unit::KNOTS));
}
