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

#include "InfoBoxes/Content/Task.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/dlgAnalysis.hpp"
#include "Engine/Util/Gradient.hpp"
#include "Units/Units.hpp"
#include "Formatter/Units.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "InfoBoxes/Panel/OnlineContest.hpp"
#include "InfoBoxes/Panel/Home.hpp"
#include "Util/Macros.hpp"
#include "ComputerSettings.hpp"
#include "GlideSolvers/MacCready.hpp"
#include "GlideSolvers/GlideState.hpp"

#include <tchar.h>
#include <stdio.h>

void
InfoBoxContentBearing::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  const GeoVector &vector_remaining = task_stats.current_leg.vector_remaining;
  if (!task_stats.task_valid || !vector_remaining.IsValid() ||
      vector_remaining.distance <= fixed(10)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(vector_remaining.bearing);
}

void
InfoBoxContentBearingDiff::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  const GeoVector &vector_remaining = task_stats.current_leg.vector_remaining;
  if (!basic.track_available || !task_stats.task_valid ||
      !vector_remaining.IsValid() || vector_remaining.distance <= fixed(10)) {
    data.SetInvalid();
    return;
  }

  Angle Value = vector_remaining.bearing - basic.track;
  data.SetValueFromBearingDifference(Value);
}

void
InfoBoxContentNextWaypoint::Update(InfoBoxData &data)
{
  // use proper non-terminal next task stats

  const Waypoint* way_point = protected_task_manager != NULL
    ? protected_task_manager->GetActiveWaypoint()
    : NULL;

  if (!way_point) {
    data.SetTitle(_("Next"));
    data.SetInvalid();
    return;
  }

  data.SetTitle(way_point->name.c_str());

  // Set Comment
  if (way_point->radio_frequency.IsDefined()) {
    const unsigned freq = way_point->radio_frequency.GetKiloHertz();
    data.FormatComment(_T("%u.%03u %s"),
                       freq / 1000, freq % 1000, way_point->comment.c_str());
  }
  else
    data.SetComment(way_point->comment.c_str());

  const NMEAInfo &basic = CommonInterface::Basic();
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  const GlideResult &solution_remaining =
    task_stats.current_leg.solution_remaining;
  const GeoVector &vector_remaining = task_stats.current_leg.vector_remaining;
  if (!basic.track_available || !task_stats.task_valid ||
      !vector_remaining.IsValid()) {
    data.SetValueInvalid();
    return;
  }

  // Set Value
  Angle Value = vector_remaining.bearing - basic.track;
  data.SetValueFromBearingDifference(Value);

  // Set Color (blue/black)
  data.SetValueColor(solution_remaining.IsFinalGlide() ? 2 : 0);
}

bool
InfoBoxContentNextWaypoint::HandleKey(const InfoBoxKeyCodes keycode)
{
  if (protected_task_manager == NULL)
    return false;

  switch (keycode) {
  case ibkRight:
  case ibkUp:
    protected_task_manager->IncrementActiveTaskPoint(1);
    return true;

  case ibkLeft:
  case ibkDown:
    protected_task_manager->IncrementActiveTaskPoint(-1);
    return true;

  case ibkEnter:
    const Waypoint *wp = protected_task_manager->GetActiveWaypoint();
    if (wp) {
      dlgWaypointDetailsShowModal(UIGlobals::GetMainWindow(), *wp);
      return true;
    }
  }

  return false;
}

void
InfoBoxContentNextDistance::Update(InfoBoxData &data)
{
  const Waypoint* way_point = protected_task_manager != NULL
    ? protected_task_manager->GetActiveWaypoint()
    : NULL;

  // Set title
  if (!way_point)
    data.SetTitle(_("WP Dist"));
  else
    data.SetTitle(way_point->name.c_str());

  // use proper non-terminal next task stats

  const NMEAInfo &basic = CommonInterface::Basic();
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const GeoVector &vector_remaining = task_stats.current_leg.vector_remaining;
  if (!task_stats.task_valid || !vector_remaining.IsValid()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(vector_remaining.distance);

  if (basic.track_available) {
    Angle bd = vector_remaining.bearing - basic.track;
    data.SetCommentFromBearingDifference(bd);
  } else
    data.SetCommentInvalid();
}

void
InfoBoxContentNextETE::Update(InfoBoxData &data)
{
  // use proper non-terminal next task stats

  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.current_leg.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  assert(!negative(task_stats.current_leg.time_remaining));

  TCHAR value[32];
  TCHAR comment[32];
  const int dd = (int)task_stats.current_leg.time_remaining;
  FormatTimeTwoLines(value, comment, dd);

  data.SetValue(value);
  data.SetComment(comment);
}

void
InfoBoxContentNextETA::Update(InfoBoxData &data)
{
  // use proper non-terminal next task stats

  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.current_leg.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  const BrokenTime &now_local = CommonInterface::Calculated().date_time_local;
  const BrokenTime t = now_local +
    unsigned(task_stats.current_leg.solution_remaining.time_elapsed);

  // Set Value
  data.UnsafeFormatValue(_T("%02u:%02u"), t.hour, t.minute);

  // Set Comment
  data.UnsafeFormatComment(_T("%02u"), t.second);
}

static void
SetValueFromAltDiff(InfoBoxData &data, const TaskStats &task_stats,
                    const GlideResult &solution)
{
  if (!task_stats.task_valid || !solution.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  const ComputerSettings &settings = CommonInterface::GetComputerSettings();
  fixed altitude_difference =
    solution.SelectAltitudeDifference(settings.task.glide);
  data.SetValueFromArrival(altitude_difference);
}

void
InfoBoxContentNextAltitudeDiff::Update(InfoBoxData &data)
{
  // pilots want this to be assuming terminal flight to this wp

  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const GlideResult &next_solution = task_stats.current_leg.solution_remaining;

  SetValueFromAltDiff(data, task_stats, next_solution);
}

void
InfoBoxContentNextMC0AltitudeDiff::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  SetValueFromAltDiff(data, task_stats,
                      task_stats.current_leg.solution_mc0);
}

void
InfoBoxContentNextAltitudeRequire::Update(InfoBoxData &data)
{
  // pilots want this to be assuming terminal flight to this wp

  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const GlideResult &next_solution = task_stats.current_leg.solution_remaining;
  if (!task_stats.task_valid || !next_solution.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromAltitude(next_solution.GetRequiredAltitude());
}

void
InfoBoxContentNextAltitudeArrival::Update(InfoBoxData &data)
{
  // pilots want this to be assuming terminal flight to this wp

  const MoreData &basic = CommonInterface::Basic();
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const GlideResult next_solution = task_stats.current_leg.solution_remaining;
  if (!basic.NavAltitudeAvailable() ||
      !task_stats.task_valid || !next_solution.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromAltitude(next_solution.GetArrivalAltitude(basic.nav_altitude));
}


void
InfoBoxContentNextGR::Update(InfoBoxData &data)
{
  // pilots want this to be assuming terminal flight to this wp, and this
  // is what current_leg gradient does.

  if (!XCSoarInterface::Calculated().task_stats.task_valid) {
    data.SetInvalid();
    return;
  }

  fixed gradient = XCSoarInterface::Calculated().task_stats.current_leg.gradient;

  if (!positive(gradient)) {
    data.SetValue(_T("+++"));
    return;
  }
  if (::GradientValid(gradient)) {
    data.SetValueFromGlideRatio(gradient);
  } else {
    data.SetInvalid();
  }
}

void
InfoBoxContentFinalDistance::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  if (!task_stats.task_valid ||
      !task_stats.current_leg.vector_remaining.IsValid() ||
      !task_stats.total.remaining.IsDefined()) {
    data.SetInvalid();
    return;
  }

  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  // Set Value
  data.SetValueFromDistance(common_stats.task_finished
                            ? task_stats.current_leg.vector_remaining.distance
                            : task_stats.total.remaining.GetDistance());
}

void
InfoBoxContentFinalETE::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  if (!task_stats.task_valid || !task_stats.total.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  assert(!negative(task_stats.total.time_remaining));

  TCHAR value[32];
  TCHAR comment[32];
  const int dd = abs((int)task_stats.total.time_remaining);
  FormatTimeTwoLines(value, comment, dd);

  data.SetValue(value);
  data.SetComment(comment);
}

void
InfoBoxContentFinalETA::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.total.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  const BrokenTime &now_local = CommonInterface::Calculated().date_time_local;
  const BrokenTime t = now_local +
    unsigned(task_stats.total.solution_remaining.time_elapsed);

  // Set Value
  data.UnsafeFormatValue(_T("%02u:%02u"), t.hour, t.minute);

  // Set Comment
  data.UnsafeFormatComment(_T("%02u"), t.second);
}

void
InfoBoxContentFinalAltitudeDiff::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  SetValueFromAltDiff(data, task_stats, task_stats.total.solution_remaining);
}

void
InfoBoxContentFinalAltitudeRequire::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid ||
      !task_stats.total.solution_remaining.IsOk()) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromAltitude(task_stats.total.solution_remaining.GetRequiredAltitude());
}

void
InfoBoxContentTaskSpeed::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.total.travelled.IsDefined()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(task_stats.total.travelled.GetSpeed()));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentTaskSpeedAchieved::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid ||
      !task_stats.total.remaining_effective.IsDefined()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(task_stats.total.remaining_effective.GetSpeed()));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentTaskSpeedInstant::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.IsPirkerSpeedAvailable()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(task_stats.get_pirker_speed()));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentFinalGRTE::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid) {
    data.SetInvalid();
    return;
  }

  fixed gradient = task_stats.total.gradient;

  if (!positive(gradient)) {
    data.SetValue(_T("+++"));
    return;
  }
  if (::GradientValid(gradient)) {
    data.SetValueFromGlideRatio(gradient);
  } else {
    data.SetInvalid();
  }
}

void
InfoBoxContentFinalGR::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  if (!task_stats.task_valid) {
    data.SetInvalid();
    return;
  }

  fixed gradient = task_stats.total.gradient;

  if (!positive(gradient)) {
    data.SetValue(_T("+++"));
    return;
  }
  if (::GradientValid(gradient)) {
    data.UnsafeFormatValue(_T("%d"), (int)gradient);
  } else {
    data.SetInvalid();
  }
}

static constexpr InfoBoxContentHome::PanelContent panels[] = {
    InfoBoxContentHome::PanelContent (
    N_("Set Home"),
    LoadHomePanel),
};

const InfoBoxContentHome::DialogContent InfoBoxContentHome::dlgContent = {
  ARRAY_SIZE(panels), &panels[0], false,
};

const InfoBoxContentHome::DialogContent*
InfoBoxContentHome::GetDialogContent() {
  return &dlgContent;
}

void
InfoBoxContentHomeDistance::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.vector_home.IsValid()) {
    data.SetNotConfigured();
    return;
  }

  // Set Value
  data.SetValueFromDistance(common_stats.vector_home.distance);

  if (basic.track_available) {
    Angle bd = common_stats.vector_home.bearing - basic.track;
    data.SetCommentFromBearingDifference(bd);
  } else
    data.SetCommentInvalid();
}

void
InfoBoxContentHomeAltitudeDiff::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const ComputerSettings &settings = XCSoarInterface::GetComputerSettings();
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const MoreData &more_data = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();

  if (!common_stats.vector_home.IsValid() ||
      !settings.poi.home_location_available ||
      !settings.poi.home_elevation_available) {
    data.SetNotConfigured();
    return;
  }

  // altitude differential

  if (basic.location_available && more_data.NavAltitudeAvailable() &&
      settings.polar.glide_polar_task.IsValid()) {
    const GlideState glide_state(
      basic.location.DistanceBearing(settings.poi.home_location),
      settings.poi.home_elevation + settings.task.safety_height_arrival,
      more_data.nav_altitude,
      calculated.GetWindOrZero());

    const GlideResult &result =
      MacCready::Solve(settings.task.glide,
                       settings.polar.glide_polar_task,
                       glide_state);
    data.SetValueFromArrival(result.pure_glide_altitude_difference);
  }
  else
    data.SetInvalid();

  if (common_stats.vector_home.IsValid())
    data.SetCommentFromDistance(common_stats.vector_home.distance);
  else
     data.SetCommentInvalid();
}


void
InfoBoxContentOLC::Update(InfoBoxData &data)
{
  if (!XCSoarInterface::GetComputerSettings().task.enable_olc ||
      !protected_task_manager) {
    data.SetInvalid();
    return;
  }

  const ContestResult& result_olc = 
    XCSoarInterface::Calculated().contest_stats.GetResult();

  if (result_olc.score < fixed_one) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(result_olc.distance);

  data.UnsafeFormatComment(_T("%.1f pts"), (double)result_olc.score);
}

static constexpr InfoBoxContentOLC::PanelContent panels_olc[] = {
  InfoBoxContentOLC::PanelContent (
    N_("OLC"),
    LoadOnlineContestPanel),
};

const InfoBoxContentOLC::DialogContent InfoBoxContentOLC::dlgContent = {
  ARRAY_SIZE(panels_olc), &panels_olc[0], false,
};

const InfoBoxContentOLC::DialogContent*
InfoBoxContentOLC::GetDialogContent() {
  return &dlgContent;
}

void
InfoBoxContentTaskAATime::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid || !task_stats.total.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  TCHAR value[32];
  TCHAR comment[32];
  FormatTimeTwoLines(value, comment,
                         abs((int) common_stats.aat_time_remaining));

  data.UnsafeFormatValue(negative(common_stats.aat_time_remaining) ?
                            _T("-%s") : _T("%s"), value);
  data.SetValueColor(negative(common_stats.aat_time_remaining) ? 1 : 0);

  data.SetComment(comment);
}

void
InfoBoxContentTaskAATimeDelta::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid || !task_stats.total.IsAchievable()) {
    data.SetInvalid();
    return;
  }

  assert(!negative(task_stats.total.time_remaining));

  fixed diff = task_stats.total.time_remaining -
    common_stats.aat_time_remaining;

  TCHAR value[32];
  TCHAR comment[32];
  const int dd = abs((int)diff);
  FormatTimeTwoLines(value, comment, dd);

  data.UnsafeFormatValue(negative(diff) ? _T("-%s") : _T("%s"), value);

  data.SetComment(comment);

  // Set Color (red/blue/black)
  data.SetValueColor(negative(diff) ? 1 :
                   task_stats.total.time_remaining >
                       common_stats.aat_time_remaining + fixed(5*60) ? 2 : 0);
}

void
InfoBoxContentTaskAADistance::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid ||
      !task_stats.total.planned.IsDefined()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(task_stats.total.planned.GetDistance());
}

void
InfoBoxContentTaskAADistanceMax::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(task_stats.distance_max);
}

void
InfoBoxContentTaskAADistanceMin::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(task_stats.distance_min);
}

void
InfoBoxContentTaskAASpeed::Update(InfoBoxData &data)
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid || !positive(common_stats.aat_speed_remaining)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(common_stats.aat_speed_remaining));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentTaskAASpeedMax::Update(InfoBoxData &data)
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid || !positive(common_stats.aat_speed_max)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(common_stats.aat_speed_max));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentTaskAASpeedMin::Update(InfoBoxData &data)
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;

  if (!common_stats.ordered_has_targets ||
      !task_stats.task_valid || !positive(common_stats.aat_speed_min)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserTaskSpeed(common_stats.aat_speed_min));

  // Set Unit
  data.SetValueUnit(Units::current.task_speed_unit);
}

void
InfoBoxContentTaskTimeUnderMaxHeight::Update(InfoBoxData &data)
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = XCSoarInterface::Calculated().task_stats;
  const fixed maxheight = fixed(protected_task_manager->
                                GetOrderedTaskBehaviour().start_max_height);

  if (!task_stats.task_valid || !positive(maxheight)
      || !protected_task_manager
      || !positive(common_stats.TimeUnderStartMaxHeight)) {
    data.SetInvalid();
    return;
  }

  const int dd = (int)(XCSoarInterface::Basic().time -
      common_stats.TimeUnderStartMaxHeight);

  TCHAR value[32];
  TCHAR comment[32];
  FormatTimeTwoLines(value, comment, dd);

  data.SetValue(value);
  data.SetComment(_("Time Below"));
}

void
InfoBoxContentNextETEVMG::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

  if (!basic.ground_speed_available || !task_stats.task_valid ||
      !task_stats.current_leg.remaining.IsDefined()) {
    data.SetInvalid();
    return;
  }

  const fixed d = task_stats.current_leg.remaining.GetDistance();
  const fixed v = basic.ground_speed;

  if (!task_stats.task_valid ||
      !positive(d) ||
      !positive(v)) {
    data.SetInvalid();
    return;
  }

  TCHAR value[32];
  TCHAR comment[32];
  const int dd = (int)(d/v);
  FormatTimeTwoLines(value, comment, dd);

  data.SetValue(value);
  data.SetComment(comment);
}

void
InfoBoxContentFinalETEVMG::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

  if (!basic.ground_speed_available || !task_stats.task_valid ||
      !task_stats.total.remaining.IsDefined()) {
    data.SetInvalid();
    return;
  }

  const fixed d = task_stats.total.remaining.GetDistance();
  const fixed &v = basic.ground_speed;

  if (!task_stats.task_valid ||
      !positive(d) ||
      !positive(v)) {
    data.SetInvalid();
    return;
  }

  TCHAR value[32];
  TCHAR comment[32];
  const int dd = (int)(d/v);
  FormatTimeTwoLines(value, comment, dd);

  data.SetValue(value);
  data.SetComment(comment);
}

void
InfoBoxContentCruiseEfficiency::Update(InfoBoxData &data)
{
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  if (!task_stats.task_valid || !task_stats.task_started) {
    data.SetInvalid();
    return;
  }

  data.UnsafeFormatValue(_T("%d"), (int) (task_stats.cruise_efficiency * 100));
  data.SetCommentFromVerticalSpeed(task_stats.effective_mc, false);
}
