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

#ifndef XCSOAR_OBSERVATION_ZONE_SUMMARY_WIDGET_HPP
#define XCSOAR_OBSERVATION_ZONE_SUMMARY_WIDGET_HPP

#include "ObservationZoneSummaryWidget.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Dialogs/WidgetDialog.hpp"

#include <assert.h>

class ObservationZoneEditWidget;

/**
 * A class for that manages an ObservationZoneEditWidget
 * and displays its summary as a button.
 * Displays the ObservationZoneEditWidget to make edits
 */
class ObservationZoneSummaryWidget : public RowFormWidget, ActionListener {

protected:

  /**
   * pointer to the widget that gets edited and saved
   */
  ObservationZoneEditWidget& edit_widget;

  /**
   * the dialog that manages the ObservationZoneEditWidget
   */
  WidgetDialog dialog;

public:
  ObservationZoneSummaryWidget(ObservationZoneEditWidget& edit_widget);

public:

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

protected:
  /**
   * virtual from ActionListener
   * causes the ObservationZoneEditWidget to display
   */
  virtual void OnAction(int id) override;

  /**
   * Updates the button caption with info from the edit widget
   */
  virtual void UpdateButtonText();

  /**
   * returns the EditWdiget.
   */
  ObservationZoneEditWidget& GetWidget() {
    return edit_widget;
  }
};

#endif
