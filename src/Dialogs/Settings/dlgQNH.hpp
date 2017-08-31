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

#ifndef XCSOAR_DIALOGS_QNH_HPP
#define XCSOAR_DIALOGS_QNH_HPP

#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Event/Timer.hpp"
#include "UIGlobals.hpp"
#include "Atmosphere/Temperature.hpp"
#include "Atmosphere/Pressure.hpp"

class ContainerWindow;
struct PixelRect;

void dlgQNHShowModal();

class QNHPanel
  : public RowFormWidget, DataFieldListener, private Timer
{
  fixed last_altitude;
  bool show_double_units_checkbox;

public:
  QNHPanel(bool _show_double_units_checkbox)
    :RowFormWidget(UIGlobals::GetDialogLook()),
     last_altitude(-2),
     show_double_units_checkbox(_show_double_units_checkbox)
  {}

  void ShowAltitude(fixed altitude);
  void RefreshAltitudeControl();
  void SetQNH(AtmosphericPressure qnh);

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  virtual void Show(const PixelRect &rc) override {
    RowFormWidget::Show(rc);
    Timer::Schedule(500);

    OnTimer();
  }

  virtual void Hide() override {
    Timer::Cancel();
    RowFormWidget::Hide();
  }

private:
  /* virtual methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;

  /* virtual methods from Timer */
  virtual void OnTimer() override;
};


#endif
