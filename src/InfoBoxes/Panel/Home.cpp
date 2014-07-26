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

#include "Home.hpp"
#include "Base.hpp"
#include "Components.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Waypoint/WaypointGlue.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hpp"
#include "Language/Language.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Protection.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Computer/Settings.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Message.hpp"
#include "Look/DialogLook.hpp"


enum ControlIndex {
  HomeName = 100,
  Change,
};


class HomePanel : public BaseAccessPanel, TwoButtonLayout {

protected:
  /**
   * These 4 buttons and the mc_value frame use the layout rectangles
   * calculated in NumberButtonLayout
   */
  WndButton *change;
  WndFrame *home_name;

public:
  HomePanel(unsigned _id)
    :BaseAccessPanel(_id) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  void Refresh();

protected:

  /* methods from ActionListener */
  virtual void OnAction(int id);
};

void
HomePanel::OnAction(int action_id)
{
  switch (action_id) {
  case Change:
  {

    const Waypoint* waypoint =
      ShowWaypointListDialog(CommonInterface::Basic().location);
    if (waypoint == nullptr)
      return;

    ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
    settings_computer.poi.SetHome(*waypoint);

    {
      ScopeSuspendAllThreads suspend;
      WaypointGlue::SetHome(way_points, terrain,
                            settings_computer.poi,
                            settings_computer.team_code,
                            device_blackboard, false);
      WaypointGlue::SaveHome(settings_computer.poi, settings_computer.team_code);
    }
    SetModalResult(mrOK);
    StaticString<255> message;
    message.Format(_T("%s %s: %s"), _("Home"), _("set to"),
                   waypoint->name.c_str());
    Message::AddMessage(message.c_str());
    break;
  }
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  Refresh();
}

void
HomePanel::Refresh()
{
  StaticString<256> name;
  ComputerSettings &settings_computer =
    CommonInterface::SetComputerSettings();
  const Waypoint *wp = WaypointGlue::FindHomeId(way_points, settings_computer.poi);

  if (wp == NULL)
    name.Format(_T("%s: *** %s ***"), _("Home"), _("Not defined"));
  else
    name.Format(_T("%s: %s"), _("Home"), wp->name.c_str());

  home_name->SetCaption(name.c_str());
}

void
HomePanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  TwoButtonLayout::CalculateLayout(content_rc);

  /**
   * make Change button smaller easily distinguishable from the full-width
   * Close button
   */
  lower_rc.right /= 2;

  WindowStyle style;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  change = new WndButton(GetClientAreaWindow(), dialog_look.button,
                         _("Select waypoint"),
                         lower_rc, button_style, *this, Change);

  WindowStyle style_frame;
  home_name = new WndFrame(GetClientAreaWindow(), dialog_look,
                           upper_rc,
                           style_frame);
  home_name->SetVAlignCenter();
  AddDestruct(change);
  AddDestruct(home_name);

  Refresh();
}

void
HomePanel::Unprepare()
{
  BaseAccessPanel::Unprepare();
}

Widget *
LoadHomePanel(unsigned id)
{
  return new HomePanel(id);
}
