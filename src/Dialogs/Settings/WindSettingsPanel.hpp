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

#ifndef XCSOAR_WIND_SETTINGS_PANEL_HPP
#define XCSOAR_WIND_SETTINGS_PANEL_HPP

#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/Form.hpp"

class Button;

class WindSettingsPanel final
  : public RowFormWidget,
    private DataFieldListener {
  enum ControlIndex {
    WIND_SOURCE,
    TrailDrift,
    Speed,
    Direction,
    WIND_ARROW_LOCATION,
  };

  const bool edit_manual_wind, edit_trail_drift;

  /** controls whether the wind location property is shown */
  const bool edit_wind_location;

  /**
   * pointer to the main choice: what type of wind shall we use
   */
  WndProperty *user_wind_source;

  /**
   * the parent form containing the widget
   */
  WndForm *form;

public:

  /**
   * @param manual_wind edit the manual wind setting
   * @param clear_manual_button add a "Clear" button
   * @param edit_wind_location show the wind location property
   */
  WindSettingsPanel(bool edit_manual_wind, bool clear_manual_button,
                    bool edit_trail_drift,
                    bool edit_wind_location);

  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  virtual void Show(const PixelRect &rc) override;

  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;

  /**
   * show or hide the manual editing buttons based on whether
   * the "Manual" type of wind is selected
   */
  void UpdatetManualVisibility();
};

#endif
