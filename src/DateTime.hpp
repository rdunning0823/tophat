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

#ifndef XCSOAR_DATE_TIME_HPP
#define XCSOAR_DATE_TIME_HPP

#include "Util/TypeTraits.hpp"
#include "Compiler.h"

#include <stdint.h>

/**
 * A broken-down representation of a date.
 */
struct BrokenDate {
  /**
   * Absolute year, e.g. "2010".
   */
  uint16_t year;

  /**
   * Month number, 1-12.
   */
  uint8_t month;

  /**
   * Day of month, 1-31.
   */
  uint8_t day;

  /**
   * Day of the week (0-6, 0: sunday).  -1 means the value has not
   * been determined.
   */
  int8_t day_of_week;

  /**
   * Non-initializing default constructor.
   */
  BrokenDate() = default;

  constexpr
  BrokenDate(unsigned _year, unsigned _month, unsigned _day)
    :year(_year), month(_month), day(_day), day_of_week(-1) {}

  constexpr
  bool operator==(const BrokenDate other) const {
    return year == other.year && month == other.month && day == other.day;
  }

  constexpr
  bool operator>(const BrokenDate other) const {
    return year > other.year ||
      (year == other.year && (month > other.month ||
                              (month == other.month && day > other.day)));
  }

  /**
   * Clears the object, to make the Plausible() check returns false.
   */
  void Clear() {
    year = 0;
  }

  /**
   * Returns an instance that fails the Plausible() check.
   */
  constexpr
  static BrokenDate Invalid() {
    return BrokenDate(0, 0, 0);
  }

  /**
   * Does this object contain plausible values?
   */
  constexpr
  bool Plausible() const {
    return year >= 1800 && year <= 2500 &&
      month >= 1 && month <= 12 &&
      day >= 1 && day <= 31;
  }

  void IncrementDay();
};

/**
 * A broken-down representation of a time.
 */
struct BrokenTime {
  /**
   * Hour of day, 0-23.
   */
  uint8_t hour;

  /**
   * Minute, 0-59.
   */
  uint8_t minute;

  /**
   * Second, 0-59.
   */
  uint8_t second;

  /**
   * Non-initializing default constructor.
   */
  BrokenTime() = default;

  constexpr
  BrokenTime(unsigned _hour, unsigned _minute, unsigned _second=0)
    :hour(_hour), minute(_minute), second(_second) {}

  constexpr
  bool operator==(const BrokenTime other) const {
    return hour == other.hour && minute == other.minute &&
      second == other.second;
  }

  constexpr
  bool operator>(const BrokenTime other) const {
    return hour > other.hour ||
      (hour == other.hour && (minute > other.minute ||
                              (minute == other.minute && second > other.second)));
  }

  /**
   * Returns an instance that fails the Plausible() check.
   */
  constexpr
  static BrokenTime Invalid() {
    return BrokenTime(24, 60, 60);
  }

  /**
   * Does this object contain plausible values?
   */
  constexpr
  bool Plausible() const {
    return hour < 24 && minute < 60 && second < 60;
  }

  /**
   * Returns the number of seconds which have passed on this day.
   */
  constexpr
  unsigned GetSecondOfDay() const {
    return hour * 3600u + minute * 60u + second;
  }

  /**
   * Construct a BrokenTime object from the specified number of
   * seconds which have passed on this day.
   *
   * @param second_of_day 0 .. 3600*24-1
   */
  gcc_const
  static BrokenTime FromSecondOfDay(unsigned second_of_day);

  /**
   * A wrapper for FromSecondOfDay() which allows values bigger than
   * or equal to 3600*24.
   */
  gcc_const
  static BrokenTime FromSecondOfDayChecked(unsigned second_of_day);

  /**
   * Returns a BrokenTime that has the specified number of seconds
   * added to it.  It properly wraps around midnight.
   *
   * @param seconds the number of seconds to add
   */
  gcc_pure
  BrokenTime operator+(unsigned seconds) const;

  /**
   * Returns a BrokenTime that has the specified number of seconds
   * added to it.  It properly wraps around midnight.
   *
   * @param seconds the number of seconds to add; may be negative
   */
  gcc_pure
  BrokenTime operator+(int seconds) const;

  BrokenTime operator-(int seconds) const {
    return *this + (-seconds);
  }

  BrokenTime operator-(unsigned seconds) const {
    return *this - int(seconds);
  }
};

/**
 * A broken-down representation of date and time.
 */
struct BrokenDateTime : public BrokenDate, public BrokenTime {
  /**
   * Non-initializing default constructor.
   */
  BrokenDateTime() = default;

  constexpr
  BrokenDateTime(unsigned _year, unsigned _month, unsigned _day,
                 unsigned _hour, unsigned _minute, unsigned _second=0)
    :BrokenDate(_year, _month, _day), BrokenTime(_hour, _minute, _second) {}

  constexpr
  BrokenDateTime(unsigned _year, unsigned _month, unsigned _day)
    :BrokenDate(_year, _month, _day), BrokenTime(0, 0) {}

  constexpr
  BrokenDateTime(const BrokenDate &date, const BrokenTime &time)
    :BrokenDate(date), BrokenTime(time) {}

  constexpr
  bool operator==(const BrokenDateTime other) const {
    return (const BrokenDate &)*this == (const BrokenDate &)other &&
      (const BrokenTime &)*this == (const BrokenTime &)other;
  }

  /**
   * Returns an instance that fails the Plausible() check.
   */
  constexpr
  static BrokenDateTime Invalid() {
    return BrokenDateTime(BrokenDate::Invalid(), BrokenTime::Invalid());
  }

  /**
   * Does this object contain plausible values?
   */
  constexpr
  bool Plausible() const {
    return BrokenDate::Plausible() && BrokenTime::Plausible();
  }

#ifdef HAVE_POSIX
  /**
   * Convert a UNIX UTC time stamp (seconds since epoch) to a
   * BrokenDateTime object.
   */
  gcc_const
  static BrokenDateTime FromUnixTimeUTC(int64_t t);
#endif

  gcc_pure
  int64_t ToUnixTimeUTC() const;

  /**
   * Returns the current system date and time, in UTC.
   */
  static const BrokenDateTime NowUTC();

  /**
   * Returns the current system date and time, in the current time zone.
   */
  static const BrokenDateTime NowLocal();

  /**
   * Returns a BrokenDateTime that has the specified number of seconds
   * added to it.
   *
   * @param seconds the number of seconds to add; may be negative
   */
  gcc_pure
  BrokenDateTime operator+(int seconds) const;

  BrokenDateTime operator-(int seconds) const {
    return *this + (-seconds);
  }

  /**
   * Returns the number of seconds between the two BrokenDateTime structs.
   * The second one is subtracted from the first one.
   *
   * <now> - <old> = positive timespan since <old> in seconds
   */
  int operator-(const BrokenDateTime &other) const;
};

static_assert(is_trivial<BrokenDateTime>::value, "type is not trivial");

#endif
