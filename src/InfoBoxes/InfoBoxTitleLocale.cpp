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
/*
 * InfoBoxCustomTitle.cpp
 * Allows for customisation of InfoBox titles.
 * This is done by reading from a key=value file located in the current profile directory.
 * @see InfoBoxManager::DisplayInfoBox()
 * Titles (key) having an entry in the file will have their title replaced by the corresponding value.
 * If no entry can be found, the default title (using gettext) is used.
 *
 *  Created on: Nov 3, 2017
 *      Author: Ludovic Launer
 */

#include "InfoBoxTitleLocale.hpp"

#include "LogFile.hpp"
#include "Profile/File.hpp"
#include "LocalPath.hpp"
#include "Util/Macros.hpp"

#include <windef.h> /* for MAX_PATH */

#define INFOBOX_LOCALE_FILE "infoBoxTitle.locale"

/**
 * Initialise
 * Init and Load local title from file.
 * This needs to be called first to populate the map.
 * @return true if the file was found. false otherwise
 */
bool
InfoBoxTitleLocale::Initialise()
{
  LogFormat("Loading InfoBox Titles locale...");
  bool fileRead = LoadFile();
  return fileRead;
}

/**
 * LoadFile
 * Try to read (key="value") custom titles from a file.
 * @return true if the file was found. false otherwise
 */
bool
InfoBoxTitleLocale::LoadFile()
{
  static TCHAR path[MAX_PATH];
  LocalPath(path, _T(INFOBOX_LOCALE_FILE));

  if (!Profile::LoadFile(mapTitleLocale, path))
    LogFormat(_T("%s : File not found, will use default values"), path);
  else {
    LogFormat(_T("Loaded %d InfoBox Titles locale from: %s"),
              (int)mapTitleLocale.size(), path);
  }

  return true;
}

/**
 * Look up a string value in locale InfoBoxTile file
 *
 * @param key name of the value
 * @return the value or null if it cannot be found
 */
const char*
InfoBoxTitleLocale::GetLocale(const char *key)
{
  return mapTitleLocale.Get(key, nullptr);
}

