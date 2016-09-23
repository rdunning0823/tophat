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

#include "ComputerProfile.hpp"
#include "AirspaceConfig.hpp"
#include "TaskProfile.hpp"
#include "TrackingProfile.hpp"
#include "ProfileKeys.hpp"
#include "ContestProfile.hpp"
#include "Map.hpp"
#include "Computer/Settings.hpp"

namespace Profile {
  static void Load(const ProfileMap &map, WindSettings &settings);
  static void Load(const ProfileMap &map, PolarSettings &settings);
  static void Load(const ProfileMap &map, LoggerSettings &settings);
  static void Load(const ProfileMap &map, TeamCodeSettings &settings);
  static void Load(const ProfileMap &map, FilePickAndDownloadSettings &settings);
  static void Load(const ProfileMap &map, VoiceSettings &settings);
  static void Load(const ProfileMap &map, PlacesOfInterestSettings &settings);
  static void Load(const ProfileMap &map, FeaturesSettings &settings);
  static void Load(const ProfileMap &map, CirclingSettings &settings);
  static void Load(const ProfileMap &map, WaveSettings &settings);
};

void
Profile::Load(const ProfileMap &map, WindSettings &settings)
{
  unsigned user_wind_source = (unsigned)settings.GetUserWindSource();
  if (map.Get(ProfileKeys::UserWindSource, user_wind_source))
    settings.SetUserWindSource((UserWindSource)user_wind_source);
}

void
Profile::Load(const ProfileMap &map, PolarSettings &settings)
{
  fixed degradation;
  if (map.Get(ProfileKeys::PolarDegradation, degradation) &&
      degradation >= fixed(0.5) && degradation <= fixed(1))
    settings.SetDegradationFactor(degradation);

  map.Get(ProfileKeys::AutoBugs, settings.auto_bugs);
}

void
Profile::Load(const ProfileMap &map, LoggerSettings &settings)
{
  map.Get(ProfileKeys::LoggerTimeStepCruise, settings.time_step_cruise);
  map.Get(ProfileKeys::LoggerTimeStepCircling, settings.time_step_circling);

  // auto_logger is hard coded to default (ON)
  map.Get(ProfileKeys::LoggerID, settings.logger_id);
  map.Get(ProfileKeys::PilotName, settings.pilot_name);
  map.Get(ProfileKeys::EnableFlightLogger, settings.enable_flight_logger);
  map.Get(ProfileKeys::EnableNMEALogger, settings.enable_nmea_logger);
}

void
Profile::Load(const ProfileMap &map, TeamCodeSettings &settings)
{
  map.Get(ProfileKeys::TeamcodeRefWaypoint, settings.team_code_reference_waypoint);
}

void
Profile::Load(const ProfileMap &map, FilePickAndDownloadSettings &settings)
{
  map.Get(ProfileKeys::FilePickAndDownloadAreaFilter, settings.area_filter);
  map.Get(ProfileKeys::FilePickAndDownloadSubAreaFilter, settings.subarea_filter);
}

void
Profile::Load(const ProfileMap &map, VoiceSettings &settings)
{
  map.Get(ProfileKeys::VoiceClimbRate, settings.voice_climb_rate_enabled);
  map.Get(ProfileKeys::VoiceTerrain, settings.voice_terrain_enabled);
  map.Get(ProfileKeys::VoiceWaypointDistance,
          settings.voice_waypoint_distance_enabled);
  map.Get(ProfileKeys::VoiceTaskAltitudeDifference,
          settings.voice_task_altitude_difference_enabled);
  map.Get(ProfileKeys::VoiceMacCready, settings.voice_mac_cready_enabled);
  map.Get(ProfileKeys::VoiceNewWaypoint, settings.voice_new_waypoint_enabled);
  map.Get(ProfileKeys::VoiceInSector, settings.voice_in_sector_enabled);
  map.Get(ProfileKeys::VoiceAirspace, settings.voice_airspace_enabled);
}

void
Profile::Load(const ProfileMap &map, PlacesOfInterestSettings &settings)
{
  map.Get(ProfileKeys::HomeWaypoint, settings.home_waypoint);
  settings.home_location_available =
    map.GetGeoPoint(ProfileKeys::HomeLocation, settings.home_location);
  map.Get(ProfileKeys::HomeElevationAvailable, settings.home_elevation_available);
  if (settings.home_elevation_available)
    map.Get(ProfileKeys::HomeElevation, settings.home_elevation);

  map.GetGeoPoint(ProfileKeys::ATCReference, settings.atc_reference);
}

void
Profile::Load(const ProfileMap &map, FeaturesSettings &settings)
{
  // hard code to line for Top Hat
  settings.final_glide_terrain = FeaturesSettings::FinalGlideTerrain::LINE;
  //map.GetEnum(ProfileKeys::FinalGlideTerrain, settings.final_glide_terrain);
  map.Get(ProfileKeys::BlockSTF, settings.block_stf_enabled);
  map.Get(ProfileKeys::EnableNavBaroAltitude, settings.nav_baro_altitude_enabled);
}

void
Profile::Load(const ProfileMap &map, CirclingSettings &settings)
{
  map.Get(ProfileKeys::EnableExternalTriggerCruise,
          settings.external_trigger_cruise_enabled);
  fixed min_turn_rate;
  if (map.Get(ProfileKeys::CirclingMinTurnRate, min_turn_rate))
    settings.min_turn_rate = settings.min_turn_rate.Degrees(min_turn_rate);
  map.Get(ProfileKeys::CirclingCruiseClimbSwitch, settings.cruise_climb_switch);
  map. Get(ProfileKeys::CirclingClimbCruiseSwitch, settings.climb_cruise_switch);
}

void
Profile::Load(const ProfileMap &map, WaveSettings &settings)
{
  map.Get(ProfileKeys::WaveAssistant, settings.enabled);
}

static bool
LoadUTCOffset(const ProfileMap &map, RoughTimeDelta &value_r)
{
  /* NOTE: Until 6.2.4 utc_offset was stored as a positive int in the
     settings file (with negative offsets stored as "utc_offset + 24 *
     3600").  Later versions will create a new signed settings key. */

  int value;
  if (map.Get(ProfileKeys::UTCOffsetSigned, value)) {
  } else if (map.Get(ProfileKeys::UTCOffset, value)) {
    if (value > 12 * 3600)
      value = (value % (24 * 3600)) - 24 * 3600;
  } else
    /* no profile value present */
    return false;

  if (value > 13 * 3600 || value < -13 * 3600)
    /* illegal value */
    return false;

  value_r = RoughTimeDelta::FromSeconds(value);
  return true;
}

void
Profile::Load(const ProfileMap &map, ComputerSettings &settings)
{
  Load(map, settings.wind);
  Load(map, settings.polar);
  Load(map, settings.team_code);
  Load(map, settings.file_pick_and_download);
  Load(map, settings.voice);
  Load(map, settings.poi);
  Load(map, settings.features);
  Load(map, settings.airspace);
  Load(map, settings.circling);
  Load(map, settings.wave);

  map.GetEnum(ProfileKeys::AverEffTime, settings.average_eff_time);

  map.Get(ProfileKeys::SetSystemTimeFromGPS, settings.set_system_time_from_gps);

  LoadUTCOffset(map, settings.utc_offset);

  Load(map, settings.task);
  Load(map, settings.contest);
  Load(map, settings.logger);

#ifdef HAVE_TRACKING
  Load(map, settings.tracking);
#endif
}
