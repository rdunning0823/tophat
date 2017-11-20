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
 * InfoBoxCustomTitle.hpp
 *
 *  Created on: Nov 3, 2017
 *      Author: Ludovic Launer
 */

#ifndef SRC_INFOBOXES_INFOBOXTITLELOCALE_HPP_
#define SRC_INFOBOXES_INFOBOXTITLELOCALE_HPP_

#include "Profile/Map.hpp"

namespace InfoBoxTitleLocale
{
static ProfileMap map_title_locale;

// Special cases where the caption already has a "translation" in en.po
static std::map<std::string, std::string> map_title_locale_existing_en_po = {
                                                                             {"AAT Dtgt", "Task D rem"},
                                                                             {"AAT dT","Task dT"},
                                                                             {"H AGL","Alt AGL"},
                                                                             {"Start Height","Start height"},
                                                                            };

bool
Initialise();

bool
LoadFile();

void
CreateDefaultFile(TCHAR* filePath);

const TCHAR*
GetLocale(const TCHAR *caption);

// This file containing this function is generated automatically by generate.mk
// It should be in /output/include/TextResources.c
extern "C"
{
const char*
find_embedded_file(const char *name, size_t *size);
}

};

#endif /* SRC_INFOBOXES_INFOBOXTITLELOCALE_HPP_ */
