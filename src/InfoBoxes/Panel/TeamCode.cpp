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
#include "Base.hpp"


#include "Form/SymbolButton.hpp"
#include "Form/Frame.hpp"
#include "Screen/Fonts.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"

#include "InfoBoxes/Content/Team.hpp"
#include "Formatter/Units.hpp"

enum ControlIndex {
  Plus,
  Minus,
};

class TeamCodePanel : public BaseAccessPanel, NumberButtonLayout {
protected:

 WndSymbolButton *plus, *minus;
 WndFrame *team_code_value;

public:
  TeamCodePanel(unsigned id)
    :BaseAccessPanel(id) {}

  /**
   * changes the code
   * *param direction 1 or -1
   */
  void ChangeCode(const int direction);

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  /**
   * displays the same value as displayed in the TeamCode InfoBox
   */
  void Refresh();

protected:
  /* methods from ActionListener */
  virtual void OnAction(int id);
};

void
TeamCodePanel::OnAction(int action_id)
{
  switch (action_id) {
  case Plus:
    ChangeCode(1);
    break;
  case Minus:
    ChangeCode(-1);
    break;
  default:
    BaseAccessPanel::OnAction(action_id);
  }
}

void
TeamCodePanel::ChangeCode(const int direction)
{
  TeamCodeSettings &settings =
    CommonInterface::SetComputerSettings().team_code;
  const TrafficList &flarm = XCSoarInterface::Basic().flarm.traffic;
  const FlarmTraffic *traffic =
    settings.team_flarm_id.IsDefined()
    ? flarm.FindTraffic(settings.team_flarm_id)
    : NULL;

  if (direction == 1)
    traffic = (traffic == NULL ?
               flarm.FirstTraffic() : flarm.NextTraffic(traffic));
  else
    traffic = (traffic == NULL ?
               flarm.LastTraffic() : flarm.PreviousTraffic(traffic));

  if (traffic != NULL) {
    settings.team_flarm_id = traffic->id;

    if (traffic->HasName()) {
      // copy the 3 first chars from the name to TeamFlarmCNTarget
      settings.team_flarm_callsign = traffic->name;
    } else {
      settings.team_flarm_callsign.clear();
    }
  } else {
    // no flarm traffic to select!
    settings.team_flarm_id.Clear();
    settings.team_flarm_callsign.clear();
  }
  Refresh();
}

void
TeamCodePanel::Refresh()
{
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;

  if (!settings.team_code_reference_waypoint)
    team_code_value->SetCaption(_T("---"));
  else
    team_code_value->SetCaption(XCSoarInterface::Calculated().own_teammate_code.GetCode());
}

void
TeamCodePanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  NumberButtonLayout::Prepare(parent, content_rc);

  const DialogLook &look = UIGlobals::GetDialogLook();

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  plus = new WndSymbolButton(GetClientAreaWindow(), look, _T("^"),
                             double_size_plus_rc,
                             button_style, this, Plus);
  plus->SetFont(Fonts::infobox);

  minus = new WndSymbolButton(GetClientAreaWindow(), look, _T("v"),
                              double_size_minus_rc,
                              button_style, this, Minus);
  minus->SetFont(Fonts::infobox);

  WindowStyle style_frame;
  team_code_value = new WndFrame(GetClientAreaWindow(), look,
                                 value_rc.left, value_rc.top,
                                 value_rc.right - value_rc.left,
                                 value_rc.bottom - value_rc.top,
                                 style_frame);
  team_code_value->SetVAlignCenter();
  team_code_value->SetAlignCenter();
  team_code_value->SetFont(Fonts::infobox);

  Refresh();
}

void
TeamCodePanel::Unprepare()
{
  delete plus;
  delete minus;
  delete team_code_value;
  BaseAccessPanel::Unprepare();
}

Widget *
LoadTeamCodePanel(unsigned id)
{
  return new TeamCodePanel(id);
}
