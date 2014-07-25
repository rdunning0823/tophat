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
static WndSymbolButton *search_button;
static WndSymbolButton *name_header;
static WndSymbolButton *elevation_header;
static WndSymbolButton *distance_header;
static PixelRect rc_name_header;
static PixelRect rc_elevation_header;
static PixelRect rc_distance_header;
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
  ELEVATION,
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
    UISettings &ui_settings = CommonInterface::SetUISettings();
    if (ui_settings.show_waypoints_list_warning) {
      StaticString<99> message;
      message.Format(_T("%s %u %s"), _("Only the nearest"), MAX_LIST_SIZE, _("waypoints will be displayed"));
      ShowMessageBox(message.buffer(),  _("Too many waypoints"), MB_OK);
      ui_settings.show_waypoints_list_warning = false;
      Profile::Set(ProfileKeys::ShowWaypointListWarning,
                   ui_settings.show_waypoints_list_warning);
    }
  }

  switch (sort_direction) {
  case SortDirection::NAME:
    if (size >= MAX_LIST_SIZE)
      list.SortByName();
    break;
  case SortDirection::DISTANCE:
    if (size < MAX_LIST_SIZE)
      list.SortByDistance(location);
    break;
  case SortDirection::ELEVATION:
    list.SortByElevation();
    break;
  case SortDirection::BEARING:
    break;
  }
}

static void
UpdateButtons()
{
  if (dialog_state.name.empty())
    search_button->SetCaption(_T("Search"));
  else
    search_button->SetCaption(_T("SearchChecked"));
}

/**
 * updates text and placement of list headers
 * - Name
 * - Elevation
 * - Dist / Bearing
 */
static void
UpdateHeaders()
{
  PixelRect rc_list = waypoint_list_control->GetPosition();
  const unsigned padding = Layout::GetTextPadding();

  const TCHAR *elevation_header_text = _("Elev.");
  const TCHAR *distance_header_text = _("Dist.");

  rc_name_header = name_header->GetPosition();
  rc_elevation_header = elevation_header->GetPosition();
  rc_distance_header = distance_header->GetPosition();

  rc_name_header.left = rc_list.left;
  rc_name_header.right = rc_list.left
      + (rc_list.GetSize().cx - waypoint_list_control->GetScrollBarWidth()) / 2
      - padding;

  rc_distance_header.right = rc_list.right;
  rc_distance_header.left = (rc_name_header.right + rc_distance_header.right) / 2;

  rc_elevation_header.left = rc_name_header.right;
  rc_elevation_header.right = rc_distance_header.left;

  name_header->Move(rc_name_header);
  elevation_header->Move(rc_elevation_header);
  distance_header->Move(rc_distance_header);

  name_header->SetCaption(sort_direction == SortDirection::NAME ?
      _("_chkmark_Name") : _("Name"));
  elevation_header->SetCaption(sort_direction == SortDirection::ELEVATION ?
      _("_chkmark_Elev.") : elevation_header_text);
  distance_header->SetCaption(sort_direction == SortDirection::DISTANCE ?
      _("_chkmark_Dist.") : distance_header_text);
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
  UpdateHeaders();
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

  WaypointListRenderer::Draw2(canvas, rc, *info.waypoint,
                              info.GetVector(location),
                              UIGlobals::GetDialogLook(),
                              UIGlobals::GetMapLook().waypoint,
                              CommonInterface::GetMapSettings().waypoint,
                              rc_elevation_header.GetSize().cx,
                              rc_distance_header.GetSize().cx);
}

void
WaypointListSimpleDialog::OnActivateItem(unsigned index)
{
  OnWaypointListEnter();
}

static void
OnByDistanceClicked(gcc_unused WndButton &button)
{
  sort_direction = SortDirection::DISTANCE;
  UpdateList();
}

static void
OnByBearingClicked(gcc_unused WndButton &button)
{
  sort_direction = SortDirection::BEARING;
  UpdateList();
}

static void
OnByElevationClicked(gcc_unused WndButton &button)
{
  sort_direction = SortDirection::ELEVATION;
  UpdateList();
}

static void
OnByNameClicked(gcc_unused WndButton &button)
{
  sort_direction = SortDirection::NAME;
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
  DeclareCallBackEntry(OnByNameClicked),
  DeclareCallBackEntry(OnByElevationClicked),
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

  name_header = (WndSymbolButton *)dialog->FindByName(_T("btnNameHeader"));
  elevation_header = (WndSymbolButton *)dialog->FindByName(_T("btnElevationHeader"));
  distance_header = (WndSymbolButton *)dialog->FindByName(_T("btnDistanceHeader"));
  assert(name_header != nullptr);
  assert(elevation_header != nullptr);
  assert(distance_header != nullptr);

  search_button = (WndSymbolButton*)dialog->FindByName(_T("cmdSearch"));
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
