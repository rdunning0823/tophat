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

#include "Dialogs/GeoPointEntry.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/FixedWindowWidget.hpp"
#include "Widget/TwoWidgets.hpp"
#include "Form/DigitEntry.hpp"
#include "Form/LambdaActionListener.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Geo/GeoPoint.hpp"
#include "Screen/Layout.hpp"

bool
GeoPointEntryDialog(const TCHAR *caption, GeoPoint &value,
                    const CoordinateFormat format,
                    bool nullable)
{
  /* create the dialog */

  const DialogLook &look = UIGlobals::GetDialogLook();

  WidgetDialog dialog(look);
  dialog.CreatePreliminaryFull(UIGlobals::GetMainWindow(), caption);

  ContainerWindow &client_area = dialog.GetClientAreaWindow();

  /* create the input control */

  WindowStyle control_style;
  control_style.Hide();
  control_style.TabStop();

  PixelRect rc_top, rc_bottom, rc_content;
  rc_content = client_area.GetClientRect();
  rc_content.bottom -= Layout::GetMaximumControlHeight(); // button
  rc_top = rc_bottom = rc_content;
  rc_top.bottom = rc_bottom.top = rc_content.GetSize().cy / 2;


  DigitEntry latitude_entry(look);
  latitude_entry.CreateLatitude(client_area, rc_top,
                                control_style, format);
  latitude_entry.Resize(latitude_entry.GetRecommendedSize());
  latitude_entry.SetActionListener(dialog, mrOK);

  DigitEntry longitude_entry(look);
  longitude_entry.CreateLongitude(client_area, rc_bottom,
                                  control_style, format);
  longitude_entry.Resize(longitude_entry.GetRecommendedSize());
  longitude_entry.SetActionListener(dialog, mrOK);

  if (value.IsValid()) {
    latitude_entry.SetLatitude(value.latitude, format);
    longitude_entry.SetLongitude(value.longitude, format);
  } else {
    latitude_entry.SetInvalid();
    longitude_entry.SetInvalid();
  }

  /* create buttons */

  dialog.AddSymbolButton(_T("_X"), dialog, mrOK);
  dialog.AddButton(_("Cancel"), dialog, mrCancel);

  auto clear_listener = MakeLambdaActionListener([&latitude_entry,
                                                  &longitude_entry](unsigned){
      latitude_entry.SetInvalid();
      longitude_entry.SetInvalid();
    });
  if (nullable)
    dialog.AddButton(_("Clear"), clear_listener, 0);

  /* run it */

  TwoWidgets widget(new FixedWindowWidget(&latitude_entry),
                    new FixedWindowWidget(&longitude_entry),
                    true, true);
  dialog.FinishPreliminary(&widget);

  bool result = dialog.ShowModal() == mrOK;
  dialog.StealWidget();
  if (!result)
    return false;

  value = GeoPoint(longitude_entry.GetLongitude(format),
                   latitude_entry.GetLatitude(format));
  return true;
}
