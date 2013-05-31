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

#ifndef OS_PATH_HPP
#define OS_PATH_HPP

#include "Util/Macros.hpp"
#include "Util/ConvertString.hpp"
#include "Compiler.h"

#ifdef _UNICODE
#include "Util/ConvertString.hpp"
#endif

#include <tchar.h>

/**
 * Representation of a file name.  It is automatically converted to
 * the file system character set.  If no conversion is needed, then
 * this object will hold a pointer to the original input string; it
 * must not be Invalidated.
 */
class PathName {
#ifdef _UNICODE
  TCHAR *allocated;
#endif
  const TCHAR *value;

public:
#ifdef _UNICODE
  explicit PathName(const TCHAR *_value)
    :allocated(NULL), value(_value) {}

  explicit PathName(const char *_value)
    :allocated(ConvertACPToWide(_value)), value(allocated) {}

  ~PathName() {
    delete[] allocated;
  }
#else /* !_UNICODE */
  explicit PathName(const TCHAR *_value):value(_value) {}
#endif /* !_UNICODE */

public:
  bool IsDefined() const {
#ifdef _UNICODE
    return value != NULL;
#else
    return true;
#endif
  }

  operator const TCHAR *() const {
    return value;
  }
};

/**
 * Representation of a file name in narrow characters.  If no
 * conversion is needed, then this object will hold a pointer to the
 * original input string; it must not be Invalidated.
 */
class NarrowPathName {
#ifdef _UNICODE
  char *allocated;
#endif
  const char *value;

public:
#ifdef _UNICODE
  explicit NarrowPathName(const char *_value)
    :allocated(NULL), value(_value) {}

  explicit NarrowPathName(const TCHAR *_value)
    :allocated(ConvertWideToACP(_value)), value(allocated) {}

  ~NarrowPathName() {
    delete[] allocated;
  }
#else /* !_UNICODE */
  explicit NarrowPathName(const char *_value):value(_value) {}
#endif /* !_UNICODE */

public:
  bool IsDefined() const {
#ifdef _UNICODE
    return value != NULL;
#else
    return true;
#endif
  }

  operator const char *() const {
    return value;
  }
};

/**
 * Is this path a "base name", i.e. is there no path separate?
 * Behaviour is undefined when the string is empty.
 */
gcc_pure
bool
IsBaseName(const TCHAR *path);

/**
 * Returns the base name of the specified path, i.e. the part after
 * the last separator.  May return NULL if there is no base name.
 */
gcc_pure
const TCHAR *
BaseName(const TCHAR *path);

/**
 * Returns the directory name of the specified path, i.e. the part
 * before the last separator.  Returns "." if there is no directory
 * name.
 */
const TCHAR *
DirName(const TCHAR *path, TCHAR *buffer);

/**
 * Replaces the "base name" of the specified path, i.e. the portion
 * after the last path separator.  If the input path does not contain
 * a directory name, the whole string is replaced.
 *
 * @param the input and output buffer
 * @param new_base the new base name to be copied to #path
 */
void
ReplaceBaseName(TCHAR *path, const TCHAR *new_base);

/**
 * Checks whether the given filename matches the given extension
 * @param filename Filename to check
 * @param extension Extension to check against (e.g. .xcm)
 * @return True if filename matches the given extension, False otherwise
 */
gcc_pure
bool MatchesExtension(const TCHAR *filename, const TCHAR *extension);

#ifdef _UNICODE

/**
 * Checks whether the given filename matches the given extension
 * @param filename Filename to check
 * @param extension Extension to check against (e.g. .xcm)
 * @return True if filename matches the given extension, False otherwise
 */
gcc_pure
bool MatchesExtension(const char *filename, const char *extension);

#endif

#endif
