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

#include "System.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "OS/Process.hpp"
#include "OS/Sleep.h"
#include "Util/StaticString.hxx"
#include "Net/IpAddress.hpp"
#include "DisplaySettings.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef KOBO

#include <sys/mount.h>
#include <errno.h>

template<typename... Args>
static bool
InsMod(const char *path, Args... args)
{
  return Run("/sbin/insmod", path, args...);
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
KoboUmountData()
{
#ifdef KOBO
  return umount("/mnt/onboard") == 0 || errno == EINVAL;
#else
  return true;
#endif
}

bool
KoboMountData()
{
#ifdef KOBO
  Run("/bin/dosfsck", "-a", "-w", "/dev/mmcblk0p3");
  return mount("/dev/mmcblk0p3", "/mnt/onboard", "vfat",
               MS_NOATIME|MS_NODEV|MS_NOEXEC|MS_NOSUID,
               "iocharset=utf8");
#else
  return true;
#endif
}

bool
KoboExportUSBStorage()
{
#ifdef KOBO
  RmMod("g_ether");
  RmMod("g_file_storage");

  InsMod("/drivers/ntx508/usb/gadget/arcotg_udc.ko");
  return InsMod("/drivers/ntx508/usb/gadget/g_file_storage.ko",
                "file=/dev/mmcblk0p3", "stall=0");
#else
  return true;
#endif
}

void
KoboUnexportUSBStorage()
{
#ifdef KOBO
  RmMod("g_ether");
  RmMod("g_file_storage");
  RmMod("arcotg_udc");
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
  if (system(command) != 0)
    return false;
  StaticString <256> buffer;
  File::ReadString(local_path, buffer.buffer(), 256);
  return buffer.Contains(search_string);
#else
  return false;
#endif
}

bool
WriteSystemInfo()
{
#ifdef KOBO
  static char cmd_kobo_kernel_info[] = "/bin/dd if=/dev/mmcblk0 bs=8 count=1 skip=64";
  static char cmd_uname_all[] = "/bin/uname -a";

  const char *local_path = "/TophatSystemInfo.txt";
  TCHAR command[256];
  _stprintf(command, _T("%s > %s"), cmd_kobo_kernel_info, local_path);
  bool retval = system(command);
  _stprintf(command, _T(" %s >> %s"), cmd_uname_all, local_path);
  retval = retval && system(command);
  return retval;
#endif
  return false;
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

DisplayOrientation
ReadKoboLastScreenOrientation()
{
  DisplayOrientation orientation =
      DisplayOrientation::DEFAULT;
  char line[4];

  if (File::ReadString(kobo_orientation_file, line, sizeof(line))) {
    switch(line[0]) {
    case '3':
      orientation = DisplayOrientation::PORTRAIT;
      break;
    case '1':
      orientation = DisplayOrientation::REVERSE_PORTRAIT;
      break;
    case '0':
      orientation = DisplayOrientation::LANDSCAPE;
      break;
    case '2':
      orientation = DisplayOrientation::REVERSE_LANDSCAPE;
      break;
    }
  }
  return orientation;
}

static const char *kobo_use_sunblind_file =
    "/mnt/onboard/XCSoarData/SunblindKoboLast.tx0";
void
WriteUseKoboMiniSunblind(bool value)
{
  File::CreateExclusive(kobo_use_sunblind_file);
  File::WriteExisting(kobo_use_sunblind_file, value ? "1" : "0");
}

bool
ReadUseKoboMiniSunblind()
{
  char line[4];
  if (File::ReadString(kobo_use_sunblind_file, line, sizeof(line))) {
    if (line[0] == '1') {
      return true;
    }
  }
  return false;
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

bool
UploadTasksToDevice()
{
#ifdef KOBO
  Directory::Create(_T("/mnt/onboard/XCSoarData/tasks"));
  return Run("/bin/cp", "-r", "/media/usb_storage/XCSoarData/tasks", "/mnt/onboard/XCSoarData");
#else
  return false;
#endif
}

bool
CopyFlightsToSDCard()
{
#ifdef KOBO
  Directory::Create(_T("/media/usb_storage/XCSoarData"));
  Directory::Create(_T("/media/usb_storage/XCSoarData/logs"));
  bool retval = Run("/bin/cp", "-r", "-p", "/mnt/onboard/XCSoarData/logs/", "/media/usb_storage/XCSoarData/");
  return retval && Run("/bin/sync");
  #else
  return false;
#endif
}

bool
UploadSDCardToDevice()
{
#ifdef KOBO
  return Run("/bin/cp", "-r", "-p", "/media/usb_storage/XCSoarData", "/mnt/onboard");
#else
  return false;
#endif
}

bool
CleanSDCard()
{
#ifdef KOBO
  bool retval = Run("/bin/rm", "-r", "-f", "/mnt/onboard/XCSoarData");
  mkdir("/mnt/onboard/XCSoarData", 0777);
  mkdir("/mnt/onboard/XCSoarData/kobo", 0777);
  return retval && Run("/bin/sync");
#else
  return false;
#endif
}

bool
CopyTopHatDataToSDCard()
{
#ifdef KOBO
  bool retval = Run("/bin/cp", "-r", "-p", "/mnt/onboard/XCSoarData", "/media/usb_storage");
  return retval && Run("/bin/sync");
#else
  return false;
#endif
}

void
UnmountKoboUSBStorage()
{
#ifdef KOBO
  if (!IsUSBStorageConnected())
    return;
  Run("/bin/umount /media/usb_storage");
  Run("/bin/rm -rf /media/usb_storage");
  Run("/bin/sync");
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

  bool retval = Run("/bin/cp", "/media/usb_storage/KoboRoot.tgz", "/mnt/onboard/.kobo");
  return retval && Run("/bin/sync");
#else
  return false;
#endif
}

void
KoboRunFtpd()
{
#ifdef KOBO
  /* ftpd needs to be fired through tcpsvd (or inetd) */
  Start("/usr/bin/tcpsvd", "-E", "0.0.0.0", "21", "ftpd", "-w", "/mnt/onboard");
#endif
}
