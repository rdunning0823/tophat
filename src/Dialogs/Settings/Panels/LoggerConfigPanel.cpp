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

#include "LoggerConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Form/DataField/Enum.hpp"
#include "Logger/NMEALogger.hpp"
#include "UtilsSettings.hpp"

enum ControlIndex {
  PilotName,
  LoggerTimeStepCruise,
  LoggerTimeStepCircling,
  EnableNMEALogger,
  EnableFlightLogger,
  LoggerID,
};

class LoggerConfigPanel final : public RowFormWidget {
public:
  LoggerConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
};

void
LoggerConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const LoggerSettings &logger = settings_computer.logger;

  RowFormWidget::Prepare(parent, rc);
  AddText(_("Pilot name"), nullptr, logger.pilot_name);

  AddTime(_("Time step cruise"),
          _("This is the time interval between logged points when not circling."),
          1, 30, 1, logger.time_step_cruise);
  SetExpertRow(LoggerTimeStepCruise);

  AddTime(_("Time step circling"),
          _("This is the time interval between logged points when circling."),
          1, 30, 1, logger.time_step_circling);
  SetExpertRow(LoggerTimeStepCircling);

  AddBoolean(_("NMEA logger"),
             _("Enable the NMEA logger on startup? If this option is disabled, "
                 "the NMEA logger can still be started manually."),
             logger.enable_nmea_logger);
  SetExpertRow(EnableNMEALogger);

  AddBoolean(_("Log book"), _("Logs each start and landing."),
             logger.enable_flight_logger);
  SetExpertRow(EnableFlightLogger);

  AddText(_("Logger ID"), nullptr, logger.logger_id);
  SetExpertRow(LoggerID);
}

bool
LoggerConfigPanel::Save(bool &changed)
{
  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  LoggerSettings &logger = settings_computer.logger;

  changed |= SaveValue(PilotName, ProfileKeys::PilotName,
                       logger.pilot_name);

  changed |= SaveValue(LoggerTimeStepCruise, ProfileKeys::LoggerTimeStepCruise,
                       logger.time_step_cruise);

  changed |= SaveValue(LoggerTimeStepCircling, ProfileKeys::LoggerTimeStepCircling,
                       logger.time_step_circling);

  changed |= SaveValue(EnableNMEALogger, ProfileKeys::EnableNMEALogger,
                       logger.enable_nmea_logger);

  if (logger.enable_nmea_logger)
    NMEALogger::enabled = true;

  if (SaveValue(EnableFlightLogger, ProfileKeys::EnableFlightLogger,
                logger.enable_flight_logger)) {
    changed = true;

    /* currently, the GlueFlightLogger instance is created on startup
       only, which means XCSoar needs to be restarted to apply the new
       setting */
    require_restart = true;
  }

  changed |= SaveValue(LoggerID, ProfileKeys::LoggerID, logger.logger_id);

  return true;
}

Widget *
CreateLoggerConfigPanel()
{
  return new LoggerConfigPanel();
}
