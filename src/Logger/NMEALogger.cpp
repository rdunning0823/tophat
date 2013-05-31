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

#include "Logger/NMEALogger.hpp"
#include "IO/BatchTextWriter.hpp"
#include "LocalPath.hpp"
#include "DateTime.hpp"
#include "Thread/Mutex.hpp"
#include "OS/FileUtil.hpp"
#include "Util/StaticString.hpp"

#include <windef.h> // for MAX_PATH
#include <stdio.h>

namespace NMEALogger
{
  Mutex mutex;
  BatchTextWriter *writer;

  bool enabled = false;

  bool Start();
}

bool
NMEALogger::Start()
{
  if (writer != NULL)
    return true;

  BrokenDateTime dt = BrokenDateTime::NowUTC();
  assert(dt.Plausible());

  StaticString<64> name;
  name.Format(_T("%04u-%02u-%02u_%02u-%02u.nmea"),
              dt.year, dt.month, dt.day,
              dt.hour, dt.minute);

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("logs"));
  Directory::Create(path);

  LocalPath(path, _T("logs"), name);

  writer = new BatchTextWriter(path, false);
  return writer != NULL;
}

void
NMEALogger::Shutdown()
{
  delete writer;
}

void
NMEALogger::Log(const char *text)
{
  if (!enabled)
    return;

  ScopeLock protect(mutex);
  if (Start())
    writer->WriteLine(text);
}
