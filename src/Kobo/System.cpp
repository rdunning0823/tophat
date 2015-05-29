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

#include "System.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "OS/Process.hpp"
#include "OS/Sleep.h"
#include "Util/StaticString.hpp"
#include "Net/IpAddress.hpp"
#include "DisplaySettings.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef KOBO

#include <sys/mount.h>

static bool
InsMod(const char *path)
{
  return Run("/sbin/insmod", path);
}

static bool
RmMod(const char *name)
{
  return Run("/sbin/rmmod", name);
}

/**
 * Determine the location of the current program, and build a path to
 * another program in the same directory.
 */
static bool
SiblingPath(const char *name, char *buffer, size_t size)
{
  if (readlink("/proc/self/exe", buffer, size) <= 0)
    return false;

  ReplaceBaseName(buffer, name);
  return true;
}

#endif

/**
 * Force unmount of USB Storage.  Do this before poweroff
 */
void UnmountKoboUSBStorage();

bool
KoboReboot()
{
#ifdef KOBO
  UnmountKoboUSBStorage();
  return Run("/sbin/reboot");
#else
  return false;
#endif
}

bool
KoboPowerOff()
{
#ifdef KOBO
  UnmountKoboUSBStorage();
  char buffer[256];
  if (SiblingPath("PowerOff", buffer, sizeof(buffer)))
    execl(buffer, buffer, nullptr);

  /* fall back */
  return Run("/sbin/poweroff");
#else
  return false;
#endif
}

bool
IsKoboWifiOn()
{
#ifdef KOBO
  return Directory::Exists("/sys/class/net/eth0");
#else
  return false;
#endif
}

bool
KoboWifiOn()
{
#ifdef KOBO
  InsMod("/drivers/ntx508/wifi/sdio_wifi_pwr.ko");
  InsMod("/drivers/ntx508/wifi/dhd.ko");

  Sleep(2000);

  Run("/sbin/ifconfig", "eth0", "up");
  Run("/bin/wlarm_le", "-i", "eth0", "up");
  Run("/bin/wpa_supplicant", "-i", "eth0",
      "-c", "/etc/wpa_supplicant/wpa_supplicant.conf",
      "-C", "/var/run/wpa_supplicant", "-B");

  Sleep(2000);

  Start("/sbin/udhcpc", "-S", "-i", "eth0",
        "-s", "/etc/udhcpc.d/default.script",
        "-t15", "-T10", "-A3", "-f", "-q");

  return true;
#else
  return false;
#endif
}

bool
KoboWifiOff()
{
#ifdef KOBO
  Run("/usr/bin/killall", "wpa_supplicant", "udhcpc");
  Run("/bin/wlarm_le", "-i", "eth0", "down");
  Run("/sbin/ifconfig", "eth0", "down");

  RmMod("dhd");
  RmMod("sdio_wifi_pwr");

  return true;
#else
  return false;
#endif
}

void
KoboExecNickel()
{
#ifdef KOBO
  /* our "rcS" will call the original Kobo "rcS" if start_nickel
     exists */
  mkdir("/mnt/onboard/XCSoarData", 0777);
  mkdir("/mnt/onboard/XCSoarData/kobo", 0777);
  File::CreateExclusive("/mnt/onboard/XCSoarData/kobo/start_nickel");

  /* unfortunately, a bug in the Kobo applications forces us to reboot
     the Kobo at this point */
  KoboReboot();
#endif
}

void
KoboRunXCSoar(const char *mode)
{
#ifdef KOBO
  char buffer[256];
  const char *cmd = buffer;

  if (!SiblingPath("tophat", buffer, sizeof(buffer)))
    cmd = "/mnt/onboard/XCSoar/tophat";

  Run(cmd, mode);
#endif
}

void
KoboRunTelnetd()
{
#ifdef KOBO
  /* telnetd requires /dev/pts - mount it (if it isn't already) */
  if (mkdir("/dev/pts", 0777) == 0)
    mount("none", "/dev/pts", "devpts", MS_RELATIME, NULL);

  Run("/usr/sbin/telnetd", "-l", "/bin/sh");
#endif
}

bool IsKoboUsbHostKernel()
{
#ifdef KOBO
  static char cmd_find_ip[] = "/bin/dd if=/dev/mmcblk0 bs=1M bs=512 skip=2048 count=1 2>/dev/null | strings | grep";
  static char search_string[] = "USB-hot-plug";

  const char *local_path = "/tmp/usb_host.txt";
  TCHAR command[256];
  _stprintf(command, _T("%s %s > %s"), cmd_find_ip, search_string, local_path);
  system(command);
  StaticString <256> buffer;
  File::ReadString(local_path, buffer.buffer(), 256);
  return buffer.Contains(search_string);
#else
  return false;
#endif
}

void
WriteSystemInfo()
{
#ifdef KOBO
  static char cmd_kobo_kernel_info[] = "dd if=/dev/mmcblk0 bs=8 count=1 skip=64";
  static char cmd_uname_all[] = "uname -a";

  const char *local_path = "/TophatSystemInfo.txt";
  TCHAR command[256];
  _stprintf(command, _T("%s > %s"), cmd_kobo_kernel_info, local_path);
  system(command);
  _stprintf(command, _T(" %s >> %s"), cmd_uname_all, local_path);
  system(command);
#endif
}

#ifdef KOBO
static const char *kobo_orientation_file =
    "/mnt/onboard/XCSoarData/OrientationKoboLast.tx0";

void
WriteKoboScreenOrientation(const char * rotate)
{
  File::CreateExclusive(kobo_orientation_file);
  File::WriteExisting(kobo_orientation_file, rotate);

  static const char *kobo_orientation_file_old =
      "/mnt/onboard/XCSoarData/OrientationKoboLast.txt";
  if (File::Exists(kobo_orientation_file_old))
    File::Delete(kobo_orientation_file_old);
}

DisplaySettings::Orientation
ReadKoboLastScreenOrientation()
{
  DisplaySettings::Orientation orientation =
      DisplaySettings::Orientation::DEFAULT;
  char line[4];

  if (File::ReadString(kobo_orientation_file, line, sizeof(line))) {
    switch(line[0]) {
    case '3':
      orientation = DisplaySettings::Orientation::PORTRAIT;
      break;
    case '1':
      orientation = DisplaySettings::Orientation::REVERSE_PORTRAIT;
      break;
    case '0':
      orientation = DisplaySettings::Orientation::LANDSCAPE;
      break;
    case '2':
      orientation = DisplaySettings::Orientation::REVERSE_LANDSCAPE;
      break;
    }
  }
  return orientation;
}
#endif

bool
IsUSBStorageConnected()
{
#ifdef KOBO
  return Directory::Exists(_T("/media/usb_storage"));
#else
  return false;
#endif
}

void
UploadTasksToDevice()
{
#ifdef KOBO
  Directory::Create(_T("/mnt/onboard/XCSoarData/tasks"));
  system(_T("cp -r /media/usb_storage/XCSoarData/tasks/*.* /mnt/onboard/XCSoarData/tasks"));
#endif
}

void
CopyFlightsToSDCard()
{
#ifdef KOBO
  Directory::Create(_T("/media/usb_storage/XCSoarData"));
  Directory::Create(_T("/media/usb_storage/XCSoarData/logs"));
  system(_T("cp -r /mnt/onboard/XCSoarData/logs/*.igc /media/usb_storage/XCSoarData/logs"));
#endif
}

void
UploadSDCardToDevice()
{
#ifdef KOBO
  system(_T("cp -r /media/usb_storage/XCSoarData /mnt/onboard"));
#endif
}

void
CopyTopHatDataToSDCard()
{
#ifdef KOBO
  system(_T("cp -r /mnt/onboard/XCSoarData /media/usb_storage"));
  system(_T("sync"));
#endif
}

void
UnmountKoboUSBStorage()
{
#ifdef KOBO
  if (!IsUSBStorageConnected())
    return;
  system(_T("umount /media/usb_storage"));
  system(_T("rm -rf /media/usb_storage"));
  system(_T("sync"));
#endif
}

bool
IsUSBStorageKoboRootInRoot()
{
#ifdef KOBO
  return IsUSBStorageConnected() &&
      File::Exists(_T("/media/usb_storage/KoboRoot.tgz"));
#else
  return false;
#endif
}

bool
InstallKoboRootTgz()
{
#ifdef KOBO
  if (!IsUSBStorageKoboRootInRoot())
    return false;

  system(_T("cp /media/usb_storage/KoboRoot.tgz /mnt/onboard/.kobo"));
  system(_T("rm -f /media/usb_storage/KoboRoot.tgz"));
  system(_T("sync"));
  return true;
#else
  return false;
#endif
}
