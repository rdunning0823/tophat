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

#include "TeamCode.hpp"
#include "Geo/Math.hpp"
#include "Geo/GeoPoint.hpp"
#include "Math/Angle.hpp"
#include "Util/StringUtil.hpp"

#include <math.h>
#include <string.h>

#define TEAMCODE_COMBINATIONS 1296

/**
 * Decodes the TeamCode
 * @param code The teamcode (or part of it)
 * @param maxCount Maximum chars to decode
 * @return The decoded value
 */
static int
GetValueFromTeamCode(const TCHAR *code, int maxCount)
{
  int val = 0;
  int charPos = 0;

  while (code[charPos] != 0 && charPos < maxCount) {
    int cifferVal = 0;

    if (code[charPos] >= '0' && code[charPos] <= '9') {
      cifferVal = (int)(code[charPos] - '0');
    } else if (code[charPos] >= 'A' && code[charPos] <= 'Z') {
      cifferVal = (int)(code[charPos] + -'A') + 10;
    }

    val = val * 36;
    val += (int)cifferVal;

    charPos++;
  }

  return val;
}

/**
 * Encodes a value to teamcode
 * @param value The value to encode
 * @param code The teamcode (pointer)
 * @param minCiffers Number of chars for the teamcode
 */
static void
NumberToTeamCode(fixed value, TCHAR *code, int minCiffers)
{
  int maxCif = 0;
  int curCif = 0;

  if (minCiffers > 0)	{
    maxCif = minCiffers - 1;
    curCif = maxCif;
  }

  fixed rest = value;
  while (positive(rest) || curCif >= 0) {
    int cifVal = (int)pow(36.0, curCif);
    int partSize = (int)(rest / cifVal);
    fixed partVal(partSize * cifVal);
    int txtPos = maxCif - curCif;

    if (partSize < 10) {
      rest -= partVal;
      code[txtPos] = (unsigned char)('0' + partSize);
      curCif--;
    } else if (partSize < 36) {
      rest -= partVal;
      code[txtPos] = (unsigned char)('A' + partSize - 10);
      curCif--;
    } else {
      curCif++;
      maxCif = curCif;
    }

    if (rest < fixed_one)
      rest = fixed_zero;
  }
}

/**
 * Converts a given bearing to the bearing part of the teamcode
 * @param bearing Bearing to the reference waypoint
 * @param code The teamcode (pointer)
 */
static void
ConvertBearingToTeamCode(const Angle bearing, TCHAR *code)
{
  const fixed bamValue = bearing.AsBearing().Degrees() / 360
                          * TEAMCODE_COMBINATIONS;
  NumberToTeamCode(bamValue, code, 2);
}

Angle
TeamCode::GetBearing() const
{
  // Get the first two values from teamcode (1-2)
  int val = GetValueFromTeamCode(code, 2);

  // Calculate bearing
  return Angle::Degrees(fixed(val * 360.0 / TEAMCODE_COMBINATIONS)).AsBearing();
}

fixed
TeamCode::GetRange() const
{
  // Get last three values from teamcode (3-5)
  int val = GetValueFromTeamCode(code.begin() + 2, 3);
  return fixed(val * 100);
}

void
TeamCode::Update(Angle bearing, fixed range)
{
  // Clear teamcode
  std::fill(code.buffer(), code.buffer() + code.MAX_SIZE, _T('\0'));
  // Calculate bearing part of the teamcode
  ConvertBearingToTeamCode(bearing, code.buffer());
  // Calculate distance part of the teamcode
  NumberToTeamCode(range / 100, code.buffer() + 2, 0);
}

void
TeamCode::Update(const TCHAR* _code)
{
  code = _code;
}

GeoPoint
TeamCode::GetLocation(const GeoPoint ref) const
{
  Angle bearing = GetBearing();
  fixed distance = GetRange();

  return FindLatitudeLongitude(ref, bearing, distance);
}
