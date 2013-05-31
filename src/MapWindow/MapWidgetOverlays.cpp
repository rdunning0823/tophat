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

#include "MapWidgetOverlays.hpp"
#include "Widgets/MapOverlayWidget.hpp"
#include "Form/Widget.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "Interface.hpp"
#include "UISettings.hpp"

#include <assert.h>


MapWidgetOverlays::~MapWidgetOverlays()
{
   for (const auto i : widget_list) {
     Widget *widget = (Widget*)i;
     widget->Leave();
     widget->Hide();
     widget->Unprepare();
     delete widget;
   }
}

void
MapWidgetOverlays::Add(Widget *widget, const PixelRect &rc_map) {
  assert(widget != NULL);
  assert (!widget_list.full());
  widget_list.append(widget);
}

void
MapWidgetOverlays::Move(PixelRect rc_map)
{
  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    if (widget->IsVisible())
      widget->Move(rc_map);
  }
}

UPixelScalar
MapWidgetOverlays::HeightFromTop()
{
  UPixelScalar max_y = 0u;

  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    max_y = max(widget->HeightFromTop(), max_y);
  }
  return max_y;
}

UPixelScalar
MapWidgetOverlays::HeightFromBottomRight()
{
  UPixelScalar max_y = 0u;

  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    max_y = max(widget->HeightFromBottomRight(), max_y);
  }
  return max_y;
}

UPixelScalar
MapWidgetOverlays::HeightFromBottomLeft()
{
  UPixelScalar max_y = 0u;

  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    max_y = max(widget->HeightFromBottomLeft(), max_y);
  }
  return max_y;
}

UPixelScalar
MapWidgetOverlays::HeightFromBottomMax()
{
  return max(HeightFromBottomLeft(), HeightFromBottomRight());
}

void MapWidgetOverlays::Initialise(ContainerWindow &parent, const PixelRect &rc)
{
  for (const auto i : widget_list) {
    Widget *widget = (Widget*)i;
    widget->Initialise(parent, rc);
  }
}

void MapWidgetOverlays::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  for (const auto i : widget_list) {
    Widget *widget = (Widget*)i;
    widget->Prepare(parent, rc);
  }
}

void MapWidgetOverlays::Unprepare()
{
  for (const auto i : widget_list) {
    Widget *widget = (Widget*)i;
    widget->Unprepare();
  }
}

void
MapWidgetOverlays::UpdateVisibility(const PixelRect &rc_full_screen,
                                    bool is_panning,
                                    bool is_main_window_widget,
                                    bool is_map,
                                    bool is_full_screen)
{

  const UISettings &ui_settings = CommonInterface::GetUISettings();
  const InfoBoxLayout::Layout ib_layout =
    InfoBoxLayout::Calculate(rc_full_screen, ui_settings.info_boxes.geometry);

  const PixelRect &rc_current = is_full_screen ? rc_full_screen :
    ib_layout.remaining;

  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    widget->UpdateVisibility(rc_current, is_panning, is_main_window_widget,
                             is_map);
    if (widget->IsVisible())
      Move(rc_current);
  }
}

void MapWidgetOverlays::Show(const PixelRect &rc)
{
  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    if (!widget->IsVisible())
      widget->Show(rc);
  }
}

bool MapWidgetOverlays::Leave()
{
  bool ret_val = true;
  for (const auto i : widget_list) {
    Widget *widget = (Widget*)i;
    ret_val &= widget->Leave();
  }
  return ret_val;
}

void MapWidgetOverlays::Hide()
{
  for (const auto i : widget_list) {
    MapOverlayWidget *widget = (MapOverlayWidget*)i;
    if (widget->IsVisible())
      widget->Hide();
  }
}
