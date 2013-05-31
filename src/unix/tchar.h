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

#ifndef TCHAR_H
#define TCHAR_H

#ifdef LIBCXX
/* libc++ uses "_T" as template argument names; this conflicts with
   the macro "_T()" defined below.  To work around that problem,
   include all relevant libc++ headers before defining _T() */
#include <iterator>
#endif

#include <string.h>
#include <stdarg.h>

#include <ctype.h>

typedef char TCHAR;
#define _stprintf sprintf
#define _vstprintf vsprintf
#define _sntprintf snprintf
#define _vsntprintf vsnprintf
#define _tprintf printf
#define _ftprintf fprintf
#define _vftprintf vfprintf
#define _fputts fputs
#define _tcsdup strdup
#define _tcscpy strcpy
#define _tcscmp strcmp
#define _tcsncmp strncmp
#define _tcslen strlen
#define _tcsclen strlen
#define _tcsstr strstr
#define _tcschr strchr
#define _tcsrchr strrchr
#define _tcspbrk strpbrk
#define _tcscat strcat
#define _tcsncat strncat
#define _T(x) x
#define _tfopen fopen
#define _TEOF EOF
#define _fgetts fgets
#define _putts puts
#define _stscanf sscanf

#define _tcstok strtok
#define _totupper toupper
#define _tcstol strtol
#define _tcstod strtod

#define _istalnum isalnum
#define _istpunct ispunct

#define _tcsupr CharUpper

static inline TCHAR *
CharUpper(TCHAR *s)
{
  TCHAR *p;
  for (p = s; *p != 0; ++p)
    *p = (TCHAR)toupper(*p);
  return s;
}

#endif
