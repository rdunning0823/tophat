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

#include "ObservationZoneSummaryWidget.hpp"
#include "ObservationZoneEditWidget.hpp"
#include "Engine/Task/ObservationZones/CylinderZone.hpp"
#include "Language/Language.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/ButtonPanel.hpp"
#include "Look/DialogLook.hpp"
#include "UIGlobals.hpp"
#include "Engine/Task/ObservationZones/SectorZone.hpp"
#include "SectorZoneEditWidget.hpp"
#include "Screen/SingleWindow.hpp"

#include <assert.h>

enum Controls {
  SummaryButton,
};

ObservationZoneSummaryWidget::ObservationZoneSummaryWidget(
    ObservationZoneEditWidget& _edit_widget)
  :RowFormWidget(UIGlobals::GetDialogLook(), false),
   edit_widget(_edit_widget), dialog(UIGlobals::GetDialogLook()) {}

void
ObservationZoneSummaryWidget::OnAction(int id)
{
  dialog.ShowModal();
  dialog.Hide();

  UpdateButtonText();
}

void
ObservationZoneSummaryWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  AddButton(_T(""), *this, SummaryButton);

  StaticString<255> name(GetWidget().GetWaypointName());
  dialog.CreateFull(UIGlobals::GetMainWindow(),
                    name.c_str(), &edit_widget,
                    nullptr, 0, ButtonPanel::ButtonPanelPosition::Bottom);

  dialog.AddSymbolButton(_T("_X"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  UpdateButtonText();
}

void
ObservationZoneSummaryWidget::UpdateButtonText()
{
  Button &button = (Button&)RowFormWidget::GetRow(SummaryButton);
  StaticString<255>summary;
  summary = GetWidget().GetOzSummary();
  button.SetCaption(summary.c_str());
}

bool
ObservationZoneSummaryWidget::Save(bool &changed)
{
  return GetWidget().Save(changed);
}
