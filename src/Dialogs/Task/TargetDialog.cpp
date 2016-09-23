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

#include "TaskDialogs.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/Widget.hpp"
#include "Form/Edit.hpp"
#include "Form/CheckBox.hpp"
#include "Form/DataField/Float.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/Form.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"
#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/UserUnits.hpp"
#include "Language/Language.hpp"
#include "MapWindow/TargetMapWindow.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/AATPoint.hpp"
#include "Units/Units.hpp"
#include "Blackboard/RateLimitedBlackboardListener.hpp"
#include "Interface.hpp"
#include "Util/Clamp.hpp"

class TargetWidget;

class TargetDialogMapWindow : public TargetMapWindow {
  TargetWidget &widget;

public:
  TargetDialogMapWindow(TargetWidget &_widget,
                        const WaypointLook &waypoint_look,
                        const AirspaceLook &airspace_look,
                        const TrailLook &trail_look,
                        const TaskLook &task_look,
                        const AircraftLook &aircraft_look,
                        const TopographyLook &topography_look)
    :TargetMapWindow(waypoint_look, airspace_look, trail_look,
                     task_look, aircraft_look, topography_look),
     widget(_widget) {}

protected:
  void OnTaskModified() override;
};

class TargetWidget
  : public NullWidget, ActionListener,
    DataFieldListener,
    NullBlackboardListener {
  enum Buttons {
#ifndef GNAV
    PREVIOUS,
    NEXT,
#endif
    NAME,
    OPTIMIZED,
  };

  struct Layout {
    PixelRect map;

#ifndef GNAV
    PixelRect previous_button, next_button;
#endif
    PixelRect range, radial, ete, speed_remaining;
    PixelRect optimized;
    PixelRect close_button;

    explicit Layout(PixelRect rc);
  };

  ActionListener &dialog;
  WndForm &form;

  RateLimitedBlackboardListener rate_limited_bl;

  TargetDialogMapWindow map;

#ifndef GNAV
  Button previous_button;
  Button next_button;
#endif

  WndProperty range, radial, ete, speed_remaining;

  CheckBoxControl optimized;

  WndSymbolButton close_button;

  unsigned initial_active_task_point;
  unsigned task_size;

  RangeAndRadial range_and_radial;
  unsigned target_point;
  bool is_locked;

public:
  TargetWidget(WndForm & _form, ActionListener &_dialog,
               const DialogLook &dialog_look, const MapLook &map_look)
    :dialog(_dialog),
     form(_form),
     rate_limited_bl(*this, 1800, 300),
     map(*this,
         map_look.waypoint, map_look.airspace,
         map_look.trail, map_look.task, map_look.aircraft,
         map_look.topography),
     range(dialog_look),
     radial(dialog_look),
     ete(dialog_look),
     speed_remaining(dialog_look) {
    map.SetTerrain(terrain);
    map.SetTopograpgy(topography);
    map.SetAirspaces(&airspace_database);
    map.SetWaypoints(&way_points);
    map.SetTask(protected_task_manager);
    map.SetGlideComputer(glide_computer);
  }

  bool GetTaskData();

  /**
   * Reads task points from the protected task manager and loads the
   * Task Point UI and initializes the pan mode on the map
   */
  bool InitTargetPoints(int _target_point);

  void SetTarget();

  /**
   * Resets the target point and reads its polar coordinates from the
   * AATPoint's target.
 */
  void RefreshTargetPoint();

  /**
   * Locks target fields if turnpoint does not have adjustable target
   */
  void LockCalculatorUI();
  void LoadRange();
  void LoadRadial();

  /**
   * Loads the #range_and_radial variable into the range/radial form
   * controls.
   */
  void LoadRangeAndRadial() {
    LoadRange();
    LoadRadial();
  }

  /**
   * Refreshes UI based on location of target and current task stats
   */
  void RefreshCalculator();

  void UpdateName();

  void OnPrevClicked();
  void OnNextClicked();
  void OnNameClicked();
  void OnOptimized();

  void OnRangeModified(fixed new_value);
  void OnRadialModified(fixed new_value);

  /* virtual methods from class Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;

  void Show(const PixelRect &rc) override {
    const Layout layout(rc);

    map.MoveAndShow(layout.map);
#ifndef GNAV
    previous_button.MoveAndShow(layout.previous_button);
    next_button.MoveAndShow(layout.next_button);
#endif
    range.MoveAndShow(layout.range);
    radial.MoveAndShow(layout.radial);
    ete.MoveAndShow(layout.ete);
    speed_remaining.MoveAndShow(layout.speed_remaining);
    optimized.MoveAndShow(layout.optimized);
    close_button.MoveAndShow(layout.close_button);

    SetTarget();
    UpdateName();

    CommonInterface::GetLiveBlackboard().AddListener(rate_limited_bl);
  }

  void Hide() override {
    CommonInterface::GetLiveBlackboard().RemoveListener(rate_limited_bl);

    map.Hide();
#ifndef GNAV
    previous_button.Hide();
    next_button.Hide();
#endif
    range.Hide();
    radial.Hide();
    ete.Hide();
    speed_remaining.Hide();
    optimized.Hide();
    close_button.Hide();
  }

  void Move(const PixelRect &rc) override {
    const Layout layout(rc);

    map.Move(layout.map);
#ifndef GNAV
    previous_button.Move(layout.previous_button);
    next_button.Move(layout.next_button);
#endif
    range.Move(layout.range);
    radial.Move(layout.radial);
    ete.Move(layout.ete);
    speed_remaining.Move(layout.speed_remaining);
    optimized.Move(layout.optimized);
    close_button.Move(layout.close_button);
  }

  bool SetFocus() override {
    close_button.SetFocus();
    return true;
  }

  bool KeyPress(unsigned key_code) override;

private:
  /* virtual methods from class ActionListener */
  void OnAction(int id) override {
    switch (id) {
#ifndef GNAV
    case PREVIOUS:
      OnPrevClicked();
      break;

    case NEXT:
      OnNextClicked();
      break;
#endif

    case NAME:
      OnNameClicked();
      break;

    case OPTIMIZED:
      OnOptimized();
      break;
    }
  }

  /* virtual methods from class DataFieldListener */
  void OnModified(DataField &df) override {
    if (&df == range.GetDataField())
      OnRangeModified(((DataFieldFloat &)df).GetAsFixed());
    else if (&df == radial.GetDataField())
      OnRadialModified(((DataFieldFloat &)df).GetAsFixed());
  }

  /* virtual methods from class BlackboardListener */
  void OnCalculatedUpdate(const MoreData &basic,
                          const DerivedInfo &calculated) override {
    map.Invalidate();
    RefreshCalculator();
  }
};

class RowLayout {
  PixelRect rc;

public:
  explicit constexpr RowLayout(PixelRect _rc):rc(_rc) {}

  PixelRect NextRow(unsigned height) {
    PixelRect row = rc;
    row.bottom = rc.top += height;
    return row;
  }

  PixelRect BottomRow(unsigned height) {
    PixelRect row = rc;
    row.top = rc.bottom -= height;
    return row;
  }

  const PixelRect &GetRemaining() const {
    return rc;
  }
};

static PixelRect
SplitRow(PixelRect &left)
{
  PixelRect right = left;
  right.left = left.right = (right.left + left.right) / 2;
  return right;
}

TargetWidget::Layout::Layout(PixelRect rc)
{
  const unsigned width = rc.right - rc.left;
  const unsigned height = rc.bottom - rc.top;
  const unsigned min_control_height = ::Layout::GetMinimumControlHeight();
  const unsigned max_control_height = ::Layout::GetMaximumControlHeight();
  const unsigned close_button_height = ::Layout::GetMinimumControlHeight();

  map = rc;

  if (width > height) {
    /* landscape: form on the right */

    map.left += ::Layout::Scale(120);

    constexpr unsigned n_static = 4;
#ifndef GNAV
    constexpr unsigned n_elastic = 3;
#else
    constexpr unsigned n_elastic = 3;
#endif
    constexpr unsigned n_rows = n_static + n_elastic;

    const unsigned control_height = n_rows * min_control_height >= height
      ? (height - close_button_height) / (n_rows - 1)
      : std::min(max_control_height,
                 (height - n_static * min_control_height) / n_elastic);

    RowLayout rl(PixelRect(rc.left, rc.top, map.left, rc.bottom));

#ifndef GNAV
    previous_button = next_button = rl.NextRow(control_height);
    previous_button.right = next_button.left =
      (previous_button.right + next_button.left) / 2;
#endif

    range = rl.NextRow(control_height);
    radial = rl.NextRow(control_height);
    ete = rl.NextRow(std::min(control_height, min_control_height));
    speed_remaining = rl.NextRow(std::min(control_height, min_control_height));
    optimized = rl.NextRow(control_height);
    close_button = rl.BottomRow(close_button_height);
  } else {
    /* portrait: form on the top */

    RowLayout rl(rc);

    const unsigned control_height = min_control_height;

#ifndef GNAV
    previous_button = next_button = rl.NextRow(control_height);
    previous_button.right = next_button.left = previous_button.left +
        previous_button.GetSize().cx / 2;
#endif

    range = rl.NextRow(control_height);
    radial = SplitRow(range);

    ete = rl.NextRow(control_height);
    speed_remaining = SplitRow(ete);

    close_button = rl.BottomRow(close_button_height);
    optimized = SplitRow(close_button);

    map = rl.GetRemaining();
  }
}

template<typename... Args>
static void
UseRecommendedCaptionWidths(Args&&... args)
{
  WndProperty *controls[] = { &args... };

  unsigned width = 0;
  for (const auto *i : controls)
    width = std::max(width, i->GetRecommendedCaptionWidth());
  for (auto *i : controls)
    i->SetCaptionWidth(width);
}

void
TargetWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const Layout layout(rc);

  WindowStyle style;
  style.Hide();

  WindowStyle button_style;
  button_style.Hide();
  button_style.TabStop();

  map.Create(parent, layout.map, style);

  const auto &button_look = UIGlobals::GetDialogLook().button;

#ifndef GNAV
  previous_button.Create(parent, layout.previous_button, button_style,
                         new SymbolButtonRenderer(button_look,
                                                  _T("<")),
                         *this, PREVIOUS);
  next_button.Create(parent, layout.next_button, button_style,
                     new SymbolButtonRenderer(button_look, _T(">")),
                     *this, NEXT);
#endif

  const unsigned caption_width = ::Layout::Scale(50);

  range.Create(parent, layout.range, _("Distance"), caption_width, style);
  range.SetHelpText(_("For AAT tasks, this setting can be used to adjust the target points within the AAT sectors.  Larger values move the target points to produce larger task distances, smaller values move the target points to produce smaller task distances."));
  range.SetDataField(new DataFieldFloat(_T("%.0f"), _T("%.0f %%"),
                                        fixed(-100), fixed(100), fixed(0),
                                        fixed(5), false, this));

  radial.Create(parent, layout.radial, _("Radial"), caption_width, style);
  radial.SetHelpText(_("For AAT tasks, this setting can be used to adjust the target points within the AAT sectors.  Positive values rotate the range line clockwise, negative values rotate the range line counterclockwise."));
  radial.SetDataField(new DataFieldFloat(_T("%.0f"), _T("%.0f" DEG),
                                         fixed(-90), fixed(90), fixed(0),
                                         fixed(5), false, this));

  ete.Create(parent, layout.ete, _("ETE"), caption_width, style);
  ete.SetReadOnly();
  ete.SetHelpText(_("Estimated time en-route to the next AAT target."));

  speed_remaining.Create(parent, layout.speed_remaining, _("V rem."), caption_width, style);
  speed_remaining.SetReadOnly();
  speed_remaining.SetHelpText(_("Speed remaining"));

  UseRecommendedCaptionWidths(range, radial, ete,
                              speed_remaining);

  optimized.Create(parent, UIGlobals::GetDialogLook(), _("Optimized"),
                   layout.optimized, button_style, *this, OPTIMIZED);

  close_button.Create(parent, button_look, _T("_X"),
                      layout.close_button,
                      button_style, dialog, mrOK);
}

void
TargetWidget::LockCalculatorUI()
{
  range.SetEnabled(is_locked);
  radial.SetEnabled(is_locked);
}

void
TargetWidget::LoadRange()
{
  DataFieldFloat &df = *(DataFieldFloat *)range.GetDataField();
  assert(df.GetType() == DataField::Type::REAL);
  df.Set(range_and_radial.range * 100);
  range.RefreshDisplay();
}

void
TargetWidget::LoadRadial()
{
  DataFieldFloat &df = *(DataFieldFloat *)radial.GetDataField();
  assert(df.GetType() == DataField::Type::REAL);
  df.Set(range_and_radial.radial.Degrees());
  radial.RefreshDisplay();
}

void
TargetWidget::RefreshCalculator()
{
  bool nodisplay = false;
  bool is_aat;

  {
    ProtectedTaskManager::Lease lease(*protected_task_manager);

    const AATPoint *ap = lease->GetAATTaskPoint(target_point);

    is_aat = ((ap != nullptr) && lease->GetOrderedTask().IsOptimizable());

    if (!is_aat || target_point < initial_active_task_point) {
      nodisplay = true;
      is_locked = false;
    } else {
      range_and_radial = ap->GetTargetRangeRadial(range_and_radial.range);
      is_locked = ap->IsTargetLocked();
    }
  }

  optimized.SetVisible(is_aat);
  optimized.SetState(!is_locked);

  LockCalculatorUI();

  range.SetVisible(!nodisplay);
  radial.SetVisible(!nodisplay);

  if (!nodisplay)
    LoadRangeAndRadial();

  // update outputs
  const auto &calculated = CommonInterface::Calculated();
  const TaskStats &task_stats = calculated.ordered_task_stats;

  ete.SetVisible(!nodisplay);

  if (!nodisplay) {
    if (!task_stats.task_valid || !task_stats.current_leg.IsAchievable()) {
      ete.SetText(_T("--"));
    } else {
      assert(!negative(task_stats.current_leg.time_remaining_now));
      ete.SetText(FormatTimespanSmart((int)task_stats.current_leg.time_remaining_now, 2));
    }
  }

  const ElementStat &total = task_stats.total;
  if (total.remaining_effective.IsDefined())
    speed_remaining.SetText(FormatUserTaskSpeed(total.remaining_effective.GetSpeed()));
}

void
TargetWidget::UpdateName()
{
  StaticString<80u> buffer;

  {
    ProtectedTaskManager::Lease lease(*protected_task_manager);
    const OrderedTask &task = lease->GetOrderedTask();
    if (target_point < task.TaskSize()) {
      StaticString<16>prefix;
      if (target_point == 0)
        prefix = _T("Start");
      else if (target_point + 1 == task.TaskSize())
        prefix = _T("Finish");
      else prefix.Format(_T("%s %u"), _("Target"), target_point);

      const OrderedTaskPoint &tp = task.GetTaskPoint(target_point);
      buffer.Format(_T("%s: %s"), prefix.c_str(),
                    tp.GetWaypoint().name.c_str());
    } else
      buffer.clear();
  }

  form.SetCaption(buffer);
}

void
TargetDialogMapWindow::OnTaskModified()
{
  TargetMapWindow::OnTaskModified();
  widget.RefreshCalculator();
}

void
TargetWidget::OnOptimized()
{
  is_locked = !optimized.GetState();
  protected_task_manager->TargetLock(target_point, is_locked);
  RefreshCalculator();
}

void
TargetWidget::OnNextClicked()
{
  if (target_point < (task_size - 1))
    target_point++;
  else
    target_point = 0;

  UpdateName();
  RefreshTargetPoint();
}

void
TargetWidget::OnPrevClicked()
{
  if (target_point > 0)
    target_point--;
  else
    target_point = task_size - 1;

  UpdateName();
  RefreshTargetPoint();
}

void
TargetWidget::OnRangeModified(fixed new_value)
{
  if (target_point < initial_active_task_point)
    return;

  const fixed new_range = new_value / fixed(100);
  if (new_range == range_and_radial.range)
    return;

  if (negative(new_range) != negative(range_and_radial.range)) {
    /* when the range gets flipped, flip the radial as well */
    if (negative(range_and_radial.radial.Native()))
      range_and_radial.radial += Angle::HalfCircle();
    else
      range_and_radial.radial -= Angle::HalfCircle();
    LoadRadial();
  }

  range_and_radial.range = new_range;

  {
    ProtectedTaskManager::ExclusiveLease lease(*protected_task_manager);
    lease->SetTarget(target_point, range_and_radial);
  }

  map.Invalidate();
}

void
TargetWidget::OnRadialModified(fixed new_value)
{
  if (target_point < initial_active_task_point)
    return;

  Angle new_radial = Angle::Degrees(new_value);
  if (new_radial == range_and_radial.radial)
    return;

  bool must_reload_radial = false;
  if (new_radial >= Angle::HalfCircle()) {
    new_radial -= Angle::FullCircle();
    must_reload_radial = true;
  } else if (new_radial <= -Angle::HalfCircle()) {
    new_radial += Angle::FullCircle();
    must_reload_radial = true;
  }

  if ((new_radial.Absolute() > Angle::QuarterCircle()) !=
      (range_and_radial.radial.Absolute() > Angle::QuarterCircle())) {
    /* when the radial crosses the +/-90 degrees threshold, flip the
       range */
    range_and_radial.range = -range_and_radial.range;
    LoadRange();
  }

  range_and_radial.radial = new_radial;

  {
    ProtectedTaskManager::ExclusiveLease lease(*protected_task_manager);
    lease->SetTarget(target_point, range_and_radial);
  }

  if (must_reload_radial)
    LoadRadial();

  map.Invalidate();
}

void
TargetWidget::SetTarget()
{
  if (target_point >= task_size)
    return;

  map.SetTarget(target_point);
}

/**
 * resets the target point and reads its polar coordinates
 * from the AATPoint's target
 */
void
TargetWidget::RefreshTargetPoint()
{
  if (target_point < task_size && target_point >= initial_active_task_point)
    RefreshCalculator();
  else
    range_and_radial = RangeAndRadial::Zero();
  SetTarget();
}

inline void
TargetWidget::OnNameClicked()
{
  Waypoint waypoint;

  {
    ProtectedTaskManager::Lease lease(*protected_task_manager);
    const OrderedTask &task = lease->GetOrderedTask();
    if (target_point >= task.TaskSize())
      return;

    const OrderedTaskPoint &tp = task.GetTaskPoint(target_point);
    waypoint = tp.GetWaypoint();
  }

  dlgWaypointDetailsShowModal(waypoint);
}

bool
TargetWidget::GetTaskData()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  if (task_manager->GetMode() != TaskType::ORDERED)
    return false;

  const OrderedTask &task = task_manager->GetOrderedTask();

  initial_active_task_point = task.GetActiveIndex();
  task_size = task.TaskSize();
  return true;
}

bool
TargetWidget::InitTargetPoints(int _target_point)
{
  if (!GetTaskData())
    return false;

  if (_target_point >= 0) {
    // Explicitly requested target point (e.g. via map item list)
    target_point = _target_point;
  } else if (target_point < initial_active_task_point ||
             task_size <= target_point) {
    // Use active task point if previous selected one already achieved or invalid
    target_point = initial_active_task_point;
  }

  target_point = Clamp(int(target_point), 0, (int)task_size - 1);
  return true;
}

bool
TargetWidget::KeyPress(unsigned key_code)
{
  switch (key_code) {
  case KEY_LEFT:
    OnPrevClicked();
    return true;

  case KEY_RIGHT:
    OnNextClicked();
    return true;
  }

  return false;
}

void
dlgTargetShowModal(int _target_point)
{
  if (protected_task_manager == nullptr)
    return;

  const Look &look = UIGlobals::GetLook();
  WidgetDialog dialog(look.dialog);
  TargetWidget widget(dialog, dialog, look.dialog, look.map);
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Target"), &widget);

  if (widget.InitTargetPoints(_target_point))
    dialog.ShowModal();
  dialog.StealWidget();
}
