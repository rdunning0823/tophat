/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#ifndef XCSOAR_KOBO_SYSTEM_HPP
#define XCSOAR_KOBO_SYSTEM_HPP

#include "Compiler.h"
#include "DisplaySettings.hpp"

bool
KoboReboot();

bool
KoboPowerOff();

gcc_pure
bool
IsKoboWifiOn();

bool
KoboWifiOn();

bool
KoboWifiOff();

void
KoboExecNickel();

void
KoboRunXCSoar(const char *mode);

void
KoboRunTelnetd();

/**
 * returns true if the current kernel supports USB Host mode
 */
bool
IsKoboUsbHostKernel();

/**
 * Writes uname -a info and dd if=/dev/mmcblk0 bs=8 count=1 skip=64
 * results to /tmp/TophatSystemInfo.txt
 */
void WriteSystemInfo();

#ifdef KOBO
/**
 * Writes the hardware screen orientation to a text file
 * for use during the next startup of Top Hat
 */
void WriteKoboScreenOrientation(const char * rotate);

/**
 * Reads rotation byte from file saved on disk
 * @return Orientation based on hardware settings
 */
DisplaySettings::Orientation ReadKoboLastScreenOrientation();
#endif
/**
 * returns true if a USB Storage device is currently mounted
 * at /media/usb_storage
 */
bool IsUSBStorageConnected();

/**
 * Copies the XCSoarData folder on the SDCard to the device
 */
void UploadSDCardToDevice();

/**
 * copies the XCSoarData folder on the device to the SD Card
 */
void CopyTopHatDataToSDCard();

/**
 * does KoboRoot.tgz exist in the root of the USB Storage device
 */
bool IsUSBStorageKoboRootInRoot();

/**
 * Copies KoboRoot.tgz from root of USB storage to .kobo folder on device
 * And deletes KoboRoot.tgz from the USB storage
 * @return true if KoboRoot.tgz was found on the USB storage
 */
bool InstallKoboRootTgz();
#endif
