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

#ifndef XCSOAR_CONDITION_MONITOR_START_RULES_HPP
#define XCSOAR_CONDITION_MONITOR_START_RULES_HPP

#include "ConditionMonitor.hpp"

/**
 * Checks whether aircraft in start sector is within height/speed rules
 */
class ConditionMonitorStartRules: public ConditionMonitor
{
  bool withinMargin;

public:
  ConditionMonitorStartRules():ConditionMonitor(60, 1), withinMargin(false) {}

protected:
  virtual bool CheckCondition(const NMEAInfo &basic,
                              const DerivedInfo &calculated,
                              const ComputerSettings &settings) gcc_override;
  virtual void Notify();
  virtual void SaveLast() {}
};

#endif
