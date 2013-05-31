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

#ifndef XCSOAR_WIND_SETTINGS_PANEL_HPP
#define XCSOAR_WIND_SETTINGS_PANEL_HPP

#include "Form/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"

class WndProperty;

class WindSettingsPanel : public RowFormWidget, DataFieldListener {
  enum ControlIndex {
    AutoWind,
    ExternalWind,
    TrailDrift,
    Speed,
    Direction,
  };

  const bool edit_manual_wind, edit_trail_drift;

  bool external_wind;

  WndProperty *auto_wind;

public:
  /**
   * @param manual_wind edit the manual wind setting
   */
  WindSettingsPanel(bool edit_manual_wind, bool edit_trail_drift);

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

  /** DataFieldListener */
  virtual void OnModified(DataField &df);

  /**
   * Shows/Hides controls based on Auto Wind value
   */
  void SetVisibility();
};

#endif
