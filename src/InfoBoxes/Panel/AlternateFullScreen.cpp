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

#include "AlternateFullScreen.hpp"
#include "Base.hpp"
#include "UIGlobals.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Screen/Timer.hpp"
#include "Form/List.hpp"
#include "Form/ActionListener.hpp"
#include "Widget/DockWindow.hpp"
#include "Widget/TwoWidgets.hpp"
#include "UIGlobals.hpp"
#include "Dialogs/Task/AlternatesListDialog.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Interface.hpp"
#include "Look/MapLook.hpp"
#include "Look/DialogLook.hpp"
#include "Renderer/WaypointListRenderer.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Language/Language.hpp"
#include "Screen/Font.hpp"
#include "Screen/SingleWindow.hpp"


class AlternateFullScreen final : public BaseAccessPanel, TwoCommandButtonListLayout
{
protected:
  /**
   * This timer updates the alternates list
   */
  WindowTimer dialog_timer;

  DockWindow list_dock;

  AlternatesListWidget2 *alternates_list_widget2;
  TwoWidgets *two_widgets;

  WndButton *details_button, *goto_button;

public:
  AlternateFullScreen(unsigned _id)
    :BaseAccessPanel(_id), dialog_timer(*this) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Show(const PixelRect &rc);
  /* This is a hack because Move() must discard rc and use GetMainWindow() */
  virtual void Move(const PixelRect &rc) override;

protected:
  /**
   * refresh the glide solutions
   */
  virtual bool OnTimer(WindowTimer &timer);

  /* methods from ActionListener */
  virtual void OnAction(int id);
};

void
AlternateFullScreen::Show(const PixelRect &rc)
{
  list_dock.ShowOnTop();
  BaseAccessPanel::Show(rc);
}

bool
AlternateFullScreen::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    alternates_list_widget2->Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

void
AlternateFullScreen::Move(const PixelRect &rc_unused)
{

  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  BaseAccessPanel::Move(rc);
  TwoCommandButtonListLayout::CalculateLayout(content_rc);
  list_dock.Move(list_rc);
  PixelRect list_rc_adj = list_rc;
  list_rc_adj.Offset(-list_rc.left, -list_rc.top);
  two_widgets->Move(list_rc_adj);
  goto_button->Move(left_button_rc);
  details_button->Move(right_button_rc);
}

void
AlternateFullScreen::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  TwoCommandButtonListLayout::CalculateLayout(content_rc);

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;

  WindowStyle style;
  style.ControlParent();
  list_dock.Create(*this, list_rc, style);
  alternates_list_widget2 = new AlternatesListWidget2(dialog_look);
  alternates_list_widget2->Update();
  alternates_list_widget2->SetForm(this);
  two_widgets = new TwoWidgets(new AlternatesListHeaderWidget(),
                               alternates_list_widget2);
  // dock prepares/ unprepares/ deletes widget
  list_dock.SetWidget(two_widgets);
  list_dock.Move(list_rc);

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  goto_button = new WndButton(GetClientAreaWindow(), button_look, _T("Goto"),
                              left_button_rc,
                              button_style, *this, Goto);

  details_button = new WndButton(GetClientAreaWindow(), button_look, _T("Details"),
                                 right_button_rc,
                                 button_style, *this, Details);
  dialog_timer.Schedule(1000);
}

void
AlternateFullScreen::OnAction(int id)
{
  if (id == Goto) {
    if (alternates_list_widget2->DoGoto())
      BaseAccessPanel::Close();
  } else if (id == Details) {
    if (alternates_list_widget2->DoDetails())
      BaseAccessPanel::Close();
  } else
    BaseAccessPanel::OnAction(id);
}

void
AlternateFullScreen::Unprepare()
{
  delete details_button;
  delete goto_button;
  dialog_timer.Cancel();
}

Widget *
LoadAlternatesPanelFullScreen(unsigned id)
{
  return new AlternateFullScreen(id);
}
