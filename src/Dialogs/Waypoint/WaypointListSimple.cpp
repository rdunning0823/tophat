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

#include "WaypointDialogs.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Canvas.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/CheckBox.hpp"
#include "Form/SymbolButton.hpp"
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

#include <algorithm>
#include <list>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static GeoPoint location;
static WndForm *dialog = nullptr;
static ListControl *waypoint_list_control = nullptr;
static WndFrame *summary_labels1;
static WndFrame *summary_values1;
static WndFrame *summary_labels2;
static WndFrame *summary_values2;
static CheckBox *distance_sort_checkbox;
static WndSymbolButton *search_button;

static OrderedTask *ordered_task;
static unsigned ordered_task_index;

static constexpr unsigned distance_filter_items[] = {
  0, 25, 50, 75, 100, 150, 250, 500, 1000
};

static constexpr int direction_filter_items[] = {
  -1, -1, 0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330
};

static Angle last_heading = Angle::Zero();

enum SortDirection {
  NAME,
  DISTANCE,
  BEARING,
};

static SortDirection sort_direction;

struct WaypointListDialogState
{
  StaticString<WaypointFilter::NAME_LENGTH + 1> name;

  int distance_index;
  int direction_index;
  TypeFilter type_index;

  bool IsDefined() const {
    return !name.empty() || distance_index > 0 ||
      direction_index > 0 || type_index != TypeFilter::ALL;
  }

  void ToFilter(WaypointFilter &filter, Angle heading) const {
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

class WaypointListSimpleDialog : public ListItemRenderer,
                                 public ListCursorHandler {
public:
  /* virtual methods from ListItemRenderer */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /* virtual methods from ListCursorHandler */
  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }

  virtual void OnActivateItem(unsigned index) override;

  virtual void OnCursorMoved(unsigned index) override;

};

static WaypointListDialogState dialog_state;
static WaypointList waypoint_list;

static void
PrepareData()
{
  sort_direction = SortDirection::NAME;
}

static void
FillList(WaypointList &list, const Waypoints &src,
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
    if (sort_direction == SortDirection::NAME)
      list.SortByName();
    UISettings &ui_settings = CommonInterface::SetUISettings();
    if (ui_settings.show_waypoints_list_warning) {
      StaticString<99> message;
      message.Format(_T("%s %u %s"), _("Only the nearest"), MAX_LIST_SIZE, _("waypoints will be displayed"));
      ShowMessageBox(message.buffer(),  _("Too many waypoints"), MB_OK);
      ui_settings.show_waypoints_list_warning = false;
      Profile::Set(ProfileKeys::ShowWaypointListWarning,
                   ui_settings.show_waypoints_list_warning);
    }
  } else {

    switch (sort_direction) {
    case SortDirection::NAME:
      break;
    case SortDirection::DISTANCE:
      list.SortByDistance(location);
      break;
    case SortDirection::BEARING:
      break;
    }
  }
}

static void
UpdateButtons()
{
  distance_sort_checkbox->SetState(sort_direction == SortDirection::DISTANCE);

  if (dialog_state.name.empty())
    search_button->SetCaption(_T("Search"));
  else
    search_button->SetCaption(_T("SearchChecked"));
}

void
WaypointListSimpleDialog::OnCursorMoved(gcc_unused unsigned i)
{
  StaticString<100> sunset_buffer;
  StaticString<100> alt_buffer;

  if (waypoint_list.size() == 0)
    return;

  const MoreData &more_data = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const NMEAInfo &basic = CommonInterface::Basic();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  const struct WaypointListItem &info = waypoint_list[i];

  alt_buffer.clear();
  sunset_buffer.clear();
  if (basic.location_available && more_data.NavAltitudeAvailable() &&
      settings.polar.glide_polar_task.IsValid()) {
    const GlideState glide_state(
      basic.location.DistanceBearing(info.waypoint->location),
      info.waypoint->elevation + settings.task.safety_height_arrival,
      more_data.nav_altitude,
      calculated.GetWindOrZero());

    const GlideResult &result =
      MacCready::Solve(settings.task.glide,
                       settings.polar.glide_polar_task,
                       glide_state);
    FormatRelativeUserAltitude(result.pure_glide_altitude_difference,
                               alt_buffer.buffer(), true);
  }

  if (basic.time_available && basic.date_time_utc.IsDatePlausible()) {
    const SunEphemeris::Result sun =
      SunEphemeris::CalcSunTimes(info.waypoint->location, basic.date_time_utc,
                                 settings.utc_offset);

    const unsigned sunset_hour = (int)sun.time_of_sunset;
    const unsigned sunset_minute = (int)((sun.time_of_sunset - fixed(sunset_hour)) * 60);

    sunset_buffer.UnsafeFormat(_T("%02u:%02u"), sunset_hour, sunset_minute);
  }

  summary_labels1->SetCaption(_T("Alt. diff:"));
  summary_values1->SetCaption(alt_buffer.c_str());
  summary_labels2->SetCaption(_T("Sunset:"));
  summary_values2->SetCaption(sunset_buffer.c_str());
}

static void
UpdateList()
{
  waypoint_list.clear();

  FillList(waypoint_list, way_points, location, last_heading,
           dialog_state);

  waypoint_list_control->SetLength(std::max(1, (int)waypoint_list.size()));
  waypoint_list_control->SetOrigin(0);
  waypoint_list_control->SetCursorIndex(1);
  waypoint_list_control->SetCursorIndex(0);
  UpdateButtons();
  waypoint_list_control->Invalidate();
}

static const TCHAR *
WaypointNameAllowedCharacters(const TCHAR *prefix)
{
  static TCHAR buffer[256];
  return way_points.SuggestNamePrefix(prefix, buffer, ARRAY_SIZE(buffer));
}

static void
OnSearchClicked(gcc_unused WndButton &button)
{
  TCHAR new_name_filter[WaypointFilter::NAME_LENGTH + 1];
  CopyString(new_name_filter, dialog_state.name.c_str(),
             WaypointFilter::NAME_LENGTH + 1);

  TextEntryDialog(new_name_filter, WaypointFilter::NAME_LENGTH,
                  _("Waypoint name"),
                  WaypointNameAllowedCharacters);

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

static void
OnWaypointListEnter()
{
  if (waypoint_list.size() > 0)
    dialog->SetModalResult(mrOK);
  else
    OnSearchClicked(*search_button);
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
                    dialog_state.IsDefined() || way_points.IsEmpty() ?
                _ ("No Match!") : _("Too many points. Use Search or click here"));
    return;
  }

  assert(i < waypoint_list.size());

  const struct WaypointListItem &info = waypoint_list[i];

  WaypointListRenderer::Draw(canvas, rc, *info.waypoint,
                             info.GetVector(location),
                             UIGlobals::GetDialogLook(),
                             UIGlobals::GetMapLook().waypoint,
                             CommonInterface::GetMapSettings().waypoint);
}

void
WaypointListSimpleDialog::OnActivateItem(unsigned index)
{
  OnWaypointListEnter();
}

static void
OnByDistanceClicked(CheckBoxControl &control)
{
  if (control.GetState())
    sort_direction = SortDirection::DISTANCE;
  else
    sort_direction = SortDirection::NAME;
  UpdateList();
}

static void
OnByBearingClicked(gcc_unused WndButton &button)
{
  sort_direction = SortDirection::BEARING;
  UpdateList();
}

static void
OnSelectClicked(gcc_unused WndButton &button)
{
  OnWaypointListEnter();
}

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  dialog->SetModalResult(mrCancel);
}

static constexpr CallBackTableEntry callback_table[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnSelectClicked),
  DeclareCallBackEntry(OnByDistanceClicked),
  DeclareCallBackEntry(OnByBearingClicked),
  DeclareCallBackEntry(OnSearchClicked),
  DeclareCallBackEntry(nullptr)
};

const Waypoint*
ShowWaypointListSimpleDialog(const GeoPoint &_location,
                             OrderedTask *_ordered_task, unsigned _ordered_task_index,
                             bool goto_button)
{
  dialog = LoadDialog(callback_table, UIGlobals::GetMainWindow(),
                      Layout::landscape ?
      _T("IDR_XML_WAYPOINTSELECTSIMPLE_L") : _T("IDR_XML_WAYPOINTSELECTSIMPLE"));
  assert(dialog != nullptr);

#ifdef GNAV
  dialog->SetKeyDownNotify(FormKeyDown);
#endif

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  WaypointListSimpleDialog dialog2;

  waypoint_list_control = (ListControl*)dialog->FindByName(_T("frmWaypointList"));
  assert(waypoint_list_control != nullptr);
  waypoint_list_control->SetItemRenderer(&dialog2);
  waypoint_list_control->SetCursorHandler(&dialog2);
  waypoint_list_control->SetItemHeight(WaypointListRenderer::GetHeight(dialog_look));
  if (goto_button) {
    WndButton *button_select = (WndButton*)dialog->FindByName(_T("cmdSelect"));
    assert (button_select != nullptr);
    button_select->SetCaption(_T("Goto"));
  }

  summary_labels1 = (WndFrame *)dialog->FindByName(_T("frmSummaryLabels1"));
  summary_values1 = (WndFrame *)dialog->FindByName(_T("frmSummaryValues1"));
  summary_labels2 = (WndFrame *)dialog->FindByName(_T("frmSummaryLabels2"));
  summary_values2 = (WndFrame *)dialog->FindByName(_T("frmSummaryValues2"));
  assert(summary_labels1 != nullptr);
  assert(summary_values1 != nullptr);
  assert(summary_labels2 != nullptr);
  assert(summary_values2 != nullptr);

  distance_sort_checkbox = (CheckBox*)dialog->FindByName(_T("chkbByDistance"));
  search_button = (WndSymbolButton*)dialog->FindByName(_T("cmdSearch"));
  assert(distance_sort_checkbox != nullptr);
  assert(search_button != nullptr);

  dialog_state.name = _T("");

  location = _location;
  ordered_task = _ordered_task;
  ordered_task_index = _ordered_task_index;
  last_heading = CommonInterface::Basic().attitude.heading;

  PrepareData();
  UpdateList();

  if (dialog->ShowModal() != mrOK) {
    delete dialog;
    return nullptr;
  }

  unsigned index = waypoint_list_control->GetCursorIndex();

  delete dialog;

  const Waypoint* retval = nullptr;

  if (index < waypoint_list.size())
    retval = waypoint_list[index].waypoint;

  return retval;
}
