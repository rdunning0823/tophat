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

#include "Profile/ComputerProfile.hpp"
#include "Profile/AirspaceConfig.hpp"
#include "Profile/TaskProfile.hpp"
#include "Profile/TrackingProfile.hpp"
#include "Profile/Profile.hpp"
#include "ComputerSettings.hpp"

namespace Profile {
  static void Load(WindSettings &settings);
  static void Load(PolarSettings &settings);
  static void Load(LoggerSettings &settings);
  static void Load(TeamCodeSettings &settings);
  static void Load(FilePickAndDownloadSettings &settings);
  static void Load(VoiceSettings &settings);
  static void Load(PlacesOfInterestSettings &settings);
  static void Load(FeaturesSettings &settings);
};

void
Profile::Load(WindSettings &settings)
{
  unsigned auto_wind_mode = settings.GetLegacyAutoWindMode();
  if (Get(ProfileKeys::AutoWind, auto_wind_mode))
    settings.SetLegacyAutoWindMode(auto_wind_mode);

  Get(ProfileKeys::ExternalWind, settings.use_external_wind);
}

void
Profile::Load(PolarSettings &settings)
{
  fixed degradation;
  if (Get(ProfileKeys::PolarDegradation, degradation) &&
      degradation >= fixed_half && degradation <= fixed_one)
    settings.SetDegradationFactor(degradation);
}

void
Profile::Load(LoggerSettings &settings)
{
  Get(ProfileKeys::LoggerTimeStepCruise, settings.time_step_cruise);
  Get(ProfileKeys::LoggerTimeStepCircling, settings.time_step_circling);

  Get(ProfileKeys::LoggerID, settings.logger_id.buffer(),
      settings.logger_id.MAX_SIZE);
  Get(ProfileKeys::PilotName, settings.pilot_name.buffer(),
      settings.pilot_name.MAX_SIZE);
  Get(ProfileKeys::EnableFlightLogger, settings.enable_flight_logger);
  Get(ProfileKeys::EnableNMEALogger, settings.enable_nmea_logger);
}

void
Profile::Load(TeamCodeSettings &settings)
{
  Get(ProfileKeys::TeamcodeRefWaypoint, settings.team_code_reference_waypoint);
}

void
Profile::Load(FilePickAndDownloadSettings &settings)
{
  Get(ProfileKeys::FilePickAndDownloadAreaFilter, settings.area_filter);
  Get(ProfileKeys::FilePickAndDownloadSubAreaFilter, settings.subarea_filter);
}

void
Profile::Load(VoiceSettings &settings)
{
  Get(ProfileKeys::VoiceClimbRate, settings.voice_climb_rate_enabled);
  Get(ProfileKeys::VoiceTerrain, settings.voice_terrain_enabled);
  Get(ProfileKeys::VoiceWaypointDistance, settings.voice_waypoint_distance_enabled);
  Get(ProfileKeys::VoiceTaskAltitudeDifference,
      settings.voice_task_altitude_difference_enabled);
  Get(ProfileKeys::VoiceMacCready, settings.voice_mac_cready_enabled);
  Get(ProfileKeys::VoiceNewWaypoint, settings.voice_new_waypoint_enabled);
  Get(ProfileKeys::VoiceInSector, settings.voice_in_sector_enabled);
  Get(ProfileKeys::VoiceAirspace, settings.voice_airspace_enabled);
}

void
Profile::Load(PlacesOfInterestSettings &settings)
{
  Get(ProfileKeys::HomeWaypoint, settings.home_waypoint);
  settings.home_location_available =
    GetGeoPoint(ProfileKeys::HomeLocation, settings.home_location);
  Get(ProfileKeys::HomeElevationAvailable, settings.home_elevation_available);
  if (settings.home_elevation_available)
    Get(ProfileKeys::HomeElevation, settings.home_elevation);
}

void
Profile::Load(FeaturesSettings &settings)
{
  // hard code to line for Top Hat
  settings.final_glide_terrain = FeaturesSettings::FinalGlideTerrain::FGT_LINE;

  Get(ProfileKeys::BlockSTF, settings.block_stf_enabled);
  Get(ProfileKeys::EnableNavBaroAltitude, settings.nav_baro_altitude_enabled);
}

void
Profile::Load(ComputerSettings &settings)
{
  Load(settings.wind);
  Load(settings.polar);
  Load(settings.team_code);
  Load(settings.file_pick_and_download);
  Load(settings.voice);
  Load(settings.poi);
  Load(settings.features);
  Load(settings.airspace);

  Get(ProfileKeys::EnableExternalTriggerCruise,
      settings.external_trigger_cruise_enabled);

  GetEnum(ProfileKeys::AverEffTime, settings.average_eff_time);

  Get(ProfileKeys::SetSystemTimeFromGPS, settings.set_system_time_from_gps);

  // NOTE: Until 6.2.4 utc_offset was stored as a positive int in the
  // settings file (with negative offsets stored as "utc_offset + 24 * 3600").
  // Later versions will create a new signed settings key.
  if (!Get(ProfileKeys::UTCOffsetSigned, settings.utc_offset)) {
    if (Get(ProfileKeys::UTCOffset, settings.utc_offset)) {
      if (settings.utc_offset > 12 * 3600)
        settings.utc_offset = (settings.utc_offset % (24 * 3600)) - 24 * 3600;
    }
  }

  if (settings.utc_offset > 13 * 3600 || settings.utc_offset < -13 * 3600)
    settings.utc_offset = 0;

  Load(settings.task);
  Load(settings.logger);

#ifdef HAVE_TRACKING
  Load(settings.tracking);
#endif
}
