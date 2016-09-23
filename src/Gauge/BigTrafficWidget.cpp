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

#include "BigTrafficWidget.hpp"
#include "Dialogs/Traffic/TrafficDialogs.hpp"
#include "Math/Screen.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Form/Button.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"
#include "UIState.hpp"
#include "UIGlobals.hpp"
#include "PageActions.hpp"
#include "Look/Look.hpp"
#include "Profile/Profile.hpp"
#include "Compiler.h"
#include "FLARM/Friends.hpp"
#include "Look/FlarmTrafficLook.hpp"
#include "Gauge/FlarmTrafficWindow.hpp"
#include "Language/Language.hpp"
#include "UIUtil/GestureManager.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Settings.hpp"
#include "Renderer/UnitSymbolRenderer.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"
#include "UISettings.hpp"
#include "Asset.hpp"
#include "Renderer/SymbolRenderer.hpp"

/**
 * A Window which renders FLARM traffic, with user interaction.
 */
class FlarmTrafficControl : public FlarmTrafficWindow {
protected:
  bool enable_auto_zoom, dragging;
  unsigned zoom;
  Angle task_direction;
  GestureManager gestures;

public:
  FlarmTrafficControl(const FlarmTrafficLook &look)
    :FlarmTrafficWindow(look, Layout::Scale(10),
                        Layout::GetMinimumControlHeight() + Layout::Scale(2)),
     enable_auto_zoom(true), dragging(false),
     zoom(2),
     task_direction(Angle::Degrees(-1)) {}

protected:
  void CalcAutoZoom();

public:
  void Update(Angle new_direction, const TrafficList &new_data,
              const TeamCodeSettings &new_settings);
  void UpdateTaskDirection(bool show_task_direction, Angle bearing);

  bool GetNorthUp() const {
    return enable_north_up;
  }

  void SetNorthUp(bool enabled);

  void ToggleNorthUp() {
    SetNorthUp(!GetNorthUp());
  }

  bool GetAutoZoom() const {
    return enable_auto_zoom;
  }

  static unsigned GetZoomDistance(unsigned zoom);

  void SetZoom(unsigned _zoom) {
    zoom = _zoom;
    SetDistance(fixed(GetZoomDistance(_zoom)));
  }

  void SetAutoZoom(bool enabled);

  void ToggleAutoZoom() {
    SetAutoZoom(!GetAutoZoom());
  }

  bool CanZoomOut() const {
    return zoom < 4;
  }

  bool CanZoomIn() const {
    return zoom > 0;
  }

  void ZoomOut();
  void ZoomIn();

  void SwitchData();
  void OpenDetails();

protected:
  void PaintTrafficInfo(Canvas &canvas) const;

  /**
   * paints a metric on the screen at the upper right of the rect in column 0-2
   *
   */
  void PaintMetric(Canvas &canvas, PixelRect rc, fixed value, const TCHAR *value_text,
                   const Unit &unit, const TCHAR *label, unsigned column,
                   bool draw_arrow, SymbolRenderer::Direction arrow_direction =
                       SymbolRenderer::Direction::UP) const;
  /**
   * Vario in upper right corner
   */
  void PaintClimbRate(Canvas &canvas, PixelRect rc, fixed climb_rate) const;

  /**
   * Distance in lower left
   */
  void PaintDistance(Canvas &canvas, PixelRect rc, fixed distance) const;

  /**
   * Relative altitude in lower right
   */
  void PaintRelativeAltitude(Canvas &canvas, PixelRect rc,
                             fixed relative_altitude) const;

  /**
   * Id in upper left
   */
  void PaintID(Canvas &canvas, PixelRect rc, const FlarmTraffic &traffic) const;

  /**
   *
   */
  void PaintTaskDirection(Canvas &canvas) const;

  void StopDragging() {
    if (!dragging)
      return;

    dragging = false;
    ReleaseCapture();
  }

protected:
  bool OnMouseGesture(const TCHAR* gesture);

  /* virtual methods from class Window */
  virtual void OnCreate() override;
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys) override;
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y) override;
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y) override;
  virtual bool OnKeyDown(unsigned key_code) override;
  virtual void OnCancelMode() override;

  /* virtual methods from class PaintWindow */
  virtual void OnPaint(Canvas &canvas) override;
};

void
FlarmTrafficControl::OnCreate()
{
  FlarmTrafficWindow::OnCreate();

  const TrafficSettings &settings = CommonInterface::GetUISettings().traffic;

  Profile::GetEnum(ProfileKeys::FlarmSideData, side_display_type);
  enable_auto_zoom = settings.auto_zoom;
  enable_north_up = settings.north_up;
}

unsigned
FlarmTrafficControl::GetZoomDistance(unsigned zoom)
{
  UnitSetting &config = CommonInterface::SetUISettings().format.units;

  if (config.distance_unit == Unit::NAUTICAL_MILES ||
      config.distance_unit == Unit::STATUTE_MILES)
    switch (zoom) {
    case 0:
      return (unsigned)Units::ToSysUnit(fixed(0.25), config.distance_unit);
    case 1:
      return (unsigned)Units::ToSysUnit(fixed(0.5), config.distance_unit);
    case 3:
      return (unsigned)Units::ToSysUnit(fixed(3), config.distance_unit);
    case 4:
      return (unsigned)Units::ToSysUnit(fixed(6), config.distance_unit);
    case 2:
    default:
      return (unsigned)Units::ToSysUnit(fixed(2), config.distance_unit);
    }
  else
    switch (zoom) {
    case 0:
      return 500;
    case 1:
      return 1000;
    case 3:
      return 5000;
    case 4:
      return 10000;
    case 2:
    default:
      return 2000;
    }
}

void
FlarmTrafficControl::SetNorthUp(bool enabled)
{
  TrafficSettings &settings = CommonInterface::SetUISettings().traffic;
  settings.north_up = enable_north_up = enabled;
  Profile::Set(ProfileKeys::FlarmNorthUp, enabled);
  //north_up->SetState(enabled);
}

void
FlarmTrafficControl::SetAutoZoom(bool enabled)
{
  TrafficSettings &settings = CommonInterface::SetUISettings().traffic;
  settings.auto_zoom = enable_auto_zoom = enabled;
  Profile::Set(ProfileKeys::FlarmAutoZoom, enabled);
  //auto_zoom->SetState(enabled);
}

void
FlarmTrafficControl::CalcAutoZoom()
{
  bool warning_mode = WarningMode();
  RoughDistance zoom_dist = fixed(0);

  for (auto it = data.list.begin(), end = data.list.end();
      it != end; ++it) {
    if (warning_mode && !it->HasAlarm())
      continue;

    zoom_dist = std::max(it->distance, zoom_dist);
  }

  fixed zoom_dist2 = zoom_dist;
  for (unsigned i = 0; i <= 4; i++) {
    if (i == 4 || fixed(GetZoomDistance(i)) >= zoom_dist2) {
      SetZoom(i);
      break;
    }
  }
}

void
FlarmTrafficControl::Update(Angle new_direction, const TrafficList &new_data,
                            const TeamCodeSettings &new_settings)
{
  FlarmTrafficWindow::Update(new_direction, new_data, new_settings);

  if (enable_auto_zoom || WarningMode())
    CalcAutoZoom();
}

void
FlarmTrafficControl::UpdateTaskDirection(bool show_task_direction, Angle bearing)
{
  if (!show_task_direction)
    task_direction = Angle::Degrees(-1);
  else
    task_direction = bearing.AsBearing();
}

/**
 * Zoom out one step
 */
void
FlarmTrafficControl::ZoomOut()
{
  if (WarningMode())
    return;

  if (zoom < 4)
    SetZoom(zoom + 1);

  SetAutoZoom(false);
}

/**
 * Zoom in one step
 */
void
FlarmTrafficControl::ZoomIn()
{
  if (WarningMode())
    return;

  if (zoom > 0)
    SetZoom(zoom - 1);

  SetAutoZoom(false);
}

/**
 * Paints an arrow into the direction of the current task leg
 * @param canvas The canvas to paint on
 */
void
FlarmTrafficControl::PaintTaskDirection(Canvas &canvas) const
{
  if (negative(task_direction.Degrees()))
    return;

  canvas.Select(look.radar_pen);
  canvas.SelectHollowBrush();

  RasterPoint triangle[4];
  triangle[0].x = 0;
  triangle[0].y = -radius / Layout::FastScale(1) + 15;
  triangle[1].x = 7;
  triangle[1].y = triangle[0].y + 30;
  triangle[2].x = -triangle[1].x;
  triangle[2].y = triangle[1].y;
  triangle[3].x = triangle[0].x;
  triangle[3].y = triangle[0].y;

  PolygonRotateShift(triangle, 4, radar_mid,
                     task_direction - (enable_north_up ?
                                       Angle::Zero() : heading));

  // Draw the arrow
  canvas.DrawPolygon(triangle, 4);
}

void
FlarmTrafficControl::PaintMetric(Canvas &canvas, PixelRect rc, fixed value,
                                 const TCHAR *value_text,
                                 const Unit &unit, const TCHAR *label,
                                 unsigned column, bool draw_arrow,
                                 SymbolRenderer::Direction arrow_direction) const
{
  // Calculate unit size
  canvas.Select(look.info_units_font);
  const unsigned unit_width = UnitSymbolRenderer::GetSize(canvas, unit).cx;
  const unsigned unit_height =
      UnitSymbolRenderer::GetAscentHeight(look.info_units_font, unit);
  unsigned column_width = rc.GetSize().cx / 4;

  UPixelScalar space_width = unit_width / 3;

  /// never draw arrow.  Confusing.
  draw_arrow = false;

  // Calculate positions
  const int column_left = (column + 1) * column_width;
  const unsigned arrow_width = draw_arrow ? Layout::Scale(6): 0;


  canvas.Select(look.info_labels_font);
  const unsigned label_height = look.info_labels_font.GetAscentHeight();
  const unsigned label_width = canvas.CalcTextSize(label).cx;
  const unsigned value_top = label_height;

  // Paint label
  unsigned label_left = label_width > column_width ? column_left :
      column_left + (column_width - label_width) / 2;
  canvas.DrawText(label_left, 0, label);

  // Calculate value size
  canvas.Select(look.info_values_font);
  const unsigned value_height = look.info_values_font.GetAscentHeight();
  const unsigned value_width = canvas.CalcTextSize(value_text).cx;

  // Paint value
  const unsigned total_value_width = unit_width + space_width + value_width +
      -                  arrow_width;
  const unsigned value_left = total_value_width > column_width ? column_left :
      column_left + (column_width - total_value_width) / 2;
  canvas.DrawText(value_left, value_top, value_text);

  // Paint unit
  canvas.Select(look.info_units_font);
  UnitSymbolRenderer::Draw(canvas,
                           RasterPoint(value_left + value_width + space_width ,
                                       value_top + value_height - unit_height),
                           unit, look.unit_fraction_pen);
  if (draw_arrow) {
    PixelRect rc_arrow = rc;
    rc_arrow.left = value_left + value_width + space_width + unit_width;
    rc_arrow.right = rc_arrow.left + arrow_width;
    rc_arrow.bottom = value_top + value_height;
    rc_arrow.top = value_top;
    if (arrow_direction == SymbolRenderer::Direction::UP) {
      rc_arrow.bottom -= arrow_width;
      rc_arrow.top -= arrow_width;
    }
    SymbolRenderer::DrawArrow(canvas, rc_arrow, arrow_direction, true);
  }


}

void
FlarmTrafficControl::PaintClimbRate(Canvas &canvas, PixelRect rc,
                                    fixed climb_rate) const
{
  // Format climb rate
  TCHAR buffer[20];
  Unit unit = Units::GetUserVerticalSpeedUnit();
  FormatUserVerticalSpeed(climb_rate, buffer, false);

  PaintMetric(canvas, rc, climb_rate, buffer, unit, _("Vario"), 2, false);
}

static unsigned GetButtonHeight()
{
  return Layout::GetMinimumControlHeight();
}

void
FlarmTrafficControl::PaintDistance(Canvas &canvas, PixelRect rc,
                                   fixed distance) const
{
  // Format distance
  TCHAR buffer[20];
  Unit unit = Units::GetUserDistanceUnit();
  FormatUserDistance(distance, buffer, false, 1);

  PaintMetric(canvas, rc, distance, buffer, unit, _("Dist."), 0, false);

}

void
FlarmTrafficControl::PaintRelativeAltitude(Canvas &canvas, PixelRect rc,
                                           fixed relative_altitude) const
{
  // Format relative altitude
  TCHAR buffer[20];
  Unit unit = Units::GetUserAltitudeUnit();
  FormatRelativeUserAltitude(relative_altitude, buffer, false);

  PaintMetric(canvas, rc, relative_altitude, buffer, unit,
              positive(relative_altitude) ? _("Above") : _("Below"), 1, true,
              positive(relative_altitude) ? SymbolRenderer::Direction::UP :
              SymbolRenderer::Direction::DOWN);
}

void
FlarmTrafficControl::PaintID(Canvas &canvas, PixelRect rc,
                             const FlarmTraffic &traffic) const
{
  TCHAR buffer[20];

  unsigned font_size;
  if (traffic.HasName()) {
    canvas.Select(look.call_sign_font);
    font_size = look.call_sign_font.GetHeight();

    _tcscpy(buffer, traffic.name);
  } else {
    canvas.Select(look.info_labels_font);
    font_size = look.info_labels_font.GetHeight();

    traffic.id.Format(buffer);
  }

  if (!WarningMode()) {
    // Team color dot
    FlarmColor team_color = FlarmFriends::GetFriendColor(traffic.id);

    // If team color found -> draw a colored circle in front of the name
    if (team_color != FlarmColor::NONE) {
      switch (team_color) {
      case FlarmColor::GREEN:
        canvas.Select(look.team_brush_green);
        break;
      case FlarmColor::BLUE:
        canvas.Select(look.team_brush_blue);
        break;
      case FlarmColor::YELLOW:
        canvas.Select(look.team_brush_yellow);
        break;
      case FlarmColor::MAGENTA:
        canvas.Select(look.team_brush_magenta);
        break;
      default:
        break;
      }

      canvas.SelectNullPen();
      canvas.DrawCircle(rc.left + Layout::FastScale(7), rc.top + (font_size / 2),
                    Layout::FastScale(7));

      rc.left += Layout::FastScale(16);
    }
  }

  canvas.DrawText(rc.left, rc.top, buffer);
}

/**
 * Paints the basic info for the selected target on the given canvas
 * @param canvas The canvas to paint on
 */
void
FlarmTrafficControl::PaintTrafficInfo(Canvas &canvas) const
{
  // Don't paint numbers if no plane selected
  if (selection == -1 && !WarningMode())
    return;

  // Shortcut to the selected traffic
  FlarmTraffic traffic = data.list[WarningMode() ? warning : selection];
  assert(traffic.IsDefined());

  const unsigned padding = Layout::GetTextPadding();
  PixelRect rc;
  rc.left = padding;
  rc.top = padding;
  rc.right = canvas.GetWidth() - padding;
  rc.bottom = canvas.GetHeight() - padding;

  // Set the text color and background
  switch (traffic.alarm_level) {
  case FlarmTraffic::AlarmType::LOW:
  case FlarmTraffic::AlarmType::INFO_ALERT:
    canvas.SetTextColor(look.warning_color);
    break;
  case FlarmTraffic::AlarmType::IMPORTANT:
  case FlarmTraffic::AlarmType::URGENT:
    canvas.SetTextColor(look.alarm_color);
    break;
  case FlarmTraffic::AlarmType::NONE:
    canvas.SetTextColor(look.default_color);
    break;
  }

  canvas.SetBackgroundTransparent();

  // Climb Rate
  if (!WarningMode() && traffic.climb_rate_avg30s_available)
    PaintClimbRate(canvas, rc, traffic.climb_rate_avg30s);

  // Distance
  PaintDistance(canvas, rc, traffic.distance);

  // Relative Height
  PaintRelativeAltitude(canvas, rc, traffic.relative_altitude);

  // ID / Name
  if (!traffic.HasAlarm())
    canvas.SetTextColor(look.selection_color);

  PaintID(canvas, rc, traffic);
}

void
FlarmTrafficControl::OnPaint(Canvas &canvas)
{
  canvas.ClearWhite();

  PaintTaskDirection(canvas);
  FlarmTrafficWindow::Paint(canvas);
  PaintTrafficInfo(canvas);
}

void
FlarmTrafficControl::OpenDetails()
{
  // If warning is displayed -> prevent from opening details dialog
  if (WarningMode())
    return;

  // Don't open the details dialog if no plane selected
  const FlarmTraffic *traffic = GetTarget();
  if (traffic == NULL)
    return;

  // Show the details dialog
  dlgFlarmTrafficDetailsShowModal(traffic->id);
}

void
TrafficWidget::OpenDetails()
{
  view->OpenDetails();
}

void
TrafficWidget::ZoomIn()
{
  view->ZoomIn();
  UpdateButtons();
}

void
TrafficWidget::ZoomOut()
{
  view->ZoomOut();
  UpdateButtons();
}

void
TrafficWidget::PreviousTarget()
{
  view->PrevTarget();
}

void
TrafficWidget::NextTarget()
{
  view->NextTarget();
}

void
FlarmTrafficControl::SwitchData()
{
  if (side_display_type == FlarmTrafficWindow::SIDE_INFO_VARIO)
    side_display_type = FlarmTrafficWindow::SIDE_INFO_RELATIVE_ALTITUDE;
  else
    side_display_type = FlarmTrafficWindow::SIDE_INFO_VARIO;

  Profile::SetEnum(ProfileKeys::FlarmSideData, side_display_type);
}

void
TrafficWidget::SwitchData()
{
  view->SwitchData();
}

bool
TrafficWidget::GetAutoZoom() const
{
  return view->GetAutoZoom();
}

void
TrafficWidget::SetAutoZoom(bool value)
{
  view->SetAutoZoom(value);
}

void
TrafficWidget::ToggleAutoZoom()
{
  view->ToggleAutoZoom();
}

bool
TrafficWidget::GetNorthUp() const
{
  return view->GetNorthUp();
}

void
TrafficWidget::SetNorthUp(bool value)
{
  view->SetAutoZoom(value);
}

void
TrafficWidget::ToggleNorthUp()
{
  view->ToggleNorthUp();
}

void
TrafficWidget::Update()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();

  if (CommonInterface::GetUISettings().traffic.auto_close_dialog &&
      basic.flarm.traffic.IsEmpty() &&
      /* auto-close only really closes the FLARM radar if the
         "restored" page has no FLARM radar */
      PageActions::GetConfiguredLayout().main != PageLayout::Main::FLARM_RADAR) {
    /* this must be deferred, because this method is called from
       within the BlackboardListener, and we must not unregister the
       listener in this context */
    PageActions::DeferredRestore();
    return;
  }

  view->Update(basic.track,
               basic.flarm.traffic,
               CommonInterface::GetComputerSettings().team_code);

  view->UpdateTaskDirection(calculated.task_stats.task_valid &&
                            calculated.task_stats.current_leg.solution_remaining.IsOk(),
                            calculated.task_stats.
                            current_leg.solution_remaining.cruise_track_bearing);

  UpdateButtons();
}

bool
FlarmTrafficControl::OnMouseMove(PixelScalar x, PixelScalar y,
                                 gcc_unused unsigned keys)
{
  if (dragging)
    gestures.Update(x, y);

  return true;
}

bool
FlarmTrafficControl::OnMouseDown(PixelScalar x, PixelScalar y)
{
  if (!dragging) {
    dragging = true;
    SetCapture();
    gestures.Start(x, y, Layout::Scale(20));
  }

  return true;
}

bool
FlarmTrafficControl::OnMouseUp(PixelScalar x, PixelScalar y)
{
  InputEvents::HideMenu();
  if (dragging) {
    StopDragging();

    const TCHAR *gesture = gestures.Finish();
    if (gesture && OnMouseGesture(gesture))
      return true;
  }

  if (!WarningMode())
    SelectNearTarget(x, y, Layout::Scale(15));

  return true;
}

bool
FlarmTrafficControl::OnMouseGesture(const TCHAR* gesture)
{
  if (StringIsEqual(gesture, _T("U"))) {
    ZoomIn();
    return true;
  }
  if (StringIsEqual(gesture, _T("D"))) {
    ZoomOut();
    return true;
  }
  if (StringIsEqual(gesture, _T("UD"))) {
    SetAutoZoom(true);
    return true;
  }
  if (StringIsEqual(gesture, _T("DR"))) {
    OpenDetails();
    return true;
  }
  if (StringIsEqual(gesture, _T("RL"))) {
    SwitchData();
    return true;
  }

  return InputEvents::processGesture(gesture);
}

void
FlarmTrafficControl::OnCancelMode()
{
  FlarmTrafficWindow::OnCancelMode();
  StopDragging();
}

bool
FlarmTrafficControl::OnKeyDown(unsigned key_code)
{
  switch (key_code) {
  case KEY_UP:
    if (!HasPointer())
      break;

    ZoomIn();
    return true;

  case KEY_DOWN:
    if (!HasPointer())
      break;

    ZoomOut();
    return true;

#ifdef GNAV
  case '6':
    PrevTarget();
    return true;

  case '7':
    NextTarget();
    return true;
#endif
  }

  return FlarmTrafficWindow::OnKeyDown(key_code) ||
    InputEvents::processKey(key_code);
}

void
TrafficWidget::UpdateLayout()
{
  const PixelRect rc = GetContainer().GetClientRect();
  view->Move(rc);

#ifndef GNAV
  const unsigned margin = Layout::Scale(1);
  const unsigned button_height = GetButtonHeight();
  const unsigned button_width =  std::max(unsigned(rc.right / 6),
                                          GetButtonHeight());

  PixelRect button_rc;
  button_rc.left = 0;
  button_rc.right = button_width * (Layout::landscape ? 2 : 1);
  button_rc.bottom = rc.bottom;
  button_rc.top = button_rc.bottom - button_height;
  close_button->Move(button_rc);

  if (Layout::landscape)
    button_rc.bottom = rc.bottom -
        (close_button->IsVisible() ? button_height : 0);
  else
    button_rc.bottom = rc.bottom -
        (close_button->IsVisible() ? 2 : 1) * button_height;
  button_rc.top = button_rc.bottom - button_height;
  button_rc.right = button_width;
  zoom_in_button->Move(button_rc);

  if (Layout::landscape)
    button_rc.left = button_width;
  else
    button_rc.left = 0;
  button_rc.right = button_rc.left + button_width;
  button_rc.bottom = rc.bottom -
      (close_button->IsVisible() ? button_height : 0);
  button_rc.top = button_rc.bottom - button_height;
  zoom_out_button->Move(button_rc);

  button_rc.left = margin;
  button_rc.top = button_height;
  button_rc.right = button_rc.left + Layout::Scale(50);
  button_rc.bottom = button_rc.top + button_height;
  details_button->Move(button_rc);

  button_rc.top = button_rc.bottom;
  button_rc.bottom = button_rc.top + button_height;
  next_item_button->Move(button_rc);

#endif
}

void
TrafficWidget::UpdateButtons()
{
#ifndef GNAV
  const bool unlocked = !view->WarningMode();
  const TrafficList &traffic = CommonInterface::Basic().flarm.traffic;
  const bool not_empty = !traffic.IsEmpty();
  const bool two_or_more = traffic.GetActiveTrafficCount() >= 2;

  zoom_in_button->SetEnabled(unlocked && view->CanZoomIn());
  zoom_out_button->SetEnabled(unlocked && view->CanZoomOut());
  next_item_button->SetEnabled(unlocked && two_or_more);
  details_button->SetEnabled(unlocked && not_empty);
#endif
}

#ifndef GNAV

static Button *
NewSymbolButton(ContainerWindow &parent, const ButtonLook &look,
                const TCHAR *caption,
                const PixelRect &rc,
                ActionListener &listener, int id)
{
  return new Button(parent, rc, WindowStyle(),
                    new SymbolButtonRenderer(look, caption),
                    listener, id);
}

#endif

void
TrafficWidget::Prepare(ContainerWindow &parent, const PixelRect &_rc)
{
  ContainerWidget::Prepare(parent, _rc);

  const Look &look = UIGlobals::GetLook();

  const PixelRect rc = GetContainer().GetClientRect();

#ifndef GNAV
  zoom_in_button = NewSymbolButton(GetContainer(), look.dialog.button,
                                   _T("+"), rc, *this, ZOOM_IN);
  zoom_out_button = NewSymbolButton(GetContainer(), look.dialog.button,
                                    _T("-"), rc, *this, ZOOM_OUT);
  next_item_button = NewSymbolButton(GetContainer(), look.dialog.button,
                                     _T(">"), rc, *this, NEXT_ITEM);
  details_button = new Button(GetContainer(), look.dialog.button,
                              _("Details"), rc, WindowStyle(),
                              *this, DETAILS);
  close_button = NewSymbolButton(GetContainer(), look.dialog.button,
                                 _T("_X"), rc, *this, CLOSE);
#endif

  WindowStyle style;

  view = new FlarmTrafficControl(look.flarm_dialog);
  view->Create(GetContainer(), rc, style);
  CommonInterface::BroadcastUIStateUpdate();

  UpdateLayout();
}

void
TrafficWidget::Unprepare()
{
#ifndef GNAV
  delete zoom_in_button;
  delete zoom_out_button;
  delete next_item_button;
  delete details_button;
  delete close_button;
#endif

  delete view;

  ContainerWidget::Unprepare();
}

void
TrafficWidget::Show(const PixelRect &rc)
{
  // Update Radar and Selection for the first time
  Update();

  ContainerWidget::Show(rc);
#ifndef GNAV
  /* show the "Close" button only if this is a "special" page */

#if defined(ENABLE_OPENGL) | defined(KOBO)
  /* hide close button on OPENGL where we have the screens button */
  close_button->SetVisible(CommonInterface::GetUIState().pages.special_page.IsDefined() &&
                           CommonInterface::GetUISettings().screens_button_location ==
                                     UISettings::ScreensButtonLocation::MENU);
#else
  close_button->SetVisible(CommonInterface::GetUIState().pages.special_page.IsDefined());
#endif

#endif
  UpdateLayout();

  CommonInterface::GetLiveBlackboard().AddListener(*this);
}

void
TrafficWidget::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
  ContainerWidget::Hide();
}

void
TrafficWidget::Move(const PixelRect &rc)
{
  ContainerWidget::Move(rc);

  UpdateLayout();
}


bool
TrafficWidget::SetFocus()
{
  view->SetFocus();
  return true;
}

#ifndef GNAV

void
TrafficWidget::OnAction(int id)
{
  switch ((Action)id) {
  case CLOSE:
    PageActions::Restore();
    break;

  case DETAILS:
    OpenDetails();
    break;

  case PREVIOUS_ITEM:
    PreviousTarget();
    break;

  case NEXT_ITEM:
    NextTarget();
    break;

  case ZOOM_IN:
    ZoomIn();
    break;

  case ZOOM_OUT:
    ZoomOut();
    break;
  }
}

#endif

void
TrafficWidget::OnGPSUpdate(const MoreData &basic)
{
  Update();
}
