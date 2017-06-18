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

#ifndef XCSOAR_LOG_FILE_HPP
#define XCSOAR_LOG_FILE_HPP

#include "Compiler.h"

#ifdef _UNICODE
#include <tchar.h>
#endif
  /** should we skip debugging entries */
extern bool disable_debug_logging;

/**
 * Write a formatted line to the log file.
 *
 * @param fmt the format string, which must not contain newline or
 * carriage return characters
 */
gcc_printf(1, 2)
void
LogFormat(const char *fmt, ...);

//void LogDebugEnable(bool val);
//bool GetLogDebugEnabled();
static inline bool
GetLogDebugEnabled()
{
  return !disable_debug_logging;
}

/**
 * @return: true if it was enabled before.  else false.
 */
static inline bool
LogDebugEnable(bool val)
{
  bool temp = disable_debug_logging;
  disable_debug_logging = !val;
  return !temp;
}

#ifdef _UNICODE
void
LogFormat(const TCHAR *fmt, ...);
#endif

#if !defined(NDEBUG) && !defined(GNAV)

#define LogDebug(...) if (!disable_debug_logging) LogFormat(__VA_ARGS__)
//#define LogDebugEnable(val) disable_debug_logging = !val

#else /* NDEBUG */

/* not using an empty inline function here because we don't want to
   evaluate the parameters */
#define LogDebug(...)

#endif /* NDEBUG */

#endif
