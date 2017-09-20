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

#ifndef XCSOAR_INFOBOX_CONTENT_TASK_HPP
#define XCSOAR_INFOBOX_CONTENT_TASK_HPP

#include "InfoBoxes/Content/Base.hpp"

extern const InfoBoxPanel next_waypoint_infobox_panels[];

extern const InfoBoxPanel task_computer_setup_infobox_panels[];

class InfoBoxTaskComputerSetup: public InfoBoxContentNonTabbed
{
public:
  virtual const InfoBoxPanel *GetDialogContent() override;
};

class InfoBoxTaskSpeed: public InfoBoxTaskComputerSetup
{
public:
  virtual void Update(InfoBoxData &data) override;
};

class InfoBoxTaskSpeedHour: public InfoBoxTaskComputerSetup
{
public:
  virtual void Update(InfoBoxData &data) override;
};

class InfoBoxTaskSpeedInstant: public InfoBoxTaskComputerSetup
{
public:
  virtual void Update(InfoBoxData &data) override;
};

class InfoBoxTaskSpeedAchieved: public InfoBoxTaskComputerSetup
{
public:
  virtual void Update(InfoBoxData &data) override;
};


class InfoBoxContentBearingDiff: public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data) override;
  virtual const InfoBoxPanel *GetDialogContent() override;
};

void
UpdateInfoBoxBearing(InfoBoxData &data);

void
UpdateInfoBoxRadial(InfoBoxData &data);

class InfoBoxContentNextWaypoint : public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data) override;
  virtual bool HandleKey(const InfoBoxKeyCodes keycode) override;
  virtual const InfoBoxPanel *GetDialogContent() override;
};

void
UpdateInfoBoxNextDistance(InfoBoxData &data);

void
UpdateInfoBoxNextDistanceNominal(InfoBoxData &data);

void
UpdateInfoBoxNextETE(InfoBoxData &data);

void
UpdateInfoBoxNextETA(InfoBoxData &data);

void
UpdateInfoBoxNextAltitudeDiff(InfoBoxData &data);

void
UpdateInfoBoxNextMC0AltitudeDiff(InfoBoxData &data);

void
UpdateInfoBoxNextAltitudeRequire(InfoBoxData &data);

void
UpdateInfoBoxNextAltitudeArrival(InfoBoxData &data);

void
UpdateInfoBoxNextGR(InfoBoxData &data);

void
UpdateInfoBoxFinalDistance(InfoBoxData &data);

void
UpdateInfoBoxFinalETE(InfoBoxData &data);

void
UpdateInfoBoxFinalETA(InfoBoxData &data);

void
UpdateInfoBoxFinalAltitudeDiff(InfoBoxData &data);

void
UpdateInfoBoxFinalMC0AltitudeDiff(InfoBoxData &data);

void
UpdateInfoBoxFinalAltitudeRequire(InfoBoxData &data);

void
UpdateInfoBoxFinalGR(InfoBoxData &data);

void
UpdateInfoBoxTaskSpeed(InfoBoxData &data);

void
UpdateInfoBoxTaskSpeedAchieved(InfoBoxData &data);

extern const InfoBoxPanel home_infobox_panels[];

class InfoBoxContentHome: public InfoBoxContentNonTabbed
{
public:
  virtual const InfoBoxPanel *GetDialogContent() override;
};

class InfoBoxContentHomeDistance : public InfoBoxContentHome
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentHomeGR : public InfoBoxContentHome
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentHomeAltitudeDiff : public InfoBoxContentHome
{
public:
  virtual void Update(InfoBoxData &data);
};

void
UpdateInfoBoxTaskSpeedInstant(InfoBoxData &data);

void
UpdateInfoBoxTaskSpeedHour(InfoBoxData &data);

void
UpdateInfoBoxTaskAATime(InfoBoxData &data);

void
UpdateInfoBoxTaskAATimeDelta(InfoBoxData &data);

void
UpdateInfoBoxTaskAADistance(InfoBoxData &data);

void
UpdateInfoBoxTaskAADistanceMax(InfoBoxData &data);

void
UpdateInfoBoxTaskAADistanceMin(InfoBoxData &data);

void
UpdateInfoBoxTaskAASpeed(InfoBoxData &data);

void
UpdateInfoBoxTaskAASpeedMax(InfoBoxData &data);

void
UpdateInfoBoxTaskAASpeedMin(InfoBoxData &data);

void
UpdateInfoBoxTaskTimeUnderMaxHeight(InfoBoxData &data);

void
UpdateInfoBoxNextETEVMG(InfoBoxData &data);

void
UpdateInfoBoxFinalETEVMG(InfoBoxData &data);

void
UpdateInfoBoxCruiseEfficiency(InfoBoxData &data);

void
UpdateInfoBoxStartOpen(InfoBoxData &data);

void
UpdateInfoBoxStartOpenArrival(InfoBoxData &data);

class InfoBoxContentNextArrow: public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data) override;
  virtual void OnCustomPaint(Canvas &canvas, const PixelRect &rc) override;
};

#endif
