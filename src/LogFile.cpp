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

#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "Asset.hpp"
#include "IO/TextWriter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "DateTime.hpp"
#include "Util/StaticString.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <windef.h> // for MAX_PATH

#ifdef ANDROID
#include <android/log.h>
#endif

static TextWriter
OpenLog()
{
  static bool initialised = false;
  static TCHAR path[MAX_PATH];

  const bool append = initialised;
  if (!initialised) {
    initialised = true;
    LocalPath(path, _T("tophat-startup.log"));
  }

  return TextWriter(path, append);
}

static void
LogString(const char *p)
{
#ifdef ANDROID
  __android_log_print(ANDROID_LOG_INFO, "TopHat", "%s", p);
#elif defined(HAVE_POSIX) && !defined(NDEBUG)
  fprintf(stderr, "%s\n", p);
#endif

  TextWriter writer(OpenLog());
  if (!writer.IsOpen())
    return;

  char time_buffer[32];
  FormatISO8601(time_buffer, BrokenDateTime::NowUTC());
  writer.FormatLine("[%s] %s", time_buffer, p);
}

void
LogFormat(const char *fmt, ...)
{
  char buf[MAX_PATH];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);

  LogString(buf);
}

#ifdef _UNICODE

static void
LogString(const TCHAR *p)
{
  TextWriter writer(OpenLog());
  if (!writer.IsOpen())
    return;

  TCHAR time_buffer[32];
  FormatISO8601(time_buffer, BrokenDateTime::NowUTC());
  writer.FormatLine(_T("[%s] %s"), time_buffer, p);
}

void
LogFormat(const TCHAR *Str, ...)
{
  TCHAR buf[MAX_PATH];
  va_list ap;

  va_start(ap, Str);
  _vstprintf(buf, Str, ap);
  va_end(ap);

  LogString(buf);
}

#endif
