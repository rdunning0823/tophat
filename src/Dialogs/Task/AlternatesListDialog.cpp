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

#include "AlternatesListDialog.hpp"
#include "TaskDialogs.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Font.hpp"
#include "Look/DialogLook.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Look/MapLook.hpp"
#include "Renderer/WaypointListRenderer.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Widget/TwoWidgets.hpp"
#include "Widget/TextWidget.hpp"
#include "Engine/Task/TaskBehaviour.hpp"
#include "Widget/CheckBoxWidget.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "NMEA/Derived.hpp"

const TCHAR *distance_label_text = N_("Distance");
const TCHAR *arrival_alt_label_text = N_("Arrival alt");

/* used by item Draw() to match column headers with row data */
static unsigned distance_label_width;

void
AlternatesListWidget::OnPaintItem(Canvas &canvas, const PixelRect rc,
                                  unsigned index)
{
  assert(index < alternates.size());

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const Waypoint &waypoint = alternates[index].waypoint;
  const GlideResult& solution = alternates[index].solution;

  WaypointListRenderer::Draw3(canvas, rc, waypoint, solution.vector.distance,
                              solution.SelectAltitudeDifference(settings.task.glide),
                              UIGlobals::GetDialogLook(),
                              UIGlobals::GetMapLook().waypoint,
                              CommonInterface::GetMapSettings().waypoint,
                              distance_label_width);
}

void
AlternatesListWidget::Update()
{
  ProtectedTaskManager::Lease lease(*protected_task_manager);
  alternates = lease->GetAlternates();
}

void
AlternatesListWidget::CreateButtons(WidgetDialog &dialog)
{
  cancel_button = dialog.AddSymbolButton(_T("_X"), mrCancel);
  goto_button = dialog.AddButton(_("Goto"), *this, GOTO);
  details_button = dialog.AddButton(_("Details"), mrOK);
}

void
AlternatesListWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  UPixelScalar item_height = dialog_look.list.font_bold->GetHeight()
    + Layout::Scale(6) + dialog_look.small_font.GetHeight();
  assert(item_height > 0);

  CreateList(parent, dialog_look, rc, item_height);

  GetList().SetLength(alternates.size());
}

void
AlternatesListWidget::OnActivateItem(unsigned index)
{
  details_button->Click();
}

void
AlternatesListWidget::OnAction(int id)
{
  switch (id) {
  case GOTO:
    unsigned index = GetCursorIndex();

    auto const &item = alternates[index];
    auto const &waypoint = item.waypoint;

    protected_task_manager->DoGoto(waypoint);
    cancel_button->Click();

    break;
  }
}

void
AlternatesListWidgetNoButtons::Move(const PixelRect &rc)
{
  AlternatesListWidget::Move(rc);
}

void
AlternatesListWidgetNoButtons::Refresh()
{
  AlternatesListWidget::Update();
  GetList().SetLength(alternates.size());
  GetList().Invalidate();
}

void
AlternatesListWidgetNoButtons::OnActivateItem(unsigned index)
{
  if (DoDetails())
    form->SetModalResult(mrOK);
}

const Waypoint*
AlternatesListWidgetNoButtons::GetWaypoint()
{
  unsigned index = GetCursorIndex();
  if (index >= alternates.size())
    return nullptr;
  auto const &item = alternates[index];
  auto const &waypoint = item.waypoint;
  return &waypoint;
}

bool
AlternatesListWidgetNoButtons::DoGoto()
{
  const Waypoint *waypoint = GetWaypoint();
  if (waypoint != nullptr) {
    protected_task_manager->DoGoto(*waypoint);
    return true;
  }
  return false;
}

bool
AlternatesListWidgetNoButtons::DoDetails()
{
  const Waypoint *waypoint = GetWaypoint();
  if (waypoint != nullptr) {
    dlgWaypointDetailsShowModal(*waypoint, true, false);
    return true;
  }
  return false;
}
/**
 * *******************************************
 * methods for AlternatListHeaderWidget
 * *******************************************
 */

/**
 * @return a CheckBox widget if nonlandable airports exist, else an empty TextWidget
 */
static Widget*
CreateFirstWidget(AlternatesListHeaderWidget &listener, const DerivedInfo& calculated)
{
  if (calculated.common_stats.has_non_airfield_landables)
        return new CheckBoxWidget(UIGlobals::GetDialogLook(), _("Airports only"), listener, AlternatesListHeaderWidget::AirFieldsOnly);
        return new TextWidget();
}

AlternatesListHeaderWidget::AlternatesListHeaderWidget(const DerivedInfo& _calculated)
  :TwoWidgets(CreateFirstWidget(*this, _calculated),
              new TextWidget(), false, true), settings_computer(CommonInterface::SetComputerSettings())
{}

void
AlternatesListHeaderWidget::CalculateLayout(const PixelRect &rc)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  PixelSize sz_space = dialog_look.text_font.TextSize(_T("     "));
  distance_label_width =
      dialog_look.text_font.TextSize(gettext(distance_label_text)).cx + sz_space.cx;

  StaticString<1000> caption;
  caption.Format(_T("%s     %s"),gettext(distance_label_text),
                 gettext(arrival_alt_label_text));

  ((TextWidget&)GetSecond()).SetText(caption.c_str());

  GetAirfieldsCheckbox().SetState(settings_computer.task.abort_task_airfield_only);
}

void
AlternatesListHeaderWidget::Move(const PixelRect &rc)
{
  TwoWidgets::Move(rc);
  CalculateLayout(rc);
}

void
AlternatesListHeaderWidget::Prepare(ContainerWindow &parent,
                                    const PixelRect &rc)
{
  TwoWidgets::Prepare(parent, rc);
  CalculateLayout(rc);
}

void
AlternatesListHeaderWidget::Show(const PixelRect &rc)
{
  TwoWidgets::Show(rc);
}

void
AlternatesListHeaderWidget::Unprepare()
{
  TwoWidgets::Unprepare();
}

void
AlternatesListHeaderWidget::OnAction(int id)
{
  switch (id) {
  case Buttons::AirFieldsOnly:
    UpdateAirfieldsOnly(GetAirfieldsCheckbox().GetState());
    break;
  }
}

CheckBoxWidget &
AlternatesListHeaderWidget::GetAirfieldsCheckbox()
{
  return (CheckBoxWidget&)TwoWidgets::GetFirst();
}

void
AlternatesListHeaderWidget::UpdateAirfieldsOnly(bool airfields_only)
{
  settings_computer.task.abort_task_airfield_only = airfields_only;
}

void
dlgAlternatesListShowModal()
{
  if (protected_task_manager == nullptr)
    return;

  const DerivedInfo& calculated = CommonInterface::Calculated();
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  AlternatesListWidgetNoButtons *widget = new AlternatesListWidgetNoButtons(dialog_look);
  widget->Update();
  TwoWidgets *two_widgets = new TwoWidgets(new AlternatesListHeaderWidget(calculated),
                                           widget);

  WidgetDialog dialog(dialog_look);
  widget->SetForm(&dialog);
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Alternates"), two_widgets);
  widget->CreateButtons(dialog);

  int i = dialog.ShowModal() == mrOK
    ? (int)widget->GetCursorIndex()
    : -1;
  dialog.StealWidget();

  if (i < 0 || (unsigned)i >= widget->alternates.size())
    return;

  dlgWaypointDetailsShowModal(widget->alternates[i].waypoint);
  delete two_widgets;
}
