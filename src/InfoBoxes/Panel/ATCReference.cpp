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

#include "ATCReference.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Form/Form.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Formatter/GeoPointFormatter.hpp"
#include "Util/Macros.hpp"
#include "Components.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "InfoBoxes/Panel/Base.hpp"
#include "Waypoint/WaypointGlue.hpp"
#include "Protection.hpp"
#include "Message.hpp"
#include "Screen/SingleWindow.hpp"
#include "Profile/Current.hpp"

enum Controls {
  WAYPOINT,
  LOCATION,
  DUMMY,
  RELOCATE,
  CLEAR,
};

class ATCReferencePanel : public RowFormWidget, ActionListener {
protected:
  WndForm *form;

public:
  ATCReferencePanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void UpdateValues();

  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  /* Move() uses the passed rc for the RowFormWidget, but most move the form to
   * the fullscreen using GetMainWindow()'s rect */
  virtual void Move(const PixelRect &rc) override;

private:
  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

void
ATCReferencePanel::Move(const PixelRect &rc)
{
  RowFormWidget::Move(rc);
  form->Move(UIGlobals::GetMainWindow().GetClientRect());
}

void
ATCReferencePanel::UpdateValues()
{
  const GeoPoint &location =
    CommonInterface::GetComputerSettings().poi.atc_reference;

  const Waypoint *waypoint = location.IsValid()
    ? way_points.GetNearest(location, fixed(100))
    : nullptr;

  SetText(WAYPOINT, waypoint != nullptr ? waypoint->name.c_str() : _T("---"));

  const TCHAR *location_string;
  TCHAR buffer[64];
  if (location.IsValid()) {
    FormatGeoPoint(location, buffer, ARRAY_SIZE(buffer),
                   CommonInterface::GetUISettings().format.coordinate_format);
    location_string = buffer;
  } else
    location_string = _T("---");

  SetText(LOCATION, location_string);
}

void
ATCReferencePanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  AddReadOnly(_("Waypoint"));
  AddReadOnly(_("Location"));
  AddSpacer();

  AddButton(_("Select waypoint"), *this, RELOCATE);
  AddButton(_("Clear"), *this, CLEAR);

  UpdateValues();
}

void
ATCReferencePanel::OnAction(int id)
{
  GeoPoint &location =
    CommonInterface::SetComputerSettings().poi.atc_reference;

  const Waypoint *waypoint;

  switch (id) {
  case RELOCATE:
    waypoint = ShowWaypointListDialog(CommonInterface::Basic().location);
    if (waypoint != nullptr) {
      location = waypoint->location;
      CommonInterface::SetComputerSettings().poi.atc_reference = location;
      {
        ScopeSuspendAllThreads suspend;
        WaypointGlue::SaveATCReference(Profile::map,
            CommonInterface::SetComputerSettings().poi);
      }
      StaticString<255> message;
      message.Format(_T("%s %s: %s"), _("ATC radial"), _("set to"),
                     waypoint->name.c_str());
      Message::AddMessage(message.c_str());
      form->SetModalResult(mrOK);
    }
    break;

  case CLEAR:
    location.SetInvalid();
    UpdateValues();
    break;
  }
}


class ATCReferencePanelFullscreen : public BaseAccessPanel {
public:

  ATCReferencePanelFullscreen(unsigned _id, ATCReferencePanel *atc_reference_panel)
    :BaseAccessPanel(_id, atc_reference_panel) {}
};

Widget *
LoadATCReferencePanel(unsigned id)
{
  ATCReferencePanel *atc_reference_panel = new ATCReferencePanel();

  ATCReferencePanelFullscreen * atc_reference_panel_full_screen =
      new ATCReferencePanelFullscreen(id, atc_reference_panel);

  /* so it can call SetModelResult() */
  atc_reference_panel->SetForm(atc_reference_panel_full_screen);
  return atc_reference_panel_full_screen;
}
