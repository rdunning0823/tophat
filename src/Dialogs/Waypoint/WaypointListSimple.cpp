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

#include "WaypointDialogs.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Canvas.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/CheckBox.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"
#include "Form/List.hpp"
#include "Form/Frame.hpp"
#include "Form/Draw.hpp"
#include "Profile/Profile.hpp"
#include "Waypoint/WaypointList.hpp"
#include "Waypoint/WaypointListBuilder.hpp"
#include "Waypoint/WaypointFilter.hpp"
#include "Waypoint/Waypoints.hpp"
#include "Components.hpp"
#include "Compiler.h"
#include "LogFile.hpp"
#include "UIGlobals.hpp"
#include "Look/MapLook.hpp"
#include "Look/DialogLook.hpp"
#include "Util/Macros.hpp"
#include "Renderer/WaypointListRenderer.hpp"
#include "Units/Units.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "GlideSolvers/GlideState.hpp"
#include "GlideSolvers/MacCready.hpp"
#include "Math/SunEphemeris.hpp"
#include "Dialogs/Message.hpp"
#include "Widget/Widget.hpp"
#include "Widget/ManagedWidget.hpp"
#include "Screen/SingleWindow.hpp"

#include <algorithm>
#include <list>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

enum ControlIndex {
  Search,
  NameHeader,
  ElevationHeader,
  DistanceHeader,
  List,
  Select,
  Close,
};

enum Actions {
  SearchClick,
  NameHeaderClick,
  ElevationHeaderClick,
  DistanceHeaderClick,
  SelectClick,
  CloseClick,
};

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

static constexpr unsigned distance_filter_items[] = {
  0, 25, 50, 75, 100, 150, 250, 500, 1000
};

static constexpr int direction_filter_items[] = {
  -1, -1, 0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330
};

class WaypointListSimpleDialog : public NullWidget, public WndForm,
  public ListItemRenderer, public ListCursorHandler
{

protected:
  struct WaypointListDialogState
  {
    StaticString<WaypointFilter::NAME_LENGTH + 1> name;

    int distance_index;
    int direction_index;
    TypeFilter type_index;

    WaypointListDialogState(): distance_index(0), direction_index(0), type_index(TypeFilter::ALL) {}

    bool IsDefined() const {
      return !name.empty() || distance_index > 0 ||
        direction_index > 0 || type_index != TypeFilter::ALL;
    }

    void ToFilter(WaypointFilter &filter, Angle heading) const {
      assert(distance_index == 0);
      assert(direction_index == 0);
      assert(type_index == TypeFilter::ALL);

      filter.name = name;
      filter.distance =
        Units::ToSysDistance(fixed(distance_filter_items[distance_index]));
      filter.type_index = type_index;

      if (direction_index != 1)
        filter.direction = Angle::Degrees(
            fixed(direction_filter_items[direction_index]));
      else
        filter.direction = heading;
    }
  };

  /* display Goto on Select button */
  bool goto_mode;
  GeoPoint location;
  OrderedTask *ordered_task;
  unsigned ordered_task_index;
  Angle last_heading;

  WaypointListDialogState dialog_state;
  WaypointList waypoint_list;
  UISettings::WaypointSortDirection sort_direction;

  ListControl waypoint_list_control;
  WndSymbolButton search_button;
  WndSymbolButton name_header;
  WndSymbolButton elevation_header;
  WndSymbolButton distance_header;
  WndSymbolButton select_button;
  WndSymbolButton cancel_button;

  PixelRect rc_list;
  PixelRect rc_search_button;
  PixelRect rc_name_header;
  PixelRect rc_elevation_header;
  PixelRect rc_distance_header;
  PixelRect rc_select_button;
  PixelRect rc_cancel_button;

public:
  WaypointListSimpleDialog(const GeoPoint &_location,
                           OrderedTask *_ordered_task, unsigned _ordered_task_index,
                           bool _goto_mode)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _("Select Waypoint"), GetDialogStyle()),
   goto_mode(_goto_mode), location(_location),
   ordered_task(_ordered_task),
   ordered_task_index(_ordered_task_index),
   last_heading(CommonInterface::Basic().attitude.heading),
   waypoint_list_control(UIGlobals::GetDialogLook()) {};

  /* virtual methods from ListItemRenderer */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /* virtual methods from ListCursorHandler */
  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }

  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override {};
  virtual void Move(const PixelRect &rc) override;

  /* virtual methods from ActionListener */
  virtual void OnAction(int id) override;

  /* overrides from WndForm */
  virtual void OnResize(PixelSize new_size) override;
  virtual void ReinitialiseLayout(const PixelRect &parent_rc) override;

  /* virtual methods from ListCursorHandler */
  virtual void OnActivateItem(unsigned index) override;

  void OnSearchClicked();
  fixed CalcAltitudeDifferential(const Waypoint &waypoint);
  const Waypoint* GetSelectedWaypoint();
  void ItemSelected(unsigned index);
  void SaveSortDirection();
  void FillList(WaypointList &list, const Waypoints &src,
                GeoPoint location, Angle heading,
                const WaypointListDialogState &state);

  void SetRectangles(const PixelRect &rc_outer);
  void UpdateButtons();
  void UpdateList();
  void UpdateHeaders();

};

void
WaypointListSimpleDialog::OnAction(int id)
{
  switch(id) {
  case SearchClick:
    OnSearchClicked();
    break;
  case NameHeaderClick:
    sort_direction = UISettings::WaypointSortDirection::NAME;
    UpdateList();
    break;
  case ElevationHeaderClick:
    sort_direction = UISettings::WaypointSortDirection::ARRIVAL_ALTITUDE;
    UpdateList();
    break;
  case DistanceHeaderClick:
    sort_direction = UISettings::WaypointSortDirection::DISTANCE;
    UpdateList();
    break;
  case SelectClick:
    ItemSelected(waypoint_list_control.GetCursorIndex());
    break;
  case CloseClick:
   SetModalResult(mrCancel);
    break;
  }
}

void
WaypointListSimpleDialog::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc = WndForm::GetClientRect();
  rc.bottom -= WndForm::GetTitleHeight();

  unsigned control_height = Layout::Scale(35);
  if (Layout::landscape) {

    const unsigned left_col_width = Layout::Scale(65);

    rc_list.left = left_col_width;
    rc_list.right = rc.right;
    rc_list.top = control_height;
    rc_list.bottom = rc.bottom;

    rc_search_button.left = 0;
    rc_search_button.right = left_col_width;
    rc_search_button.top = 0;
    rc_search_button.bottom = control_height;

    rc_name_header.top = 0;
    rc_name_header.bottom = control_height;

    rc_elevation_header.top = 0;
    rc_elevation_header.bottom = control_height;

    rc_distance_header.top = 0;
    rc_distance_header.bottom = control_height;

    rc_select_button.left = 0;
    rc_select_button.right = left_col_width;
    rc_select_button.bottom = rc.bottom - control_height;
    rc_select_button.top = rc_select_button.bottom - control_height;

    rc_cancel_button.left = 0;
    rc_cancel_button.right = left_col_width;
    rc_cancel_button.bottom = rc.bottom;
    rc_cancel_button.top = rc_cancel_button.bottom - control_height;
  } else {

    rc_list.left = 0;
    rc_list.right = rc.right;
    rc_list.top = Layout::Scale(70);
    rc_list.bottom = rc.bottom - control_height;

    rc_search_button.left = Layout::Scale(160);
    rc_search_button.right = rc_search_button.left + Layout::Scale(80);
    rc_search_button.top = 0;
    rc_search_button.bottom = control_height;

    rc_name_header.top = Layout::Scale(35);
    rc_name_header.bottom = rc_name_header.top + control_height;

    rc_elevation_header.top = Layout::Scale(35);
    rc_elevation_header.bottom = rc_elevation_header.top + control_height;

    rc_distance_header.top = Layout::Scale(35);
    rc_distance_header.bottom = rc_distance_header.top + control_height;

    rc_select_button.left = 0;
    rc_select_button.right = rc.right / 2;
    rc_select_button.bottom = rc.bottom;
    rc_select_button.top = rc_select_button.bottom - control_height;

    rc_cancel_button.left = rc.right / 2;
    rc_cancel_button.right = rc.right;
    rc_cancel_button.bottom = rc.bottom;
    rc_cancel_button.top = rc_cancel_button.bottom - control_height;
  }

  const unsigned padding = Layout::GetTextPadding();

  rc_name_header.left = rc_list.left;
  rc_name_header.right = rc_list.left
      + (rc_list.GetSize().cx - waypoint_list_control.GetScrollBarWidth()) / 2
      - padding;

  rc_distance_header.right = rc_list.right;
  rc_distance_header.left = (rc_name_header.right + rc_distance_header.right) / 2;

  rc_elevation_header.left = rc_name_header.right;
  rc_elevation_header.right = rc_distance_header.left;
}

void
WaypointListSimpleDialog::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const PixelRect rc_form = rc;
  NullWidget::Prepare(parent, rc_form);

  SetCaption(_("Select Waypoint"));
  SetRectangles(rc_form);

  WindowStyle style_frame;

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const ButtonLook &button_look = dialog_look.button;
  WindowStyle button_style;
  button_style.TabStop();

  if (Layout::landscape) {
    cancel_button.Create(GetClientAreaWindow(), button_look, _("Cancel"),
                         rc_cancel_button,
                         button_style, *this, CloseClick);

    waypoint_list_control.Create(GetClientAreaWindow(), rc_list,
                                 button_style,
                                 WaypointListRenderer::GetHeight(dialog_look));

    search_button.Create(GetClientAreaWindow(), button_look, _T(""),
                          rc_search_button,
                          button_style, *this, SearchClick);

    name_header.Create(GetClientAreaWindow(), button_look, _("Name"),
                        rc_name_header,
                        button_style, *this, NameHeaderClick);

    elevation_header.Create(GetClientAreaWindow(), button_look, _("Arriv."),
                             rc_elevation_header,
                             button_style, *this, ElevationHeaderClick);

    distance_header.Create(GetClientAreaWindow(), button_look, _("Dist."),
                           rc_distance_header,
                           button_style, *this, DistanceHeaderClick);

    select_button.Create(GetClientAreaWindow(), button_look,
                         (goto_mode ? _("Goto") : _("Select")),
                          rc_select_button,
                          button_style, *this, SelectClick);

  } else {
    cancel_button.Create(GetClientAreaWindow(), button_look, _("Cancel"),
                         rc_cancel_button,
                         button_style, *this, CloseClick);

    select_button.Create(GetClientAreaWindow(), button_look,
                         (goto_mode ? _("Goto") : _("Select")),
                          rc_select_button,
                          button_style, *this, SelectClick);

    search_button.Create(GetClientAreaWindow(), button_look, _T(""),
                          rc_search_button,
                          button_style, *this, SearchClick);

    name_header.Create(GetClientAreaWindow(), button_look, _("Name"),
                        rc_name_header,
                        button_style, *this, NameHeaderClick);

    elevation_header.Create(GetClientAreaWindow(), button_look, _("Arriv."),
                             rc_elevation_header,
                             button_style, *this, ElevationHeaderClick);

    distance_header.Create(GetClientAreaWindow(), button_look, _("Dist."),
                           rc_distance_header,
                           button_style, *this, DistanceHeaderClick);

    waypoint_list_control.Create(GetClientAreaWindow(), rc_list,
                                 button_style,
                                 WaypointListRenderer::GetHeight(dialog_look));
  }

  search_button.SetPrefixIcon(SymbolButtonRenderer::PrefixIcon::SEARCH);

  waypoint_list_control.SetItemRenderer(this);
  waypoint_list_control.SetCursorHandler(this);

  dialog_state.name = _T("");
  sort_direction = CommonInterface::SetUISettings().waypoint_sort_direction;

  WndForm::Move(rc_form);
  UpdateList();
  UpdateHeaders();
}

void
WaypointListSimpleDialog::FillList(WaypointList &list, const Waypoints &src,
         GeoPoint location, Angle heading, const WaypointListDialogState &state)
{
  WaypointFilter filter;
  state.ToFilter(filter, heading);

  WaypointListBuilder builder(filter, location, list,
                              ordered_task, ordered_task_index);
  builder.Visit(src);
  enum MaxListSize {
    MAX_LIST_SIZE = 425,
  };

  unsigned size = (unsigned)src.size();
  if (!state.IsDefined() && size >= MAX_LIST_SIZE) {
    list.SortByDistance(location);
    list.erase(list.begin() + MAX_LIST_SIZE, list.end());
    UISettings &ui_settings = CommonInterface::SetUISettings();
    if (ui_settings.show_waypoints_list_warning) {
      if (CommonInterface::Calculated().flight.flying == false) {
        StaticString<99> message;
        message.Format(_T("%s %u %s"), _("Only the nearest"), MAX_LIST_SIZE, _("waypoints will be displayed"));
        ShowMessageBox(message.buffer(),  _("Too many waypoints"), MB_OK);
        ui_settings.show_waypoints_list_warning = false;
        Profile::Set(ProfileKeys::ShowWaypointListWarning,
                     ui_settings.show_waypoints_list_warning);
      }
    }
  }

  switch (sort_direction) {
  case UISettings::WaypointSortDirection::NAME:
    if (size >= MAX_LIST_SIZE)
      list.SortByName();
    break;
  case UISettings::WaypointSortDirection::DISTANCE:
    if (size < MAX_LIST_SIZE || !state.name.empty())
      list.SortByDistance(location);
    break;
  case UISettings::WaypointSortDirection::ELEVATION:
    list.SortByElevation();
    break;
  case UISettings::WaypointSortDirection::ARRIVAL_ALTITUDE:
    list.SortByArrivalAltitude(location);
    break;
  case UISettings::WaypointSortDirection::BEARING:
    break;
  }
}

void
WaypointListSimpleDialog::UpdateButtons()
{
  if (dialog_state.name.empty())
    search_button.SetPrefixIcon(SymbolButtonRenderer::PrefixIcon::SEARCH);
  else
    search_button.SetPrefixIcon(SymbolButtonRenderer::PrefixIcon::SEARCH_CHECKED);
}

/**
 * updates text and placement of list headers
 * - Name
 * - Elevation
 * - Dist / Bearing
 */
void
WaypointListSimpleDialog::UpdateHeaders()
{
  SetRectangles(GetClientRect());
  name_header.Move(rc_name_header);
  elevation_header.Move(rc_elevation_header);
  distance_header.Move(rc_distance_header);

  name_header.SetPrefixIcon(sort_direction ==
      UISettings::WaypointSortDirection::NAME ?
          SymbolButtonRenderer::CHECK_MARK : SymbolButtonRenderer::NONE);
  distance_header.SetPrefixIcon(sort_direction ==
      UISettings::WaypointSortDirection::DISTANCE ?
          SymbolButtonRenderer::CHECK_MARK : SymbolButtonRenderer::NONE);
  elevation_header.SetPrefixIcon(sort_direction ==
      UISettings::WaypointSortDirection::ARRIVAL_ALTITUDE ?
          SymbolButtonRenderer::CHECK_MARK : SymbolButtonRenderer::NONE);
}

void
WaypointListSimpleDialog::UpdateList()
{
  waypoint_list.clear();

  FillList(waypoint_list, way_points, location, last_heading,
           dialog_state);

  waypoint_list_control.SetLength(std::max(1, (int)waypoint_list.size()));
  waypoint_list_control.SetOrigin(0);
  waypoint_list_control.SetCursorIndex(1);
  waypoint_list_control.SetCursorIndex(0);

  UpdateButtons();

  waypoint_list_control.Invalidate();
  UpdateHeaders();
}

static const TCHAR *
WaypointNameAllowedCharacters(const TCHAR *prefix)
{
  static TCHAR buffer[256];
  return way_points.SuggestNamePrefix(prefix, buffer, ARRAY_SIZE(buffer));
}

void
WaypointListSimpleDialog::OnSearchClicked()
{
  TCHAR new_name_filter[WaypointFilter::NAME_LENGTH + 1];
  CopyString(new_name_filter, dialog_state.name.c_str(),
             WaypointFilter::NAME_LENGTH + 1);

  TextEntryDialog(new_name_filter, WaypointFilter::NAME_LENGTH,
                  _("Waypoint name"),
                  WaypointNameAllowedCharacters, false);

  int i = _tcslen(new_name_filter) - 1;
  while (i >= 0) {
    if (new_name_filter[i] != _T(' '))
      break;

    new_name_filter[i] = 0;
    i--;
  }

  CopyString(dialog_state.name.buffer(), new_name_filter,
             WaypointFilter::NAME_LENGTH + 1);

  UpdateList();
}

fixed
WaypointListSimpleDialog::CalcAltitudeDifferential(const Waypoint &waypoint)
{
  const MoreData &more_data = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const NMEAInfo &basic = CommonInterface::Basic();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

/*    assert(basic.location_available);
  assert(more_data.NavAltitudeAvailable());
  assert(settings.polar.glide_polar_task.IsValid());*/

  // altitude differential
  const GlideState glide_state(
    basic.location.DistanceBearing(waypoint.location),
    waypoint.elevation + settings.task.safety_height_arrival,
    more_data.nav_altitude,
    calculated.GetWindOrZero());

  const GlideResult &result =
    MacCready::Solve(settings.task.glide,
                     settings.polar.glide_polar_task,
                     glide_state);
  return result.SelectAltitudeDifference(settings.task.glide);
}

void
WaypointListSimpleDialog::OnPaintItem(Canvas &canvas, const PixelRect rc,
                                      unsigned i)
{
  if (waypoint_list.empty()) {
    assert(i == 0);

    const UPixelScalar line_height = rc.bottom - rc.top;
    const DialogLook &look = UIGlobals::GetDialogLook();
    const Font &name_font = *look.list.font;
    canvas.SetTextColor(look.list.GetTextColor(true, true, false));
    canvas.Select(name_font);
    canvas.DrawText(rc.left + Layout::FastScale(2),
                    rc.top + line_height / 2 - name_font.GetHeight() / 2,
                    dialog_state.IsDefined() ?
                _ ("No Match!") : _("Too many points. Use Search or click here"));
    return;
  }

  assert(i < waypoint_list.size());

  const struct WaypointListItem &info = waypoint_list[i];

  WaypointListRenderer::Draw2(canvas, rc, *info.waypoint,
                              &info.GetVector(location),
                              CalcAltitudeDifferential(*info.waypoint),
                              UIGlobals::GetDialogLook(),
                              UIGlobals::GetMapLook().waypoint,
                              CommonInterface::GetMapSettings().waypoint,
                              rc_name_header.GetSize().cx,
                              rc_elevation_header.GetSize().cx,
                              rc_distance_header.GetSize().cx);
}

void
WaypointListSimpleDialog::ItemSelected(unsigned index)
{
  if (waypoint_list.size() > 0)
    SetModalResult(mrOK);
  else
    OnSearchClicked();
}
void
WaypointListSimpleDialog::OnActivateItem(unsigned index)
{
  ItemSelected(index);
}

void
WaypointListSimpleDialog::SaveSortDirection()
{
  if (sort_direction != CommonInterface::GetUISettings().waypoint_sort_direction) {
    CommonInterface::SetUISettings().waypoint_sort_direction = sort_direction;
    Profile::Set(ProfileKeys::WaypointSortDirection, sort_direction);
    Profile::SetModified(true);
  }
}

void
WaypointListSimpleDialog::ReinitialiseLayout(const PixelRect &parent_rc)
{
  WndForm::Move(parent_rc);
}

void
WaypointListSimpleDialog::OnResize(PixelSize new_size)
{
  WndForm::OnResize(new_size);
  SetRectangles(GetClientRect());
  waypoint_list_control.Move(rc_list);
  search_button.Move(rc_search_button);
  name_header.Move(rc_name_header);
  elevation_header.Move(rc_elevation_header);
  distance_header.Move(rc_distance_header);
  select_button.Move(rc_select_button);
  cancel_button.Move(rc_cancel_button);
}

void
WaypointListSimpleDialog::Move(const PixelRect &rc)
{
}

void
WaypointListSimpleDialog::Show(const PixelRect &rc)
{
}

const Waypoint*
WaypointListSimpleDialog::GetSelectedWaypoint()
{
  unsigned index = waypoint_list_control.GetCursorIndex();

  if (index < waypoint_list.size())
    return waypoint_list[index].waypoint;

  return nullptr;
}

const Waypoint*
ShowWaypointListDialog(const GeoPoint &location,
                       OrderedTask *ordered_task, unsigned ordered_task_index,
                       bool goto_mode)
{
  ContainerWindow &w = UIGlobals::GetMainWindow();
  WaypointListSimpleDialog *instance =
      new WaypointListSimpleDialog(location,
                                   ordered_task, ordered_task_index,
                                   goto_mode);

  ManagedWidget managed_widget(w, instance);
  managed_widget.Move(w.GetClientRect());
  managed_widget.Show();
  int result = instance->ShowModal();
  instance->SaveSortDirection();
  return (result == ModalResult::mrCancel) ? nullptr : instance->GetSelectedWaypoint();
}
