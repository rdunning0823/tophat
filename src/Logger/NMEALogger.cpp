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

#include "Logger/NMEALogger.hpp"
#include "IO/TextWriter.hpp"
#include "LocalPath.hpp"
#include "Time/BrokenDateTime.hpp"
#include "Thread/Mutex.hpp"
#include "OS/FileUtil.hpp"
#include "Util/StaticString.hxx"

#include <windef.h> // for MAX_PATH
#include <stdio.h>

namespace NMEALogger
{
  static Mutex mutex;
  static TextWriter *writer;

  bool enabled = false;

  static bool Start(const BrokenDateTime &initial_gps_time);
}

bool
NMEALogger::Start(const BrokenDateTime &dt)
{
  if (writer != nullptr)
    return true;

  assert(dt.IsPlausible());

  StaticString<64> name;
  name.Format(_T("%04u-%02u-%02u_%02u-%02u.nmea"),
              dt.year, dt.month, dt.day,
              dt.hour, dt.minute);

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("logs"));
  Directory::Create(path);

  LocalPath(path, _T("logs"), name);

  writer = new TextWriter(path, false);
  return writer != nullptr;
}

void
NMEALogger::Shutdown()
{
  delete writer;
}

void
NMEALogger::Log(const char *text, const BrokenDateTime &initial_gps_time)
{
  if (!enabled)
    return;

  ScopeLock protect(mutex);
  if (Start(initial_gps_time))
    writer->WriteLine(text);
}
