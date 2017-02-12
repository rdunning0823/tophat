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

#include "Home.hpp"
#include "Base.hpp"
#include "Components.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Waypoint/WaypointGlue.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hxx"
#include "Language/Language.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Protection.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Screen/SingleWindow.hpp"
#include "Computer/Settings.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "Message.hpp"
#include "Look/DialogLook.hpp"
#include "Profile/Current.hpp"

enum ControlIndex {
  HomeName = 100,
  Change,
  GotoHome,
};


class HomePanel : public BaseAccessPanel, ThreeButtonLayout {

protected:
  /**
   * These 3 items use the layout rectangles
   * calculated in ThreeButtonLayout
   */
  Button *goto_home;
  Button *change;
  WndFrame *home_name;

public:
  HomePanel(unsigned _id)
    :BaseAccessPanel(_id) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Move(const PixelRect &rc) override;

  void Refresh();

protected:

  /* methods from ActionListener */
  virtual void OnAction(int id) override;
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
      WaypointGlue::SetHome(Profile::map, way_points, terrain,
                            settings_computer.poi, settings_computer.team_code,
                            device_blackboard, false);
      WaypointGlue::SaveHome(Profile::map,
                             settings_computer.poi, settings_computer.team_code);
    }
    SetModalResult(mrOK);
    StaticString<255> message;
    message.Format(_T("%s %s: %s"), _("Home"), _("set to"),
                   waypoint->name.c_str());
    Message::AddMessage(message.c_str());
    break;
  }
  case GotoHome:
  {
    ComputerSettings &settings_computer =
      CommonInterface::SetComputerSettings();
    const Waypoint *wp = way_points.LookupId(settings_computer.poi.home_waypoint);
    if (wp != nullptr) {
      protected_task_manager->DoGoto(*wp);
      BaseAccessPanel::Close();
    }
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
HomePanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  ThreeButtonLayout::CalculateLayout(content_rc);
  change->Move(lower_left_rc);
  goto_home->Move(lower_right_rc);
  home_name->Move(upper_rc);
}

void
HomePanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  ThreeButtonLayout::CalculateLayout(content_rc);

  WindowStyle style;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  WindowStyle button_style;
  button_style.TabStop();
  change = new Button(GetClientAreaWindow(), dialog_look.button,
                         _("Select waypoint"),
                         lower_left_rc, button_style, *this, Change);

  goto_home = new Button(GetClientAreaWindow(), dialog_look.button,
                         _("Goto"),
                         lower_right_rc, button_style, *this, GotoHome);

  WindowStyle style_frame;
  home_name = new WndFrame(GetClientAreaWindow(), dialog_look,
                           upper_rc,
                           style_frame);
  home_name->SetVAlignCenter();

  Refresh();
}

void
HomePanel::Unprepare()
{
  delete change;
  delete home_name;
  BaseAccessPanel::Unprepare();
}

Widget *
LoadHomePanel(unsigned id)
{
  return new HomePanel(id);
}
