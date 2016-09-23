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

#ifndef XCSOAR_INFOBOX_CONTENT_ALTITUDE_HPP
#define XCSOAR_INFOBOX_CONTENT_ALTITUDE_HPP

#include "InfoBoxes/Content/Base.hpp"

extern const InfoBoxPanel barometric_altitude_infobox_panels[];
extern const InfoBoxPanel gps_altitude_infobox_panels[];
extern const InfoBoxPanel agl_altitude_infobox_panels[];

void
UpdateInfoBoxAltitudeNav(InfoBoxData &data);

class InfoBoxContentAltitudeGPS : public InfoBoxContent
{
public:
  virtual const InfoBoxPanel *GetDialogContent() override;
  virtual void Update(InfoBoxData &data) override;
  virtual bool HandleKey(const InfoBoxKeyCodes keycode) override;
};

class InfoBoxContentAltitudeAGL : public InfoBoxContent
{
public:
  virtual const InfoBoxPanel *GetDialogContent() override;
  virtual void Update(InfoBoxData &data) override;
};

void
UpdateInfoBoxAltitudeBaro(InfoBoxData &data);

void
UpdateInfoBoxAltitudeQFE(InfoBoxData &data);

void
UpdateInfoBoxAltitudeFlightLevel(InfoBoxData &data);

#endif
