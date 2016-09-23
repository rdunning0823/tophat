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

#include "TeamCode.hpp"
#include "Base.hpp"
#include "UIGlobals.hpp"
#include "Form/Button.hpp"
#include "Components.hpp"
#include "Look/DialogLook.hpp"
#include "Widget/DockWindow.hpp"
#include "Widget/ManagedWidget.hpp"
#include "Screen/SingleWindow.hpp"
#include "Dialogs/Traffic/TrafficDialogs.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/Message.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "FLARM/FlarmDetails.hpp"
#include "FLARM/Glue.hpp"
#include "Computer/Settings.hpp"
#include "Profile/Profile.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Interface.hpp"
#include "Blackboard/BlackboardListener.hpp"
#include "Language/Language.hpp"
#include "TeamActions.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Waypoint/FlarmGlue.hpp"
#include "Util/StringAPI.hxx"


class TeamCodeWidget2 final
  : public RowFormWidget, NullBlackboardListener {

  enum Controls {
    OWN_CODE,
    MATE_CODE,
    RANGE,
    BEARING,
    RELATIVE_BEARING,
    FLARM_LOCK,
  };

public:
  TeamCodeWidget2(const DialogLook &look)
    :RowFormWidget(look) {}

  void Update(const MoreData &basic, const DerivedInfo &calculated);

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;

  /* virtual methods from class BlackboardListener */
  virtual void OnCalculatedUpdate(const MoreData &basic,
                                  const DerivedInfo &calculated) override;
};

void
TeamCodeWidget2::Prepare(ContainerWindow &parent,
                         const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  AddReadOnly(_("Own code"));
  AddReadOnly(_("Mate code"));
  AddReadOnly(_("Range"));
  AddReadOnly(_("Bearing"));
  AddReadOnly(_("Rel. bearing"));
  AddReadOnly(_("Flarm lock"));
}

void
TeamCodeWidget2::Show(const PixelRect &rc)
{
  RowFormWidget::Show(rc);
  Update(CommonInterface::Basic(), CommonInterface::Calculated());
  CommonInterface::GetLiveBlackboard().AddListener(*this);
}

void
TeamCodeWidget2::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  RowFormWidget::Hide();
}

void
TeamCodeWidget2::Update(const MoreData &basic, const DerivedInfo &calculated)
{
  const TeamInfo &teamcode_info = calculated;
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;
  StaticString<100> buffer;

  if (teamcode_info.teammate_available && basic.track_available) {
    FormatAngleDelta(buffer.buffer(), buffer.CAPACITY,
                     teamcode_info.teammate_vector.bearing - basic.track);
  } else {
    buffer = _T("---");
  }

  SetText(RELATIVE_BEARING, buffer);

  if (teamcode_info.teammate_available) {
    FormatBearing(buffer.buffer(), buffer.CAPACITY,
                  teamcode_info.teammate_vector.bearing);
    SetText(BEARING, buffer);

    FormatUserDistanceSmart(teamcode_info.teammate_vector.distance,
                            buffer.buffer());
    SetText(RANGE, buffer);
  } else {
    SetText(BEARING, _T("---"));
    SetText(RANGE, _T("---"));
  }

  SetText(OWN_CODE, teamcode_info.own_teammate_code.GetCode());
  SetText(MATE_CODE, settings.team_code.GetCode());

  buffer = settings.team_flarm_id.IsDefined()
      ? settings.team_flarm_callsign.c_str()
      : _T("");
  if (settings.team_flarm_id.IsDefined()
      && !teamcode_info.flarm_teammate_code_current)
    buffer.AppendFormat(_T(" - %s"), _("Not current"));
  SetText(FLARM_LOCK, buffer.c_str());
}

void
TeamCodeWidget2::OnCalculatedUpdate(const MoreData &basic,
                                    const DerivedInfo &calculated)
{
  Update(basic, calculated);
}


/**
 * wrapper class for TeamCodeWidget2 so it fits in an InfoBox panel
 */
class TeamCodeFullScreen final
  : public BaseAccessPanel, FourCommandButtonWidgetLayout,
    NullBlackboardListener
{
  enum Buttons {
    SET_CODE,
    GOTO,
    SET_WAYPOINT,
    SET_FLARM_LOCK,
  };

protected:
  DockWindow widget_dock;
  TeamCodeWidget2 *team_code_widget2;

  Button *set_code_button, *goto_button, *set_waypoint_button, *flarm_lock_button;

public:
  TeamCodeFullScreen(unsigned _id)
    :BaseAccessPanel(_id) {}


  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;

  /* This is a hack because Move() must discard rc and use GetMainWindow() */
  virtual void Move(const PixelRect &rc) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;
protected:
  /* methods from ActionListener */
  virtual void OnAction(int id) override;

  /* virtual methods from class BlackboardListener */
  virtual void OnCalculatedUpdate(const MoreData &basic,
                                  const DerivedInfo &calculated) override;

  /**
   * @return. true if teammate task is active
   */
  bool IsTeammateTask();

private:
  void OnCodeClicked();
  bool OnGotoClicked();
  void OnSetWaypointClicked();
  void OnFlarmLockClicked();
  void UpdateLabels(const MoreData &basic, const DerivedInfo &calculated);
};

void
TeamCodeFullScreen::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  BaseAccessPanel::Move(rc);
  FourCommandButtonWidgetLayout::CalculateLayout(content_rc);
  widget_dock.Move(widget_rc);
  set_code_button->Move(left_button_rc);
  goto_button->Move(left_middle_button_rc);
  set_waypoint_button->Move(middle_button_rc);
  flarm_lock_button->Move(right_button_rc);
}

void
TeamCodeFullScreen::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  FourCommandButtonWidgetLayout::CalculateLayout(content_rc);

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  WindowStyle style;
  style.ControlParent();

  widget_dock.Create(*this, widget_rc, style);
  team_code_widget2 = new TeamCodeWidget2(dialog_look);
  widget_dock.SetWidget(team_code_widget2);
  widget_dock.Move(widget_rc);

  WindowStyle button_style;
  button_style.TabStop();
  set_code_button = new Button(GetClientAreaWindow(), button_look, _T("Set code"),
                                  left_button_rc,
                              button_style, *this, SET_CODE);

  goto_button = new Button(GetClientAreaWindow(), button_look, _T("Goto"),
                           left_middle_button_rc,
                           button_style, *this, GOTO);

  set_waypoint_button = new Button(GetClientAreaWindow(), button_look, _T("Set WP"),
                                 middle_button_rc,
                                 button_style, *this, SET_WAYPOINT);

  flarm_lock_button = new Button(GetClientAreaWindow(), button_look, _T("Flarm lock"),
                                 right_button_rc,
                                 button_style, *this, SET_FLARM_LOCK);

  UpdateLabels(CommonInterface::Basic(), CommonInterface::Calculated());
}

void
TeamCodeFullScreen::Unprepare()
{
  assert (team_code_widget2 != nullptr);
  team_code_widget2->Unprepare();
  delete team_code_widget2;
  delete(set_code_button);
  delete(goto_button);
  delete(set_waypoint_button);
  delete(flarm_lock_button);
}

void
TeamCodeFullScreen::OnCalculatedUpdate(const MoreData &basic,
                                       const DerivedInfo &calculated)
{
  UpdateLabels(basic, calculated);
}

void
TeamCodeFullScreen::Show(const PixelRect &rc)
{
  widget_dock.ShowOnTop();
  CommonInterface::GetLiveBlackboard().AddListener(*this);
  BaseAccessPanel::Show(rc);
}

void
TeamCodeFullScreen::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  team_code_widget2->Hide();
  widget_dock.Hide();
  BaseAccessPanel::Hide();
}

inline void
TeamCodeFullScreen::OnSetWaypointClicked()
{
  const Waypoint* wp =
    ShowWaypointListDialog(CommonInterface::Basic().location);
  if (wp != NULL) {
    CommonInterface::SetComputerSettings().team_code.team_code_reference_waypoint = wp->id;
    Profile::Set(ProfileKeys::TeamcodeRefWaypoint, wp->id);
  }
}

bool
TeamCodeFullScreen::IsTeammateTask()
{
  // we could use common_stats.task to check this, but it might be delayed
  // by a clock tick since this is called by the main thread
  // and common_stats are updated in the calculation thread.
  ProtectedTaskManager::Lease _task(*protected_task_manager);
  return _task->GetMode() == TaskType::TEAMMATE;
}

void
TeamCodeFullScreen::UpdateLabels(const MoreData &basic,
                                 const DerivedInfo &calculated)
{
  if (IsTeammateTask())
    goto_button->SetCaption(_("End goto"));
  else
    goto_button->SetCaption(_("Goto"));

  goto_button->SetEnabled(calculated.teammate_available);
}


inline void
TeamCodeFullScreen::OnCodeClicked()
{
  TCHAR newTeammateCode[10];
  newTeammateCode[0] = '\0';

  if (!TextEntryDialog(newTeammateCode, 7))
    return;

  StripRight(newTeammateCode);

  TeamCodeSettings &settings =
    CommonInterface::SetComputerSettings().team_code;
  settings.team_code.Update(newTeammateCode);

  settings.team_flarm_id.Clear();

  FlarmWaypointGlue::ClearTeammateTask(protected_task_manager);
}

inline void
TeamCodeFullScreen::OnFlarmLockClicked()
{
  TeamCodeSettings &settings =
    CommonInterface::SetComputerSettings().team_code;
  TCHAR newTeamFlarmCNTarget[settings.team_flarm_callsign.CAPACITY];
  _tcscpy(newTeamFlarmCNTarget, settings.team_flarm_callsign.c_str());

  if (!TextEntryDialog(newTeamFlarmCNTarget, 4))
    return;

  if (StringIsEmpty(newTeamFlarmCNTarget)) {
    settings.team_flarm_id.Clear();
    settings.team_flarm_callsign.clear();
    return;
  }

  LoadFlarmDatabases();

  FlarmId ids[30];
  unsigned count =
    FlarmDetails::FindIdsByCallSign(newTeamFlarmCNTarget, ids, 30);

  if (count == 0) {
    ShowMessageBox(_("Unknown Competition Number"),
                   _("Not Found"), MB_OK | MB_ICONINFORMATION);
    return;
  }
  FlarmWaypointGlue::ClearTeammateTask(protected_task_manager);

  const FlarmId id = PickFlarmTraffic(_("Set new teammate"), ids, count);
  if (!id.IsDefined())
    return;

  TeamActions::TrackFlarm(id, newTeamFlarmCNTarget);
}

bool
TeamCodeFullScreen::OnGotoClicked()
{
  if (IsTeammateTask()) {
    FlarmWaypointGlue::ClearTeammateTask(protected_task_manager);
    return false;
  }
  const TeamInfo &teamcode_info = CommonInterface::Calculated();

  if (teamcode_info.teammate_location.IsValid()) {

    const TeamCodeSettings &settings =
      CommonInterface::GetComputerSettings().team_code;

    FlarmWaypointGlue::AddTeammateTask(teamcode_info.teammate_location,
                                       terrain,
                                       protected_task_manager,
                                       settings.team_flarm_id.IsDefined()
                                       && !teamcode_info.flarm_teammate_code_current);
    return true;
  } else {
    ShowMessageBox(_("Bad team code"),
                   _("Can't goto team location"), MB_OK | MB_ICONINFORMATION);
    return false;
  }
}

void
TeamCodeFullScreen::OnAction(int id)
{
  switch (id) {
  case SET_CODE:
    OnCodeClicked();
    break;

  case GOTO:
    if (OnGotoClicked())
      Close();
    break;

  case SET_WAYPOINT:
    OnSetWaypointClicked();
    break;

  case SET_FLARM_LOCK:
    OnFlarmLockClicked();
    break;
  default:
    BaseAccessPanel::OnAction(id);
  }
}

Widget *
LoadTeamCodePanelFullScreen(unsigned id)
{
  return new TeamCodeFullScreen(id);
}
