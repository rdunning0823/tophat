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

#ifndef XCSOAR_DATA_FIELD_INTEGER_HPP
#define XCSOAR_DATA_FIELD_INTEGER_HPP

#include "Number.hpp"
#include "PeriodClock.hpp"

class DataFieldInteger : public NumberDataField
{
private:
  int value;
  int min;
  int max;
  int step;
  PeriodClock last_step;
  int speedup;
  TCHAR outpuf_buffer[OUTBUFFERSIZE + 1];

protected:
  int SpeedUp(bool keyup);

public:
  DataFieldInteger(const TCHAR *EditFormat, const TCHAR *DisplayFormat,
                   int Min, int Max,
                   int Default, int Step, DataAccessCallback OnDataAccess)
    :NumberDataField(Type::INTEGER, true, EditFormat, DisplayFormat, OnDataAccess),
     value(Default), min(Min), max(Max), step(Step) {}

  void Inc();
  void Dec();
  virtual ComboList *CreateComboList() const;
  virtual void SetFromCombo(int iDataFieldIndex, TCHAR *sValue);

  virtual int GetAsInteger() const;
  virtual const TCHAR *GetAsString() const;
  virtual const TCHAR *GetAsDisplayString() const;

  void Set(int _value) {
    value = _value;
  }

  int SetMin(int _min) {
    min = _min;
    return min;
  }

  int SetMax(int _max) {
    max = _max;
    return max;
  }

  virtual void SetAsInteger(int value);
  virtual void SetAsString(const TCHAR *value);

protected:
  void AppendComboValue(ComboList &combo_list, int value) const;
};

#endif
