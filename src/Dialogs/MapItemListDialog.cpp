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

#include "Dialogs/MapItemListDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Dialogs/Airspace/Airspace.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/Traffic/TrafficDialogs.hpp"
#include "Screen/SingleWindow.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "MapSettings.hpp"
#include "MapWindow/Items/MapItem.hpp"
#include "MapWindow/Items/List.hpp"
#include "Renderer/MapItemListRenderer.hpp"
#include "Widget/ListWidget.hpp"
#include "Form/Button.hpp"
#include "Weather/Features.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Airspace/ProtectedAirspaceWarningManager.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Dialogs/Dialogs.h"
#include "Terrain/TerrainSettings.hpp"
#include "Renderer/TwoTextRowsRenderer.hpp"

#ifdef HAVE_NOAA
#include "Dialogs/Weather/WeatherDialogs.hpp"
#endif

static bool
HasDetails(const MapItem &item)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::THERMAL:
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  case MapItem::SKYLINES_TRAFFIC:
#endif
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
  MapItemListDialog(const DialogLook &dialog_look,
                    const MapSettings &_settings,
                    const MapItemList &_footer_list)
    :WidgetDialog(dialog_look),
     footer_list(_footer_list),
     look(UIGlobals::GetLook()),
     settings(_settings)
     {}

  virtual void CreateFull(SingleWindow &parent, const TCHAR *caption,
                          Widget *widget,
                          UPixelScalar footer_height)
  {
    WidgetDialog::CreateFull(parent, caption, widget, this, footer_height);
  }
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
      + Layout::Scale(6) + dialog_look.text_font.GetHeight();
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
    canvas.DrawText(rc_caption.left + Layout::FastScale(2),
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
                                CommonInterface::GetComputerSettings().utc_offset,
                                &CommonInterface::Basic().flarm.traffic);
    }
  }
};

class MapItemListWidget final : public ListWidget, private ActionListener {
  enum Buttons {
    GOTO,
    ACK,
  };

  const MapItemList &list;

  const DialogLook &dialog_look;
  const MapSettings &settings;

  Button *details_button, *cancel_button, *goto_button;
  Button *ack_button;
  // height of rendering area for item.  list item height may be larger.
  unsigned inner_item_height;

public:
  void CreateButtons(WidgetDialog &dialog);

public:
  MapItemListWidget(const MapItemList &_list,
                    const DialogLook &_dialog_look, const MapLook &_look,
                    const TrafficLook &_traffic_look,
                    const FinalGlideBarLook &_final_glide_look,
                    const MapSettings &_settings)
    :list(_list),
     dialog_look(_dialog_look),
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
      ack_button->SetEnabled(false);
      return;
    }
    if (CanDragItem(current))
      details_button->SetCaption(_("Drag Target"));
    else
      details_button->SetCaption(_("Details"));

    details_button->SetEnabled(HasDetails(*list[current]));
    goto_button->SetEnabled(CanGotoItem(current));
    ack_button->SetEnabled(CanAckItem(current));
  }

  void OnGotoClicked();
  void OnAckClicked();

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override {
    DeleteWindow();
  }

  /* virtual methods from class List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  virtual void OnCursorMoved(unsigned index) override {
    UpdateButtons();
  }

  virtual bool CanActivateItem(unsigned index) const override {
    return HasDetails(*list[index]);
  }

  bool CanDragItem(unsigned index) const {
    assert(index < list.size());
    return protected_task_manager != NULL &&
      list[index]->type == MapItem::TASK_OZ;
  }

  bool CanGotoItem(unsigned index) const {
    return CanGotoItem(*list[index]);
  }

  static bool CanGotoItem(const MapItem &item) {
    return protected_task_manager != NULL &&
      item.type == MapItem::WAYPOINT;
  }

  bool CanAckItem(unsigned index) const {
    return CanAckItem(*list[index]);
  }

  static bool CanAckItem(const MapItem &item) {
    const AirspaceMapItem &as_item = (const AirspaceMapItem &)item;

    return item.type == MapItem::AIRSPACE &&
      GetAirspaceWarnings() != nullptr &&
      !GetAirspaceWarnings()->GetAckDay(*as_item.airspace);
  }

  virtual void OnActivateItem(unsigned index) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

void
MapItemListWidget::CreateButtons(WidgetDialog &dialog)
{
  cancel_button = dialog.AddSymbolButton(_T("_X"), mrCancel);
  goto_button = dialog.AddButton(_("Goto"), *this, GOTO);
  ack_button = dialog.AddButton(_("Ack Day"), *this, ACK);
  details_button = dialog.AddButton(_("Details"), mrOK);
}

void
MapItemListWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  TwoTextRowsRenderer renderer;
  inner_item_height = MapItemListRenderer::CalculateLayout(dialog_look, renderer);
  // maximize the list item height to use all available space if too few items.
  const unsigned list_size = std::max(1u, (unsigned)list.size());
  unsigned list_item_height = (list_size * inner_item_height < (unsigned)rc.GetSize().cy) ?
    rc.GetSize().cy / list_size : inner_item_height;

  CreateList(parent, dialog_look, rc, list_item_height);

  GetList().SetLength(list.size());
  UpdateButtons();

  for (unsigned i = 0; i < list.size(); ++i) {
    const MapItem &item = *list[i];
    if (HasDetails(item) || CanGotoItem(item)) {
      GetList().SetCursorIndex(i);
      break;
    }
  }
}

void
MapItemListWidget::OnPaintItem(Canvas &canvas, const PixelRect rc,
                               unsigned idx)
{
  const MapLook &map_look = UIGlobals::GetMapLook();
  const TrafficLook &traffic_look = UIGlobals::GetLook().traffic;
  const FinalGlideBarLook &final_glide_look = UIGlobals::GetLook().final_glide_bar;
  const MapItem &item = *list[idx];
  PixelRect rc_item = rc;
  if (rc.GetSize().cy > (int)inner_item_height) {
    rc_item.top += (rc.GetSize().cy - inner_item_height) / 2;
    rc_item.bottom -= (rc.GetSize().cy - inner_item_height) / 2;
  }

  MapItemListRenderer::Draw(canvas, rc_item, item,
                            dialog_look, map_look, traffic_look,
                            final_glide_look, settings,
                            CommonInterface::GetComputerSettings().utc_offset,
                            &CommonInterface::Basic().flarm.traffic);

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
  details_button->Click();
}

inline void
MapItemListWidget::OnGotoClicked()
{
  if (protected_task_manager == NULL)
    return;

  unsigned index = GetCursorIndex();
  auto const &item = *list[index];

  assert(item.type == MapItem::WAYPOINT);

  auto const &waypoint = ((const WaypointMapItem &)item).waypoint;
  protected_task_manager->DoGoto(waypoint);
  cancel_button->Click();
}

inline void
MapItemListWidget::OnAckClicked()
{
  const AirspaceMapItem &as_item = *(const AirspaceMapItem *)
    list[GetCursorIndex()];
  GetAirspaceWarnings()->AcknowledgeDay(*as_item.airspace);
  UpdateButtons();
}

void
MapItemListWidget::OnAction(int id)
{
  switch (id) {
  case GOTO:
    OnGotoClicked();
    break;

  case ACK:
    OnAckClicked();
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
ShowMapItemListDialog(const MapItemList &list,
                      MapItemList &footer_list,
                      MapItemList &list_list,
                      const DialogLook &dialog_look, const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings)
{
  UPixelScalar item_height = dialog_look.list.font->GetHeight()
    + Layout::Scale(6) + dialog_look.text_font.GetHeight();
  assert(item_height > 0);
  UPixelScalar caption_height = dialog_look.caption.font->GetHeight();
  UPixelScalar portrait_spacer_height = !Layout::landscape
      ? caption_height : 0u;

  MapItemListWidget widget(list_list, dialog_look, look,
                           traffic_look, final_glide_look,
                           settings);
  MapItemListDialog dialog(dialog_look, settings, footer_list);

  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Nearby items:"), &widget,
                    item_height * footer_list.size() + caption_height
                    + portrait_spacer_height);

  widget.CreateButtons(dialog);
  dialog.EnableCursorSelection();

  int result = dialog.ShowModal() == mrOK
    ? (int)widget.GetCursorIndex()
    : -1;
  dialog.StealWidget();
  return result;
}

static void
ShowMapItemDialog(const MapItem &item,
                  ProtectedAirspaceWarningManager *airspace_warnings)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::THERMAL:
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  case MapItem::SKYLINES_TRAFFIC:
#endif
    break;

  case MapItem::AIRSPACE:
    dlgAirspaceDetails(*((const AirspaceMapItem &)item).airspace,
                       airspace_warnings);
    break;
  case MapItem::WAYPOINT:
    dlgWaypointDetailsShowModal(((const WaypointMapItem &)item).waypoint);
    break;
  case MapItem::TASK_OZ:
    dlgTargetShowModal(((const TaskOZMapItem &)item).index);
    break;
  case MapItem::TRAFFIC:
    dlgFlarmTrafficDetailsShowModal(((const TrafficMapItem &)item).id);
    break;

#ifdef HAVE_NOAA
  case MapItem::WEATHER:
    dlgNOAADetailsShowModal(((const WeatherStationMapItem &)item).station);
    break;
#endif
  }
}

void
ShowMapItemListDialog(const MapItemList &list,
                      const DialogLook &dialog_look,
                      const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings,
                      ProtectedAirspaceWarningManager *airspace_warnings)
{
  /* list of items in the big list that are not in the footer */
  MapItemList list_list;
  /* list of items in the big list that are in the footer */
  MapItemList footer_list;

  //list_list.clear();
  //footer_list.clear();
  SplitMapItemList(list, footer_list, list_list);

  if (list_list.size() == 1)
    /* only one map item without details, show it.
     * non-details items are at start of list) */
    ShowMapItemDialog(*list_list[0], airspace_warnings);
  //don't show terrain-only info if terrain is disabled
  else if (list_list.size() == 0 && !settings.terrain.enable) {
    footer_list.clear();
    return;
  } else {

    /* more than one map item: show a list */
    int i = ShowMapItemListDialog(list, footer_list, list_list,
                                  dialog_look, look,
                                  traffic_look, final_glide_look, settings);
    assert(i >= -1 && i < (int)list.size());
    if (i >= 0)
      ShowMapItemDialog(*list_list[i], airspace_warnings);
  }
  list_list.clear();
  footer_list.clear();
}
