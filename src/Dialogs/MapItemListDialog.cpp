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

#include "Dialogs/MapItemListDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"
#include "Dialogs/Airspace.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/Traffic.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "MapSettings.hpp"
#include "MapWindow/MapItem.hpp"
#include "MapWindow/MapItemList.hpp"
#include "Renderer/MapItemListRenderer.hpp"
#include "Form/ListWidget.hpp"
#include "Form/Button.hpp"
#include "Weather/Features.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Interface.hpp"
#include "Dialogs/Dialogs.h"
#include "Terrain/TerrainSettings.hpp"


#ifdef HAVE_NOAA
#include "Dialogs/Weather.hpp"
#endif

static bool
HasDetails(const MapItem &item)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::MARKER:
  case MapItem::THERMAL:
    return false;

  case MapItem::AIRSPACE:
  case MapItem::WAYPOINT:
  case MapItem::TASK_OZ:
  case MapItem::TRAFFIC:
#ifdef HAVE_NOAA
  case MapItem::WEATHER:
#endif
    return true;
  }

  return false;
}

/**
 * A class based on the WidgetDialog that draws map list items
 * about the plane in the footer, and map list items that exist
 * on the map as separate entities as items in a list.
 */
class MapItemListDialog : public WidgetDialog, public WidgetDialog::DialogFooter::Listener
{
protected:
  const MapItemList &footer_list;
  const Look &look;
  const MapSettings &settings;

public:
  MapItemListDialog(const TCHAR *caption, Widget *widget,
                    UPixelScalar footer_height,
                    const MapItemList &_footer_list,
                    const MapSettings &_settings)
    :WidgetDialog(caption, widget, this, footer_height),
     footer_list(_footer_list),
     look(UIGlobals::GetLook()),
     settings(_settings)
     {}

  /**
   * Inherited from WidgetDialogActionListener
   * paints the footer of the Dialog
   */
  virtual void OnPaintFooter(Canvas &canvas)
  {
    const DialogLook &dialog_look = look.dialog;
    const TrafficLook &traffic_look = look.traffic;
    const FinalGlideBarLook &final_glide_look = look.final_glide_bar;

    /* footer_caption is painted at the top of the footer */
    PixelScalar caption_height = dialog_look.caption.font->GetHeight();

    // in portrait, draw a spacer above the caption to separate it from the buttons
    const PixelScalar portrait_spacer_height = !Layout::landscape
        ? caption_height : 0u;

    PixelScalar item_height = dialog_look.list.font->GetHeight()
      + Layout::Scale(6) + dialog_look.text_font->GetHeight();
    assert(item_height > 0);

    const PixelRect rc_footer = GetFooterRect();

    // paint the caption at the top of the footer.
    PixelRect rc_caption;
    rc_caption.left = 0;
    rc_caption.top = portrait_spacer_height;
    rc_caption.right = rc_footer.right - rc_footer.left;
    rc_caption.bottom = caption_height + portrait_spacer_height;

    if (!Layout::landscape) {
      canvas.Select(look.dialog.background_brush);
      canvas.SetBackgroundTransparent();
      canvas.Rectangle(0, 0, rc_caption.right, portrait_spacer_height);
    }


    canvas.Select(Brush(COLOR_XCSOAR_DARK));
    canvas.SetBackgroundTransparent();
    canvas.SetTextColor(COLOR_WHITE);
    canvas.Select(*dialog_look.caption.font);
    canvas.Rectangle(rc_caption.left, rc_caption.top, rc_caption.right, rc_caption.bottom);
    canvas.text(rc_caption.left + Layout::FastScale(2),
                rc_caption.top, _("Terrain:"));


    // paint the main footer information
    PixelRect footer_rect_inner = rc_footer;
    footer_rect_inner.top = caption_height + portrait_spacer_height;

    canvas.SelectWhiteBrush();
    canvas.Rectangle(footer_rect_inner.left, footer_rect_inner.top,
                     footer_rect_inner.right, footer_rect_inner.bottom);

    // paint items in footer
    canvas.SetTextColor(COLOR_BLACK);
    for (unsigned i = 0; i < footer_list.size(); i++) {
      MapItem& item = *footer_list[i];
      PixelRect rc = footer_rect_inner;
      rc.left += Layout::Scale(1);
      rc.top += i * item_height;
      rc.bottom = rc.top + item_height;
      MapItemListRenderer::Draw(canvas, rc, item,
                                dialog_look, look.map, traffic_look,
                                final_glide_look, settings,
                                &XCSoarInterface::Basic().flarm.traffic);
    }

  }
};

class MapItemListWidget : public ListWidget, private ActionListener {
  enum Buttons {
    GOTO,
  };

  const MapItemList &list;

  const DialogLook &dialog_look;
  const MapLook &look;
  const TrafficLook &traffic_look;
  const FinalGlideBarLook &final_glide_look;
  const MapSettings &settings;

  WndButton *details_button, *cancel_button, *goto_button;

public:
  void CreateButtons(WidgetDialog &dialog);

public:
  MapItemListWidget(const MapItemList &_list,
                    const DialogLook &_dialog_look, const MapLook &_look,
                    const TrafficLook &_traffic_look,
                    const FinalGlideBarLook &_final_glide_look,
                    const MapSettings &_settings)
    :list(_list),
     dialog_look(_dialog_look), look(_look),
     traffic_look(_traffic_look), final_glide_look(_final_glide_look),
     settings(_settings) {}

  unsigned GetCursorIndex() const {
    return GetList().GetCursorIndex();
  }

protected:
  void UpdateButtons() {
    const unsigned current = GetCursorIndex();
    if (list.size() == 0) {
      details_button->SetEnabled(false);
      goto_button->SetEnabled(false);
      return;
    }
    details_button->SetEnabled(HasDetails(*list[current]));
    if (CanDragItem(current))
      details_button->SetCaption(_("Drag Target"));
    else
      details_button->SetCaption(_("Details"));

    goto_button->SetEnabled(CanGotoItem(current));

  }

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare() {
    DeleteWindow();
  }

  /* virtual methods from class List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx);

  virtual void OnCursorMoved(unsigned index) {
    UpdateButtons();
  }

  virtual bool CanActivateItem(unsigned index) const {
    return HasDetails(*list[index]);
  }

  bool CanGotoItem(unsigned index) const {
    assert(index < list.size());
    return protected_task_manager != NULL &&
      list[index]->type == MapItem::WAYPOINT;
  }

  bool CanDragItem(unsigned index) const {
    assert(index < list.size());
    return protected_task_manager != NULL &&
      list[index]->type == MapItem::TASK_OZ;
  }

  virtual void OnActivateItem(unsigned index);

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id);
};

void
MapItemListWidget::CreateButtons(WidgetDialog &dialog)
{
  cancel_button = dialog.AddButton(_("Close"), mrCancel);
  details_button = dialog.AddButton(_("Details"), mrOK);
  goto_button = dialog.AddButton(_("Goto"), this, GOTO);
}

void
MapItemListWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  UPixelScalar item_height = dialog_look.list.font->GetHeight()
    + Layout::Scale(6) + dialog_look.text_font->GetHeight();
  assert(item_height > 0);

  CreateList(parent, dialog_look, rc, item_height);

  GetList().SetLength(list.size());
  UpdateButtons();
}

void
MapItemListWidget::OnPaintItem(Canvas &canvas, const PixelRect rc,
                               unsigned idx)
{
  const MapItem &item = *list[idx];
  MapItemListRenderer::Draw(canvas, rc, item,
                            dialog_look, look, traffic_look,
                            final_glide_look, settings,
                            &XCSoarInterface::Basic().flarm.traffic);

  if ((settings.item_list.add_arrival_altitude &&
       item.type == MapItem::Type::ARRIVAL_ALTITUDE) ||
      (!settings.item_list.add_arrival_altitude &&
       item.type == MapItem::Type::LOCATION)) {
    canvas.SelectBlackPen();
    canvas.DrawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);
  }
}

void
MapItemListWidget::OnActivateItem(unsigned index)
{
  details_button->OnClicked();
}

void
MapItemListWidget::OnAction(int id)
{
  switch (id) {
  case GOTO:
    if (protected_task_manager == NULL)
      break;

    unsigned index = GetCursorIndex();
    auto const &item = *list[index];

    assert(item.type == MapItem::WAYPOINT);

    auto const &waypoint = ((const WaypointMapItem &)item).waypoint;
    protected_task_manager->DoGoto(waypoint);
    cancel_button->OnClicked();

    break;
  }
}

/**
 * Populates footer_list with items the belong in the footer
 * Populates list_list with items that belong in the lists
 *
 * @param footer_list. the list of all the Map Items
 * @param footer_list. an empty list
 * @param list_list. an empty list
 */
static void
SplitMapItemList(const MapItemList &full_list,
                 MapItemList& footer_list,
                 MapItemList& list_list)
{
  for (unsigned i = 0; i < full_list.size(); i++) {
    MapItem &item = *full_list[i];
    if (!HasDetails(item))
      footer_list.append(&item);
    else
      list_list.append(&item);
  }
}

static int
ShowMapItemListDialog(SingleWindow &parent,
                      const MapItemList &list,
                      const DialogLook &dialog_look, const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings)
{
  MapItemList list_list;
  MapItemList footer_list;
  list_list.clear();
  footer_list.clear();
  SplitMapItemList(list, footer_list, list_list);

  UPixelScalar item_height = dialog_look.list.font->GetHeight()
    + Layout::Scale(6) + dialog_look.text_font->GetHeight();
  assert(item_height > 0);
  UPixelScalar caption_height = dialog_look.caption.font->GetHeight();
  UPixelScalar portrait_spacer_height = !Layout::landscape
      ? caption_height : 0u;

  MapItemListWidget widget(list_list, dialog_look, look,
                           traffic_look, final_glide_look,
                           settings);
  MapItemListDialog dialog(_("Nearby items:"), &widget,
                           item_height * footer_list.size()
                           + caption_height + portrait_spacer_height,
                           footer_list, settings);

  widget.CreateButtons(dialog);

  int result = dialog.ShowModal() == mrOK
    ? (int)widget.GetCursorIndex() + footer_list.size()
    : -1;
  dialog.StealWidget();
  list_list.clear();
  footer_list.clear();
  return result;
}

static void
ShowMapItemDialog(const MapItem &item, SingleWindow &parent,
                  ProtectedAirspaceWarningManager *airspace_warnings)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::MARKER:
  case MapItem::THERMAL:
    break;

  case MapItem::AIRSPACE:
    dlgAirspaceDetails(*((const AirspaceMapItem &)item).airspace,
                       airspace_warnings);
    break;
  case MapItem::WAYPOINT:
    dlgWaypointDetailsShowModal(parent,
                                ((const WaypointMapItem &)item).waypoint);
    break;
  case MapItem::TASK_OZ:
    dlgTargetShowModal(((const TaskOZMapItem &)item).index);
    break;
  case MapItem::TRAFFIC:
    dlgFlarmTrafficDetailsShowModal(((const TrafficMapItem &)item).id);
    break;

#ifdef HAVE_NOAA
  case MapItem::WEATHER:
    dlgNOAADetailsShowModal(parent,
                            ((const WeatherStationMapItem &)item).station);
    break;
#endif
  }
}

void
ShowMapItemListDialog(SingleWindow &parent,
                      const MapItemList &list,
                      const DialogLook &dialog_look,
                      const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings,
                      ProtectedAirspaceWarningManager *airspace_warnings)
{
  unsigned list_list_count = 0;
  for (auto i = list.begin(); i != list.end(); i++) {
    if (HasDetails(**i))
        list_list_count++;
  }
  if (list_list_count == 1)
    /* only one map item without details, show it.
     * non-details items are at start of list) */
    ShowMapItemDialog(*list[list.size() - 1], parent, airspace_warnings);
  //don't show terrain-only info if terrain is disabled
  else if (list_list_count == 0 && !settings.terrain.enable)
    return;
  else {
    /* more than one map item: show a list */
    int i = ShowMapItemListDialog(parent, list, dialog_look, look,
                                  traffic_look, final_glide_look, settings);
    assert(i >= -1 && i < (int)list.size());
    if (i >= 0)
      ShowMapItemDialog(*list[i], parent, airspace_warnings);
  }
}
