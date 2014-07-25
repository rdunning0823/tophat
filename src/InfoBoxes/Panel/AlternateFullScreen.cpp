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
#include "Widget/ListWidget.hpp"
#include "Widget/TwoWidgets.hpp"
#include "Widget/TextWidget.hpp"
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
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Screen/Font.hpp"
#include "Screen/SingleWindow.hpp"

enum Buttons {
  Goto = 100,
  Details,
};

const TCHAR *distance_label_text = N_("Distance");
const TCHAR *arrival_alt_label_text = N_("Arrival alt");

/* used by item Draw() to match column headers with row data */
static unsigned distance_label_width;

/**
 * A widget class that displays header rows above the List Widget
 * when used with TwoWidgets
 */
class AlternatesListHeaderWidget : public TextWidget
{
public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Move(const PixelRect &rc) override;

  virtual PixelSize GetMinimumSize() const {
    return PixelSize { 25u, Layout::GetMinimumControlHeight() / 2 };
  }
  virtual PixelSize GetMaximumSize() const {
    return PixelSize { 25u, Layout::GetMaximumControlHeight() };
  }
  void CalculateLayout(const PixelRect &rc);
};

void
AlternatesListHeaderWidget::CalculateLayout(const PixelRect &rc)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  PixelSize sz_space = dialog_look.text_font->TextSize(_T(" "));
  distance_label_width =
      dialog_look.text_font->TextSize(distance_label_text).cx + 5 * sz_space.cx;

  unsigned spaces_needed = rc.GetSize().cx / 2 / sz_space.cx;
  StaticString<100> spaces (_T("                                                                                                    "));
  if (spaces_needed < spaces.length())
    spaces.Truncate(spaces_needed);
  StaticString<1000> caption;
  caption.Format(_T("%s%s     %s"),spaces.c_str(), distance_label_text,
                       arrival_alt_label_text);
  TextWidget::SetText(caption.c_str());
}

void
AlternatesListHeaderWidget::Move(const PixelRect &rc)
{
  TextWidget::Move(rc);
  CalculateLayout(rc);
}

void
AlternatesListHeaderWidget::Prepare(ContainerWindow &parent,
                                    const PixelRect &rc)
{
  TextWidget::Prepare(parent, rc);
  CalculateLayout(rc);
}

void
AlternatesListHeaderWidget::Unprepare()
{
  TextWidget::Unprepare();
}

/**
 * a widget that lists the alternates and executes the actions
 * but has no buttons visible
 */
class AlternatesListWidget2 : public AlternatesListWidget
{
protected:
  WndForm *form;
public:
  AlternatesListWidget2(const DialogLook &_dialog_look)
    :AlternatesListWidget(_dialog_look) {}

  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

  /* virtual methods from class Widget */
  virtual PixelSize GetMinimumSize() const {
    return GetMaximumSize();
  }
  virtual void Move(const PixelRect &rc) override;
  bool DoDetails();
  bool DoGoto();
  const Waypoint* GetWaypoint();

  virtual void OnActivateItem(unsigned index) override;

  void Refresh();

  /* virtual methods from class List::Handler */
  /**
   * paints a list that lines up with the column headers
   */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned index) override;
};

void
AlternatesListWidget2::Move(const PixelRect &rc)
{
  AlternatesListWidget::Move(rc);
}

void
AlternatesListWidget2::OnPaintItem(Canvas &canvas, const PixelRect rc,
                                  unsigned index)
{
  assert(index < alternates.size());

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const Waypoint &waypoint = alternates[index].waypoint;
  const GlideResult& solution = alternates[index].solution;

  WaypointListRenderer::Draw2(canvas, rc, waypoint, solution.vector.distance,
                             solution.SelectAltitudeDifference(settings.task.glide),
                             UIGlobals::GetDialogLook(),
                             UIGlobals::GetMapLook().waypoint,
                             CommonInterface::GetMapSettings().waypoint,
                             distance_label_width);
}

void
AlternatesListWidget2::Refresh()
{
  AlternatesListWidget::Update();
  GetList().SetLength(alternates.size());
  GetList().Invalidate();
}

void
AlternatesListWidget2::OnActivateItem(unsigned index)
{
  if (DoDetails())
    form->SetModalResult(mrOK);
}

const Waypoint*
AlternatesListWidget2::GetWaypoint()
{
  unsigned index = GetCursorIndex();
  if (index >= alternates.size())
    return nullptr;
  auto const &item = alternates[index];
  auto const &waypoint = item.waypoint;
  return &waypoint;
}

bool
AlternatesListWidget2::DoGoto()
{
  const Waypoint *waypoint = GetWaypoint();
  if (waypoint != nullptr) {
    protected_task_manager->DoGoto(*waypoint);
    return true;
  }
  return false;
}

bool
AlternatesListWidget2::DoDetails()
{
  const Waypoint *waypoint = GetWaypoint();
  if (waypoint != nullptr) {
    dlgWaypointDetailsShowModal(*waypoint, true, false);
    return true;
  }
  return false;
}


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
  TwoCommandButtonListLayout::Prepare(parent, content_rc);

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
  BaseAccessPanel::Unprepare();
}

Widget *
LoadAlternatesPanelFullScreen(unsigned id)
{
  return new AlternateFullScreen(id);
}
