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

#include "Menu/ButtonLabel.hpp"
#include "Language/Language.hpp"
#include "Logger/Logger.hpp"
#include "MainWindow.hpp"
#include "Interface.hpp"
#include "Gauge/BigTrafficWidget.hpp"
#include "Computer/Settings.hpp"
#include "Components.hpp"
#include "MapSettings.hpp"
#include "Waypoint/Waypoints.hpp"
#include "Engine/Airspace/Airspaces.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Terrain/RasterWeatherStore.hpp"
#include "Device/device.hpp"
#include "PageActions.hpp"
#include "Util/StringUtil.hpp"
#include "Util/Macros.hpp"
#include "Net/HTTP/Features.hpp"
#include "UIState.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Formatter/UserUnits.hpp"

#include <stdlib.h>

/**
 * If Condition is true, Macro in Buffer will be replaced by TrueText,
 * otherwise by FalseText.
 * @param Condition Condition to be checked
 * @param Buffer Buffer string
 * @param Macro The string that will be replaced
 * @param TrueText The replacement if Condition is true
 * @param FalseText The replacement if Condition is false
 * @param Size (?)
 */
static void
CondReplaceInString(bool Condition, TCHAR *Buffer, const TCHAR *Macro,
                    const TCHAR *TrueText, const TCHAR *FalseText, size_t Size)
{
  if (Condition)
    ReplaceInString(Buffer, Macro, TrueText, Size);
  else
    ReplaceInString(Buffer, Macro, FalseText, Size);
}

/**
 * If Condition is true, Macro in Buffer will be replaced by TrueText,
 * otherwise by FalseText.
 * Allows an optional prefix to the true text, else leave blank
 * @param condition Condition to be checked
 * @param buffer Buffer string
 * @param macro The string that will be replaced
 * @param true_text_prefix. Prefix to text if Condition is true. not NULL
 * @param true_textThe replacement if Condition is true
 * @param false_text The replacement if Condition is false
 * @param size (?)
 */
static void
CondReplaceInString(bool condition, TCHAR *buffer, const TCHAR *macro,
                    const TCHAR *true_text_prefix, const TCHAR *true_text,
                    const TCHAR *false_text, size_t size)
{
  assert(true_text_prefix != nullptr);
  assert(true_text != nullptr);

  if (condition) {
    static StaticString<256> buff2;
    buff2.Format(_T("%s%s"), true_text_prefix, true_text);

    ReplaceInString(buffer, macro, buff2.c_str(), size);
  }
  else
    ReplaceInString(buffer, macro, false_text, size);
}


static bool
ExpandTaskMacros(TCHAR *OutBuffer, size_t Size,
                 const DerivedInfo &calculated,
                 const ComputerSettings &settings_computer)
{
  const TaskStats &task_stats = calculated.task_stats;
  const TaskStats &ordered_task_stats = calculated.ordered_task_stats;
  const CommonStats &common_stats = calculated.common_stats;

  bool invalid = false;

  if (_tcsstr(OutBuffer, _T("$(CheckTaskResumed)"))) {
    // TODO code: check, does this need to be set with temporary task?
    if (common_stats.task_type == TaskType::ABORT ||
        common_stats.task_type == TaskType::GOTO)
      invalid = true;
    ReplaceInString(OutBuffer, _T("$(CheckTaskResumed)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckTask)"))) {
    if (!task_stats.task_valid)
      invalid = true;

    ReplaceInString(OutBuffer, _T("$(CheckTask)"), _T(""), Size);
  }

  if (protected_task_manager == nullptr)
    return true;

  ProtectedTaskManager::Lease task_manager(*protected_task_manager);

  const AbstractTask *task = task_manager->GetActiveTask();
  if (task == nullptr || !task_stats.task_valid ||
      common_stats.task_type == TaskType::GOTO) {

    if (_tcsstr(OutBuffer, _T("$(WaypointNext)"))) {
      invalid = true;
      ReplaceInString(OutBuffer, _T("$(WaypointNext)"),
          _("Next Turnpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPrevious)"))) {
      invalid = true;
      ReplaceInString(OutBuffer, _T("$(WaypointPrevious)"),
          _("Previous Turnpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointNextArm)"))) {
      invalid = true;
      ReplaceInString(OutBuffer, _T("$(WaypointNextArm)"),
          _("Next Turnpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPreviousArm)"))) {
      invalid = true;
      ReplaceInString(OutBuffer, _T("$(WaypointPreviousArm)"),
          _("Previous Turnpoint"), Size);
    }

  } else if (common_stats.task_type == TaskType::ABORT) {

    if (_tcsstr(OutBuffer, _T("$(WaypointNext)"))) {
      if (!common_stats.active_has_next)
        invalid = true;

      CondReplaceInString(common_stats.next_is_last,
                          OutBuffer,
                          _T("$(WaypointNext)"),
                          _("Furthest Landpoint"),
                          _("Next Landpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPrevious)"))) {
      if (!common_stats.active_has_previous)
        invalid = true;

      CondReplaceInString(common_stats.previous_is_first,
                          OutBuffer,
                          _T("$(WaypointPrevious)"),
                          _("Closest Landpoint"),
                          _("Previous Landpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointNextArm)"))) {
      if (!common_stats.active_has_next)
        invalid = true;

      CondReplaceInString(common_stats.next_is_last,
                          OutBuffer,
                          _T("$(WaypointNextArm)"),
                          _("Furthest Landpoint"),
                          _("Next Landpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPreviousArm)"))) {
      if (!common_stats.active_has_previous)
        invalid = true;

      CondReplaceInString(common_stats.previous_is_first,
                          OutBuffer,
                          _T("$(WaypointPreviousArm)"),
                          _("Closest Landpoint"),
                          _("Previous Landpoint"), Size);
    }

  } else { // ordered task mode

    const bool next_is_final = common_stats.next_is_last;
    const bool previous_is_start = common_stats.previous_is_first;
    const bool has_optional_starts = ordered_task_stats.has_optional_starts;

    if (_tcsstr(OutBuffer, _T("$(WaypointNext)"))) {
      // Waypoint\nNext
      if (!common_stats.active_has_next)
        invalid = true;

      CondReplaceInString(next_is_final,
                          OutBuffer,
                          _T("$(WaypointNext)"),
                          _("Finish Turnpoint"),
                          _("Next Turnpoint"), Size);

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPrevious)"))) {

      if (has_optional_starts && !common_stats.active_has_previous) {
        ReplaceInString(OutBuffer, _T("$(WaypointPrevious)"), _("Next Startpoint"), Size);
      } else {

        CondReplaceInString(previous_is_start,
                            OutBuffer,
                            _T("$(WaypointPrevious)"),
                            _("Start Turnpoint"),
                            _("Previous Turnpoint"), Size);

        if (!common_stats.active_has_previous)
          invalid = true;
      }

    } else if (_tcsstr(OutBuffer, _T("$(WaypointNextArm)"))) {
      // Waypoint\nNext

      switch (task_manager->GetOrderedTask().GetTaskAdvance().GetState()) {
      case TaskAdvance::MANUAL:
      case TaskAdvance::AUTO:
      case TaskAdvance::START_ARMED:
      case TaskAdvance::TURN_ARMED:
        CondReplaceInString(next_is_final,
                            OutBuffer,
                            _T("$(WaypointNextArm)"),
                            _("Finish Turnpoint"),
                            _("Next Turnpoint"), Size);
        if (!common_stats.active_has_next)
          invalid = true;
        break;
      case TaskAdvance::START_DISARMED:
        ReplaceInString(OutBuffer, _T("$(WaypointNextArm)"), _("Arm start"), Size);
        break;
      case TaskAdvance::TURN_DISARMED:
        ReplaceInString(OutBuffer, _T("$(WaypointNextArm)"), _("Arm turn"), Size);
        break;
      }

    } else if (_tcsstr(OutBuffer, _T("$(WaypointPreviousArm)"))) {

      switch (task_manager->GetOrderedTask().GetTaskAdvance().GetState()) {
      case TaskAdvance::MANUAL:
      case TaskAdvance::AUTO:
      case TaskAdvance::START_DISARMED:
      case TaskAdvance::TURN_DISARMED:

        if (has_optional_starts && !common_stats.active_has_previous) {
          ReplaceInString(OutBuffer, _T("$(WaypointPreviousArm)"), _("Next Startpoint"), Size);
        } else {

          CondReplaceInString(previous_is_start,
                              OutBuffer,
                              _T("$(WaypointPreviousArm)"),
                              _("Start Turnpoint"),
                              _("Previous Turnpoint"), Size);

          if (!common_stats.active_has_previous)
            invalid = true;
        }

        break;
      case TaskAdvance::START_ARMED:
        ReplaceInString(OutBuffer, _T("$(WaypointPreviousArm)"), _("Disarm start"), Size);
        break;
      case TaskAdvance::TURN_ARMED:
        ReplaceInString(OutBuffer, _T("$(WaypointPreviousArm)"), _("Disarm turn"), Size);
        break;
      }
    } else if (_tcscmp(OutBuffer, _T("Resume")) == 0) {
      invalid = true;
    }
  }

  if (_tcsstr(OutBuffer, _T("$(NavBarDestinationType)"))) {
    if (task_manager->GetOrderedTask().GetFactoryType() == TaskFactoryType::AAT &&
        task_manager->GetMode() == TaskType::ORDERED) {
      const UISettings &ui_settings = CommonInterface::GetUISettings();
      CondReplaceInString(ui_settings.navbar_navigate_to_aat_target,
                          OutBuffer,
                          _T("$(NavBarDestinationType)"),
                          _("_NavBarToCenter"),
                          _("_NavBarToTarget"), Size);
    } else { //hide
      ReplaceInString(OutBuffer, _T("$(NavBarDestinationType)"), _T(""), Size);
    }
  }

  if (_tcsstr(OutBuffer, _T("$(AdvanceArmed)"))) {
    switch (task_manager->GetOrderedTask().GetTaskAdvance().GetState()) {
    case TaskAdvance::MANUAL:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Advance\n(manual)"), Size);
      invalid = true;
      break;
    case TaskAdvance::AUTO:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Advance\n(auto)"), Size);
      invalid = true;
      break;
    case TaskAdvance::START_ARMED:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Abort\nStart"), Size);
      invalid = false;
      break;
    case TaskAdvance::START_DISARMED:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Arm\nStart"), Size);
      invalid = false;
      break;
    case TaskAdvance::TURN_ARMED:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Abort\nTurn"), Size);
      invalid = false;
      break;
    case TaskAdvance::TURN_DISARMED:
      ReplaceInString(OutBuffer, _T("$(AdvanceArmed)"),
                      _("Arm\nTurn"), Size);
      invalid = false;
      break;
    }
  }

  if (_tcsstr(OutBuffer, _T("$(CheckAutoMc)"))) {
    if (!task_stats.task_valid
        && settings_computer.task.IsAutoMCFinalGlideEnabled())
      invalid = true;

    ReplaceInString(OutBuffer, _T("$(CheckAutoMc)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(TaskAbortToggleActionName)"))) {
    if (common_stats.task_type == TaskType::GOTO) {
      CondReplaceInString(ordered_task_stats.task_valid,
                          OutBuffer, _T("$(TaskAbortToggleActionName)"),
                          _("Resume"), _("Abort"), Size);
    } else
      CondReplaceInString(common_stats.task_type == TaskType::ABORT,
                          OutBuffer, _T("$(TaskAbortToggleActionName)"),
                          _("Resume"), _("Abort"), Size);
  }

  return invalid;
}

static void
ExpandTrafficMacros(TCHAR *buffer, size_t size)
{
  TrafficWidget *widget = (TrafficWidget *)
    CommonInterface::main_window->GetFlavourWidget(_T("Traffic"));
  if (widget == nullptr)
    return;

  CondReplaceInString(widget->GetAutoZoom(), buffer,
                      _T("$(TrafficZoomAutoToggleActionName)"),
                      _("Manual"), _("Auto"), size);
  CondReplaceInString(widget->GetNorthUp(), buffer,
                      _T("$(TrafficNorthUpToggleActionName)"),
                      _("Track up"), _("North up"), size);
}

static const NMEAInfo &
Basic()
{
  return CommonInterface::Basic();
}

static const DerivedInfo &
Calculated()
{
  return CommonInterface::Calculated();
}

static const ComputerSettings &
GetComputerSettings()
{
  return CommonInterface::GetComputerSettings();
}

static const MapSettings &
GetMapSettings()
{
  return CommonInterface::GetMapSettings();
}

static const UIState &
GetUIState()
{
  return CommonInterface::GetUIState();
}

bool
ButtonLabel::ExpandMacros(const TCHAR *In, TCHAR *OutBuffer, size_t Size)
{
  // ToDo, check Buffer Size
  bool invalid = false;
  CopyString(OutBuffer, gettext(In), Size);

  if (_tcsstr(OutBuffer, _T("$(")) == nullptr)
    return false;

  if (_tcsstr(OutBuffer, _T("$(CheckAirspace)"))) {
    if (airspace_database.IsEmpty())
      invalid = true;

    ReplaceInString(OutBuffer, _T("$(CheckAirspace)"), _T(""), Size);
  }

  invalid |= ExpandTaskMacros(OutBuffer, Size,
                              Calculated(), GetComputerSettings());

  ExpandTrafficMacros(OutBuffer, Size);

  if (_tcsstr(OutBuffer, _T("$(CheckFLARM)"))) {
    if (!Basic().flarm.status.available)
      invalid = true;
    ReplaceInString(OutBuffer, _T("$(CheckFLARM)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckWeather)"))) {
    if (rasp == nullptr || rasp->GetItemCount() == 0)
      invalid = true;
    ReplaceInString(OutBuffer, _T("$(CheckWeather)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckCircling)"))) {
    if (!Calculated().circling)
      invalid = true;
    ReplaceInString(OutBuffer, _T("$(CheckCircling)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckVega)"))) {
    if (devVarioFindVega() == nullptr)
      invalid = true;
    ReplaceInString(OutBuffer, _T("$(CheckVega)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckReplay)"))) {
    if (CommonInterface::MovementDetected())
      invalid = true;

    ReplaceInString(OutBuffer, _T("$(CheckReplay)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckWaypointFile)"))) {
    invalid |= way_points.IsEmpty();
    ReplaceInString(OutBuffer, _T("$(CheckWaypointFile)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckLogger)"))) {
    invalid |= Basic().gps.replay;
    ReplaceInString(OutBuffer, _T("$(CheckLogger)"), _T(""), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(CheckNet)"))) {
#ifndef HAVE_HTTP
    invalid = true;
#endif

    ReplaceInString(OutBuffer, _T("$(CheckNet)"), _T(""), Size);
  }
  if (_tcsstr(OutBuffer, _T("$(CheckTerrain)"))) {
    if (!Calculated().terrain_valid)
      invalid = true;

    ReplaceInString(OutBuffer, _T("$(CheckTerrain)"), _T(""), Size);
  }

  CondReplaceInString(logger != nullptr && logger->IsLoggerActive(), OutBuffer,
                      _T("$(LoggerActive)"), _("Stop"),
                      _("Start"), Size);

  if (_tcsstr(OutBuffer, _T("$(SnailTrailToggleName)"))) {
    switch (GetMapSettings().trail.length) {
    case TrailSettings::Length::OFF:
      ReplaceInString(OutBuffer, _T("$(SnailTrailToggleName)"),
                      _("Long"), Size);
      break;
    case TrailSettings::Length::LONG:
      ReplaceInString(OutBuffer, _T("$(SnailTrailToggleName)"),
                      _("Short"), Size);
      break;
    case TrailSettings::Length::SHORT:
      ReplaceInString(OutBuffer, _T("$(SnailTrailToggleName)"),
                      _("Full"), Size);
      break;
    case TrailSettings::Length::FULL:
      ReplaceInString(OutBuffer, _T("$(SnailTrailToggleName)"),
                      _("Off"), Size);
      break;
    }
  }

  if (_tcsstr(OutBuffer, _T("$(AirSpaceToggleName)"))) {
    ReplaceInString(OutBuffer, _T("$(AirSpaceToggleName)"),
                    GetMapSettings().airspace.enable ? _("Off") : _("On"), Size);
  }

  if (_tcsstr(OutBuffer, _T("$(TerrainTopologyToggleName)"))) {
    char val = 0;
    if (GetMapSettings().topography_enabled)
      val++;
    if (GetMapSettings().terrain.enable)
      val += (char)2;
    switch (val) {
    case 0:
      ReplaceInString(OutBuffer, _T("$(TerrainTopologyToggleName)"),
                      _("Topography On"), Size);
      break;
    case 1:
      ReplaceInString(OutBuffer, _T("$(TerrainTopologyToggleName)"),
                      _("Terrain On"), Size);
      break;
    case 2:
      ReplaceInString(OutBuffer, _T("$(TerrainTopologyToggleName)"),
                      _("Terrain + Topography"), Size);
      break;
    case 3:
      ReplaceInString(OutBuffer, _T("$(TerrainTopologyToggleName)"),
                      _("Terrain Off"), Size);
      break;
    }
  }

  if (_tcsstr(OutBuffer, _T("$(TerrainTopographyToggleName)"))) {
    char val = 0;
    if (GetMapSettings().topography_enabled)
      val++;
    if (GetMapSettings().terrain.enable)
      val += (char)2;
    switch (val) {
    case 0:
      ReplaceInString(OutBuffer, _T("$(TerrainTopographyToggleName)"),
                      _("Topography On"), Size);
      break;
    case 1:
      ReplaceInString(OutBuffer, _T("$(TerrainTopographyToggleName)"),
                      _("Terrain On"), Size);
      break;
    case 2:
      ReplaceInString(OutBuffer, _T("$(TerrainTopographyToggleName)"),
                      _("Terrain + Topography"), Size);
      break;
    case 3:
      ReplaceInString(OutBuffer, _T("$(TerrainTopographyToggleName)"),
                      _("Terrain Off"), Size);
      break;
    }
  }

  CondReplaceInString(CommonInterface::main_window->GetFullScreen(), OutBuffer,
                      _T("$(FullScreenToggleActionName)"),
                      _("Off"), _("On"), Size);
  CondReplaceInString(GetMapSettings().auto_zoom_enabled, OutBuffer,
                      _T("$(ZoomAutoToggleActionName)"),
                      _("Manual"), _("Auto"), Size);

  CondReplaceInString(GetMapSettings().topography_enabled, OutBuffer,
                      _T("$(TopographyToggleActionName)"),
                      _T("_CheckMark "), _("Roads, cities, water"), _("Roads, cities, water"), Size);
  CondReplaceInString(GetMapSettings().terrain.enable, OutBuffer,
                      _T("$(TerrainToggleActionName)"),
                      _T("_CheckMark "), _("Terrain"), _("Terrain"), Size);
  CondReplaceInString(GetMapSettings().airspace.enable, OutBuffer,
                      _T("$(AirspaceToggleActionName)"),
                      _T("_CheckMark "), _("Airspace"), _("Airspace"), Size);

  CondReplaceInString(GetMapSettings().cruise_orientation ==
      MapOrientation::NORTH_UP, OutBuffer,
                      _T("$(OrientationNorthUp)"),
                      _T("_CheckMark "), _("North up"), _("North up"), Size);

  CondReplaceInString(GetMapSettings().cruise_orientation ==
      MapOrientation::TARGET_UP, OutBuffer,
                      _T("$(OrientationTargetUp)"),
                      _T("_CheckMark "), _("Target up"), _("Target up"), Size);

  CondReplaceInString(GetMapSettings().cruise_orientation ==
      MapOrientation::HEADING_UP, OutBuffer,
                      _T("$(OrientationHeadingUp)"),
                      _T("_CheckMark "), _("Heading up"), _("Heading up"), Size);

  CondReplaceInString(GetMapSettings().cruise_orientation ==
      MapOrientation::TRACK_UP, OutBuffer,
                      _T("$(OrientationTrackUp)"),
                      _T("_CheckMark "), _("Track up"), _("Track up"), Size);

  if (_tcsstr(OutBuffer, _T("$(MapLabelsToggleActionName)"))) {
    static const TCHAR *const labels[] = {
      N_("All"),
      N_("Task & Landables"),
      N_("Task"),
      N_("None"),
      N_("Task & Airfields"),
    };

    static constexpr unsigned int n = ARRAY_SIZE(labels);
    unsigned int i = (unsigned)GetMapSettings().waypoint.label_selection;
    ReplaceInString(OutBuffer, _T("$(MapLabelsToggleActionName)"),
                    gettext(labels[(i + 1) % n]), Size);
  }

  WaypointRendererSettings::LabelSelection &wls =
    CommonInterface::SetMapSettings().waypoint.label_selection;

  CondReplaceInString(wls ==
      WaypointRendererSettings::LabelSelection::ALL, OutBuffer,
                      _T("$(DeclutterLabelsAll)"),
                      _T("_CheckMark "), _("All"), _("All"), Size);

  CondReplaceInString(wls ==
      WaypointRendererSettings::LabelSelection::TASK_AND_LANDABLE, OutBuffer,
                      _T("$(DeclutterLabelsTaskLandables)"),
                      _T("_CheckMark "), _("Task, Landables"), _("Task, Landables"), Size);

  CondReplaceInString(wls ==
      WaypointRendererSettings::LabelSelection::TASK, OutBuffer,
                      _T("$(DeclutterLabelsTask)"),
                      _T("_CheckMark "), _("Task"), _("Task"), Size);

  CondReplaceInString(wls ==
      WaypointRendererSettings::LabelSelection::NONE, OutBuffer,
                      _T("$(DeclutterLabelsNone)"),
                      _T("_CheckMark "), _("None"), _("None"), Size);

  CondReplaceInString(wls ==
      WaypointRendererSettings::LabelSelection::TASK_AND_AIRFIELD, OutBuffer,
                      _T("$(DeclutterLabelsTaskAirfields)"),
                      _T("_CheckMark "), _("Task, Airfields"), _("Task, Airfields"), Size);

  CondReplaceInString(GetComputerSettings().task.auto_mc,
                      OutBuffer, _T("$(MacCreadyToggleActionName)"),
                      _("Manual"), _("Auto"), Size);
  CondReplaceInString(GetUIState().auxiliary_enabled,
                      OutBuffer, _T("$(AuxInfoToggleActionName)"),
                      _("Off"), _("On"), Size);

  CondReplaceInString(GetUIState().force_display_mode == DisplayMode::CIRCLING,
                      OutBuffer, _T("$(DispModeClimbShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetUIState().force_display_mode == DisplayMode::CRUISE,
                      OutBuffer, _T("$(DispModeCruiseShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetUIState().force_display_mode == DisplayMode::NONE,
                      OutBuffer, _T("$(DispModeAutoShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetUIState().force_display_mode == DisplayMode::FINAL_GLIDE,
                      OutBuffer, _T("$(DispModeFinalShortIndicator)"),
                      _T("(*)"), _T(""), Size);

  CondReplaceInString(GetMapSettings().airspace.altitude_mode == AirspaceDisplayMode::ALLON,
                      OutBuffer, _T("$(AirspaceModeAllShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().airspace.altitude_mode == AirspaceDisplayMode::CLIP,
                      OutBuffer, _T("$(AirspaceModeClipShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().airspace.altitude_mode == AirspaceDisplayMode::AUTO,
                      OutBuffer, _T("$(AirspaceModeAutoShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().airspace.altitude_mode == AirspaceDisplayMode::ALLBELOW,
                      OutBuffer, _T("$(AirspaceModeBelowShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().airspace.altitude_mode == AirspaceDisplayMode::ALLOFF,
                      OutBuffer, _T("$(AirspaceModeAllOffIndicator)"),
                      _T("(*)"), _T(""), Size);

  CondReplaceInString(GetMapSettings().trail.length == TrailSettings::Length::OFF,
                      OutBuffer, _T("$(SnailTrailOffShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().trail.length == TrailSettings::Length::SHORT,
                      OutBuffer, _T("$(SnailTrailShortShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().trail.length == TrailSettings::Length::LONG,
                      OutBuffer, _T("$(SnailTrailLongShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().trail.length == TrailSettings::Length::FULL,
                      OutBuffer, _T("$(SnailTrailFullShortIndicator)"),
                      _T("(*)"), _T(""), Size);

  CondReplaceInString(!GetMapSettings().airspace.enable,
                      OutBuffer, _T("$(AirSpaceOffShortIndicator)"),
                      _T("(*)"), _T(""), Size);
  CondReplaceInString(GetMapSettings().airspace.enable,
                      OutBuffer, _T("$(AirSpaceOnShortIndicator)"),
                      _T("(*)"), _T(""), Size);

  CondReplaceInString(CommonInterface::GetUISettings().traffic.enable_gauge,
                      OutBuffer, _T("$(FlarmDispToggleActionName)"),
                      _("Off"), _("On"), Size);

  CondReplaceInString(GetMapSettings().auto_zoom_enabled, OutBuffer,
                      _T("$(ZoomAutoToggleActionName)"),
                      _("Manual"), _("Auto"), Size);

  bool show_screens_button =
      (CommonInterface::GetUISettings().screens_button_location ==
      UISettings::ScreensButtonLocation::MAP) ||
      (CommonInterface::SetUISettings().pages.n_pages < 2);

  CondReplaceInString(show_screens_button, OutBuffer,
                      _T("$(SwitchScreen)"),
                      _(""), _("Switch screens"), Size);

  if (_tcsstr(OutBuffer, _T("$(NextPageName)"))) {
    TCHAR label[30];
    const PageLayout &page =
      CommonInterface::GetUISettings().pages.pages[PageActions::NextIndex()];
    page.MakeTitle(CommonInterface::GetUISettings().info_boxes, label, true);
    ReplaceInString(OutBuffer, _T("$(NextPageName)"), label, Size);

  } else if (_tcsstr(OutBuffer, _T("$(Dump)"))) {
    fixed ballast = GetComputerSettings().polar.glide_polar_task.GetBallastLitres();
    if (!positive(ballast)) {
      ReplaceInString(OutBuffer, _T("$(Dump)"), _T(""), Size); //hide button
    }
    else if (GetComputerSettings().polar.ballast_timer_active)
      ReplaceInString(OutBuffer, _T("$(Dump)"), _("Dump stop"), Size);
    else {
      StaticString<50>buffer;
      StaticString<20>units;
      FormatUserVolume(ballast, units.buffer(), true);
      buffer.Format(_T("%s %s"), _("Dump"), units.c_str());
      ReplaceInString(OutBuffer, _T("$(Dump)"), buffer.c_str(), Size);
    }
  }

  return invalid;
}
