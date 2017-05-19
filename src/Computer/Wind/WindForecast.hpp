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

#ifndef XCSOAR_WIND_FORECAST_HPP
#define XCSOAR_WIND_FORECAST_HPP

#include "Net/HTTP/DownloadManager.hpp"
#include "Thread/StandbyThread.hpp"
#include "Geo/SpeedVector.hpp"
#include "Geo/GeoPoint.hpp"
#include "Time/BrokenDateTime.hpp"
#include <string>
#include <vector>

struct MoreData;
struct DerivedInfo;

class WindForecast : protected StandbyThread {

  struct Data {
    fixed pres;
    int alt;
    fixed temp;
    fixed dewpt;
    int wdir;
    int wspd;
  };

  std::vector<Data> data;
  bool first_run = true;
  GeoPoint last_position;
  BrokenDateTime last_time;
  GeoPoint last_update_position;
  BrokenDateTime last_update_time;

  SpeedVector LinearApprox(fixed altitude, const Data *prev, const Data *next);
  bool ReadLine(const std::string& line, Data &data);
  bool NeedUpdate();


public:
  struct Result
  {
    unsigned quality;
    SpeedVector wind;

    Result() {}
    Result(unsigned _quality):quality(_quality) {}
    Result(unsigned _quality, SpeedVector _wind)
      :quality(_quality), wind(_wind) {}

    bool IsValid() const {
      return quality > 0;
    }
  };

  WindForecast():StandbyThread("WindForecast") {};

  void Reset();
  Result Update(const MoreData &basic, const DerivedInfo &derived);

protected:
  void Tick() override;
};

#endif
