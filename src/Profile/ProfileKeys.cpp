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

#include "Profile/ProfileKeys.hpp"

namespace ProfileKeys {

const char Password[] = "Password";
const char AirspaceWarning[] = "AirspaceWarn";
const char AirspaceWarningDialog[] = "AirspaceWarnDialog";
const char AirspaceBlackOutline[] = "AirspaceBlackOutline";
const char AirspaceTransparency[] = "AirspaceTransparency";
const char AirspaceFillMode[] = "AirspaceFillMode";
const char AirspaceLabelSelection[] = "AirspaceLabelSelection";
const char AltMargin[] = "AltMargin";
const char AltMode[] = "AltitudeMode";
const char AltitudeUnitsValue[] = "AltitudeUnit";
const char TemperatureUnitsValue[] = "TemperatureUnit";
const char CirclingMinTurnRate[] = "CirclingMinTurnRate";
const char CirclingCruiseClimbSwitch[] = "CirclingCruiseClimbSwitch";
const char CirclingClimbCruiseSwitch[] = "CirclingClimbCruiseSwitch";
const char CircleZoom[] = "CircleZoom";
const char MaxAutoZoomDistance[] = "MaxAutoZoomDistance";
const char ClipAlt[] = "ClipAlt";
const char DisplayText[] = "DisplayText";
const char WaypointArrivalHeightDisplay[] = "WaypointArrivalHeightDisplay";
const char WaypointArrivalHeightUnitDisplay[] = "WaypointArrivalHeightUnitDisplay";
const char WaypointLabelSelection[] = "WayPointLabelSelection";
const char WaypointLabelStyle[] = "WayPointLabelStyle";
const char WeatherStations[] = "WeatherStations";
const char DisplayUpValue[] = "DisplayUp";
const char OrientationCruise[] = "OrientationCruise";
const char OrientationCircling[] = "OrientationCircling";
const char MapShiftBias[] = "MapShiftBias";
const char DistanceUnitsValue[] = "DistanceUnit";
const char DrawTerrain[] = "DrawTerrain";
const char SlopeShading[] = "SlopeShading";
const char SlopeShadingType[] = "SlopeShadingType";
const char TerrainContours[] = "TerrainContours";
const char DrawTopography[] = "DrawTopology";
const char FinalGlideTerrain[] = "FinalGlideTerrain";
const char UserWindSource[] = "WindSourceFromUser";
const char ATCReference[] = "ATCReference";
const char HomeWaypoint[] = "HomeWaypoint";
const char HomeWaypointName[] = "HomeWaypointName";
const char HomeLocation[] = "HomeLocation";
const char HomeElevation[] = "HomeElevation";
const char HomeElevationAvailable[] = "HomeElevationAvailable";
const char LiftUnitsValue[] = "LiftUnit";
const char PressureUnitsValue[] = "Pressure";
const char WingLoadingUnitValue[] = "WingLoadingUnits";
const char MassUnitValue[] = "MassUnits";
const char VolumeUnitsValue[] = "VolumeUnits";
const char LatLonUnits[] = "LatLonUnits";
const char PolarID[] = "Polar";
const char Polar[] = "PolarInformation";
const char PolarName[] = "PolarName";
const char PolarDegradation[] = "PolarDegradation";
const char AutoBugs[] = "AutoBugs";
const char SafetyAltitudeArrival[] = "SafetyAltitudeArrival";
const char SafetyAltitudeArrivalGRMode[] = "SafetyAltitudeArrivalGRMode";
const char SafetyAltitudeTerrain[] = "SafetyAltitudeTerrain";
const char SafteySpeed[] = "SafteySpeed";
const char DryMass[] = "DryMass";
const char SnailTrail[] = "SnailTrail";
const char TrailDrift[] = "TrailDrift";
const char DetourCostMarker[] = "DetourCostMarker";
const char DisplayTrackBearing[] = "DisplayTrackBearing";
const char SpeedUnitsValue[] = "SpeedUnit";
const char TaskSpeedUnitsValue[] = "TaskSpeedUnit";
const char WarningTime[] = "WarnTime";
const char RepetitiveSound[] = "RepetitiveSound";
const char AcknowledgementTime[] = "AcknowledgementTime";
const char AirfieldFile[] = "AirfieldFile"; // pL
const char AirspaceFile[] = "AirspaceFile"; // pL
const char AdditionalAirspaceFile[] = "AdditionalAirspaceFile"; // pL
const char PolarFile[] = "PolarFile"; // pL
const char WaypointFile[] = "WPFile"; // pL
const char AdditionalWaypointFile[] = "AdditionalWPFile"; // pL
const char WatchedWaypointFile[] = "WatchedWPFile"; // pL
const char LanguageFile[] = "LanguageFile"; // pL
const char InputFile[] = "InputFile"; // pL
const char PilotName[] = "PilotName";
const char TophatArguments[] = "TophatArguments";
const char AircraftType[] = "AircraftType";
const char AircraftReg[] = "AircraftReg";
const char CompetitionId[] = "AircraftRego";
const char LoggerID[] = "LoggerID";
const char LoggerShort[] = "LoggerShortName";
const char SoundVolume[] = "SoundVolume";
const char SoundDeadband[] = "SoundDeadband";
const char SoundAudioVario[] = "AudioVario2";
const char SoundTask[] = "SoundTask";
const char SoundModes[] = "SoundModes";
const char NettoSpeed[] = "NettoSpeed";
const char AccelerometerZero[] = "AccelerometerZero";

const char AutoBlank[] = "AutoBlank";
const char AverEffTime[] = "AverEffTime";
const char VarioGauge[] = "VarioGauge";

const char AppIndLandable[] = "AppIndLandable";
const char AppUseSWLandablesRendering[] = "AppUseSWLandablesRendering";
const char AppLandableRenderingScale[] = "AppLandableRenderingScale";
const char AppScaleRunwayLength[] = "AppScaleRunwayLength";
const char AppInverseInfoBox[] = "AppInverseInfoBox";
const char AppGaugeVarioSpeedToFly[] = "AppGaugeVarioSpeedToFly";
const char AppGaugeVarioAvgText[] = "AppGaugeVarioAvgText";
const char AppGaugeVarioMc[] = "AppGaugeVarioMc";
const char AppGaugeVarioBugs[] = "AppGaugeVarioBugs";
const char AppGaugeVarioBallast[] = "AppGaugeVarioBallast";
const char AppGaugeVarioGross[] = "AppGaugeVarioGross";
const char AppStatusMessageAlignment[] = "AppStatusMessageAlignment";
const char AppTextInputStyle[] = "AppTextInputStyle";
const char HapticFeedback[] = "HapticFeedback";
const char KoboMiniSunblind[] = "KoboMiniSunblind";
const char AppDialogTabStyle[] = "AppDialogTabStyle";
const char AppDialogStyle[] = "AppDialogStyle";
const char AppInfoBoxColors[] = "AppInfoBoxColors";
const char TeamcodeRefWaypoint[] = "TeamcodeRefWaypoint";
const char AppInfoBoxBorder[] = "AppInfoBoxBorder";

const char AppInfoBoxModel[] = "AppInfoBoxModel"; // VENTA-ADDON MODEL CONFIG

const char AppAveNeedle[] = "AppAveNeedle";

const char AutoAdvance[] = "AutoAdvance";
const char UTCOffset[] = "UTCOffset";
const char UTCOffsetSigned[] = "UTCOffsetSigned";
const char BlockSTF[] = "BlockSpeedToFly";
const char AutoZoom[] = "AutoZoom";
const char MenuTimeout[] = "MenuTimeout";
const char TerrainContrast[] = "TerrainContrast";
const char TerrainBrightness[] = "TerrainBrightness";
const char TerrainRamp[] = "TerrainRamp";
const char EnableFLARMMap[] = "EnableFLARMDisplay";
const char EnableFLARMGauge[] = "EnableFLARMGauge";
const char AutoCloseFlarmDialog[] = "AutoCloseFlarmDialog";
const char EnableTAGauge[] = "EnableTAGauge";
const char EnableThermalProfile[] = "EnableThermalProfile";
const char GliderScreenPosition[] = "GliderScreenPosition";
const char SetSystemTimeFromGPS[] = "SetSystemTimeFromGPS";

const char VoiceClimbRate[] = "VoiceClimbRate";
const char VoiceTerrain[] = "VoiceTerrain";
const char VoiceWaypointDistance[] = "VoiceWaypointDistance";
const char VoiceTaskAltitudeDifference[] = "VoiceTaskAltitudeDifference";
const char VoiceMacCready[] = "VoiceMacCready";
const char VoiceNewWaypoint[] = "VoiceNewWaypoint";
const char VoiceInSector[] = "VoiceInSector";
const char VoiceAirspace[] = "VoiceAirspace";

const char FinishMinHeight[] = "FinishMinHeight";
const char FinishHeightRef[] = "FinishHeightRef";
const char StartMaxHeight[] = "StartMaxHeight";
const char StartMaxSpeed[] = "StartMaxSpeed";
const char StartMaxHeightMargin[] = "StartMaxHeightMargin";
const char StartMaxSpeedMargin[] = "StartMaxSpeedMargin";
const char StartHeightRef[] = "StartHeightRef";
const char StartType[] = "StartType";
const char StartRadius[] = "StartRadius";
const char TurnpointType[] = "TurnpointType";
const char TurnpointRadius[] = "TurnpointRadius";
const char FinishType[] = "FinishType";
const char FinishRadius[] = "FinishRadius";
const char TaskType[] = "TaskType";
const char AATMinTime[] = "AATMinTime";
const char AATTimeMargin[] = "AATTimeMargin";
const char TaskPlanningSpeedMode[] = "TaskPlanningSpeedMode";
const char TaskPlanningSpeedOverride[] = "TaskPlanningSpeedOverride";
const char ContestNationality[] = "ContestNationality";

const char EnableNavBaroAltitude[] = "EnableNavBaroAltitude";

const char LoggerTimeStepCruise[] = "LoggerTimeStepCruise";
const char LoggerTimeStepCircling[] = "LoggerTimeStepCircling";

const char SafetyMacCready[] = "SafetyMacCready";
const char AbortTaskMode[] = "AbortTaskMode";
const char AutoMcMode[] = "AutoMcMode";
const char AutoMc[] = "AutoMc";
const char EnableExternalTriggerCruise[] = "EnableExternalTriggerCruise";
const char OLCRules[] = "OLCRules";
const char PredictContest[] = "PredictContest";
const char Handicap[] = "Handicap";
const char SnailWidthScale[] = "SnailWidthScale";
const char SnailType[] = "SnailType";
const char UserLevel[] = "UserLevel";
const char RiskGamma[] = "RiskGamma";
const char PredictWindDrift[] = "PredictWindDrift";
const char WindArrowStyle[] = "WindArrowStyle";
const char WindArrowLocation[] = "WindArrowLocation";
const char EnableFinalGlideBarMC0[] = "EnableFinalGlideBarMC0";
const char FinalGlideBarDisplayMode[] = "FinalGlideBarDisplayMode";
const char EnableVarioBar[] = "EnableVarioBar";
const char ShowFAITriangleAreas[] = "ShowFAITriangleAreas";
const char FAITriangleThreshold[] = "FAITriangleThreshold";
const char AutoLogger[] = "AutoLogger";
const char DisableAutoLogger[] = "DisableAutoLogger";
const char EnableFlightLogger[] = "EnableFlightLogger";
const char EnableNMEALogger[] = "EnableNMEALogger";
const char MapFile[] = "MapFile"; // pL
const char BallastSecsToEmpty[] = "BallastSecsToEmpty";
const char StartupTipDeclineVersion[] = "StartupTipDeclineVersion";
const char ShowWaypointListWarning[] = "ShowWaypointListWarning";
const char StartupTipId[] = "StartupTipId";

const char FontNavBarWaypointName[] = "FontNavBarWaypointName";
const char FontNavBarDistance[] = "FontNavBarDistance";
const char FontMapWaypointName[] = "FontMapWaypointName";
const char FontMapPlaceName[] = "FontMapPlaceName";
const char FontInfoBoxValue[] = "FontInfoBoxValue";
const char FontInfoBoxTitle[] = "FontInfoBoxTitle";
const char FontInfoBoxComment[] = "FontInfoBoxComment";
const char FontOverlayButton[] = "FontOverlayButton";
const char FontDialog[] = "FontDialog";

const char UseFinalGlideDisplayMode[] = "UseFinalGlideDisplayMode";
const char InfoBoxGeometry[] = "InfoBoxGeometry";
const char ShowAlternateAltitudeUnits[] = "ShowAlternateAltitudeUnits";

const char FlarmSideData[] = "FlarmRadarSideData";
const char FlarmAutoZoom[] = "FlarmRadarAutoZoom";
const char FlarmNorthUp[] = "FlarmRadarNorthUp";

const char IgnoreNMEAChecksum[] = "IgnoreNMEAChecksum";
const char MapOrientation[] = "DisplayOrientation";

const char ClimbMapScale[] = "ClimbMapScale";
const char CruiseMapScale[] = "CruiseMapScale";

const char RoutePlannerMode[] = "RoutePlannerMode";
const char RoutePlannerAllowClimb[] = "RoutePlannerAllowClimb";
const char RoutePlannerUseCeiling[] = "RoutePlannerUseCeiling";
const char TurningReach[] = "TurningReach";
const char ReachPolarMode[] = "ReachPolarMode";

const char AircraftSymbol[] = "AircraftSymbol";

const char FlarmLocation[] = "FlarmLocation";

const char TrackingInterval[] = "TrackingInterval";
const char TrackingVehicleType[] = "TrackingVehicleType";
const char TrackingVehicleName[] = "TrackingVehicleName";
const char SkyLinesTrackingEnabled[] = "SkyLinesTrackingEnabled";
const char SkyLinesRoaming[] = "SkyLinesRoaming";
const char SkyLinesTrackingInterval[] = "SkyLinesTrackingInterval";
const char SkyLinesTrafficEnabled[] = "SkyLinesTrafficEnabled";
const char SkyLinesNearTrafficEnabled[] = "SkyLinesNearTrafficEnabled";
const char SkyLinesTrackingKey[] = "SkyLinesTrackingKey";
const char LiveTrack24Enabled[] = "LiveTrack24Enabled";
const char LiveTrack24Server[] = "LiveTrack24Server";
const char LiveTrack24Username[] = "LiveTrack24Username";
const char LiveTrack24Password[] = "LiveTrack24Password";

const char EnableLocationMapItem[] = "EnableLocationMapItem";
const char EnableArrivalAltitudeMapItem[] = "EnableArrivalAltitudeMapItem";

const char VarioMinFrequency[] = "VarioMinFrequency";
const char VarioZeroFrequency[] = "VarioZeroFrequency";
const char VarioMaxFrequency[] = "VarioMaxFrequency";
const char VarioMinPeriod[] = "VarioMinPeriod";
const char VarioMaxPeriod[] = "VarioMaxPeriod";
const char VarioDeadBandEnabled[] = "VarioDeadBandEnabled";
const char VarioDeadBandMin[] = "VarioDeadBandMin";
const char VarioDeadBandMax[] = "VarioDeadBandMax";

const char PagesDistinctZoom[] = "PagesDistinctZoom";

const char FilePickAndDownloadAreaFilter[] = "FilePickAndDownloadAreaFilter";
const char FilePickAndDownloadSubAreaFilter[] = "FilePickAndDownloadSubAreaFilter";
const char ScreensButtonLocation[] = "ScreensButtonLocation";
const char WaypointSortDirection[] = "WaypointSortDirection";
const char WaveAssistant[] = "WaveAssistant";

const char SystemSoundVolume [] = "SystemSoundVolume";

const char NavBarDisplayGR [] = "NavBarDisplayGR";
const char NavBarDisplayTpIndex [] = "NavBarDisplayTpIndex";
const char NavBarNavigateToAATTarget [] = "NavBarNavigateToAATTarget";
}
