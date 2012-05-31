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

#include "Alternates.hpp"
#include "Base.hpp"
#include "Form/Button.hpp"
#include "Form/SymbolButton.hpp"
#include "Form/ActionListener.hpp"
#include "Form/List.hpp"
#include "Form/WindowWidget.hpp"
#include "Form/PanelWidget.hpp"
#include "Form/Panel.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/ListPicker.hpp"
#include "Screen/Layout.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Look/MapLook.hpp"
#include "Look/DialogLook.hpp"
#include "Renderer/WaypointListRenderer.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"

enum ControlIndex {
  NextWaypoint,
  PreviousWaypoint,
};

class AlternatesWidget : public BaseAccessPanel, RatchetListLayout,
  private ListControl::Handler
{
protected:
  AbortTask::AlternateVector alternates;
  UPixelScalar font_height;

  /**
   * the list and the three buttons follow the layout calculated in
   * RatchetListLayout
   */
  ListControl *list;
  WndSymbolButton *prev, *next;
  unsigned alternate_index;

public:
  AlternatesWidget(unsigned id)
    :BaseAccessPanel(id) {}

protected:
  void RefreshList();
  void RatchetWaypoint(int offset);
  void UpdateAlternates();
  void CreateList(ContainerWindow &parent, const DialogLook &look,
                  const PixelRect &rc, UPixelScalar row_height);
  void CreateButtons(ContainerWindow &parent, const PixelRect &rc,
                     const DialogLook &dialog_look);

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  /* virtual methods from class List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx);
  virtual bool CanActivateItem(unsigned index) const;
  virtual void OnActivateItem(unsigned index);

  /**
   * Updates the Index in the InfoBox to display the data for the waypoint
   * corresponding to this index.
   */
  virtual void OnCursorMoved(unsigned index);

  /* virtual methods from class ActionListener (for buttons) */
  virtual void OnAction(int id);

  /**
   * updates the index displayed by the InfoBox
   */
  void ProcessQuickAccess(unsigned value);

  /**
   * returns the index currently displayed by the InfoBox
   */
  unsigned GetQuickAccess();
};

unsigned
AlternatesWidget::GetQuickAccess()
{
  return InfoBoxManager::GetQuickAccess(id);
}

void
AlternatesWidget::ProcessQuickAccess(unsigned value)
{
  assert(value < alternates.size());
  StaticString<8> tmp;
  tmp.UnsafeFormat(_T("%u"), value);
  InfoBoxManager::ProcessQuickAccess(id, tmp.c_str());
}

void
AlternatesWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  RatchetListLayout::Prepare(parent, content_rc);

  UpdateAlternates();

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  UPixelScalar margin = Layout::Scale(2);
  font_height = dialog_look.list.font->GetHeight();
  CreateList(GetClientAreaWindow(), dialog_look, content_rc, 3 * margin + 2 * font_height);
  alternate_index = GetQuickAccess();
  RefreshList();

  CreateButtons(GetClientAreaWindow(), content_rc, dialog_look);
}

void
AlternatesWidget::CreateList(ContainerWindow &parent, const DialogLook &look,
                               const PixelRect &rc, UPixelScalar row_height)
{
  WindowStyle list_style;
  list_style.TabStop();
  list_style.Border();

  list = new ListControl(parent, look, ratchet_list_rc, list_style, row_height);
  list->SetHandler(this);
  list->SetHasScrollBar(false);
  list->SetLength(alternates.size());

  assert(list);
}

void
AlternatesWidget::CreateButtons(ContainerWindow &parent, const PixelRect &rc,
                                  const DialogLook &dialog_look)
{
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();

  prev = new WndSymbolButton(parent, dialog_look,
                             _T("^"), ratchet_up_rc,
                             button_style,
                             this, PreviousWaypoint);

  next = new WndSymbolButton(parent, dialog_look,
                             _T("v"), ratchet_down_rc,
                             button_style,
                             this, NextWaypoint);
}

void
AlternatesWidget::Unprepare()
{
  delete prev;
  delete next;
  delete list;

  BaseAccessPanel::Unprepare();
}

void
AlternatesWidget::RefreshList()
{
  list->SetCursorIndex(alternate_index);
  list->Invalidate();
}

void
AlternatesWidget::RatchetWaypoint(int offset)
{
  unsigned old_index = list->GetCursorIndex();
  unsigned i = max(0, (int)old_index + offset);
  alternate_index = min(i, (unsigned)alternates.size() - 1u);
  RefreshList();
}

void
AlternatesWidget::OnActivateItem(unsigned index)
{
  Close();
}

bool
AlternatesWidget::CanActivateItem(unsigned index) const {
  return true;
}

void
AlternatesWidget::OnCursorMoved(unsigned index)
{
  ProcessQuickAccess(index);
}

void
AlternatesWidget::UpdateAlternates()
{
  ProtectedTaskManager::Lease lease(*protected_task_manager);
  alternates = lease->GetAlternates();
}

void
AlternatesWidget::OnPaintItem(Canvas &canvas, const PixelRect rc, unsigned i)
{
  assert(i < alternates.size());

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const Waypoint &waypoint = alternates[i].waypoint;
  const GlideResult& solution = alternates[i].solution;

  WaypointListRenderer::Draw(canvas, rc, waypoint, solution.vector.distance,
                             solution.SelectAltitudeDifference(settings.task.glide),
                             UIGlobals::GetDialogLook(),
                             UIGlobals::GetMapLook().waypoint,
                             CommonInterface::GetMapSettings().waypoint);
}

void
AlternatesWidget::OnAction(int id)
{
  switch (id) {
  case NextWaypoint:
    RatchetWaypoint(1);
    break;

  case PreviousWaypoint:
    RatchetWaypoint(-1);
    break;

  default:
    BaseAccessPanel::OnAction(id);
  }
}

Widget *
LoadAlternatesWidget(unsigned id)
{
  if (protected_task_manager == NULL)
    return NULL;

  if (way_points.IsEmpty())
    return NULL;
  else
    return new AlternatesWidget(id);
}
