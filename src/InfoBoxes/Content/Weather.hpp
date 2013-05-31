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

#ifndef XCSOAR_INFOBOX_CONTENT_WEATHER_HPP
#define XCSOAR_INFOBOX_CONTENT_WEATHER_HPP

#include "InfoBoxes/Content/Base.hpp"

class InfoBoxContentHumidity : public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentTemperature : public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentTemperatureForecast : public InfoBoxContent
{
public:
  virtual void Update(InfoBoxData &data);
  virtual bool HandleKey(const InfoBoxKeyCodes keycode);
};

class InfoBoxContentWind : public InfoBoxContent
{
public:
  virtual const DialogContent *GetDialogContent();

  static const DialogContent dlgContent;
};

class InfoBoxContentWindSpeed : public InfoBoxContentWind
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentWindBearing : public InfoBoxContentWind
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentHeadWind: public InfoBoxContentWind
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentHeadWindSimplified: public InfoBoxContentWind
{
public:
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentWindArrow: public InfoBoxContentWind
{
public:
  virtual void Update(InfoBoxData &data);
  virtual void OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas);
};

#endif
