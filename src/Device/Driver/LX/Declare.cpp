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

#include "Device/Driver/LX.hpp"
#include "Device/Driver/LX/Internal.hpp"
#include "Device/Driver/LX/Protocol.hpp"
#include "Device/Port/Port.hpp"
#include "OS/ByteOrder.hpp"
#include "Operation/Operation.hpp"

/**
 * fills dest with src and appends spaces to end
 * adds '\0' to end of string resulting in
 * len characters with last char = '\0'
 */
static void
copy_space_padded(char dest[], const TCHAR src[], unsigned int len)
{
  const unsigned slen = _tcslen(src);
  for(unsigned i = 0; i < (len - 1); i++) {
    if (i < slen)
      dest[i] = (char)max((src[i] & 0x7f), 0x20);
    else
      dest[i] = '\x20';
  }
  dest[len-1] = '\0';
}

static void
LoadPilotInfo(LX::Pilot &lxDevice_Pilot, const Declaration &declaration)
{
  memset((void*)lxDevice_Pilot.unknown1, 0, sizeof(lxDevice_Pilot.unknown1));
  copy_space_padded(lxDevice_Pilot.PilotName, declaration.pilot_name,
                    sizeof(lxDevice_Pilot.PilotName));
  copy_space_padded(lxDevice_Pilot.GliderType, declaration.aircraft_type,
                    sizeof(lxDevice_Pilot.GliderType));
  copy_space_padded(lxDevice_Pilot.GliderID, declaration.aircraft_registration,
                    sizeof(lxDevice_Pilot.GliderID));
  copy_space_padded(lxDevice_Pilot.CompetitionID, declaration.competition_id,
                    sizeof(lxDevice_Pilot.CompetitionID));
  memset((void*)lxDevice_Pilot.unknown2, 0, sizeof(lxDevice_Pilot.unknown2));
}

gcc_const
static int32_t
AngleToLX(Angle value)
{
  return ToBE32((int32_t)(value.Degrees() * 60000));
}

/**
 * Loads LX task structure from XCSoar task structure
 * @param decl  The declaration
 */
static bool
LoadTask(LX::Declaration &lxDevice_Declaration, const Declaration &declaration)
{
  if (declaration.Size() > 10)
    return false;

  if (declaration.Size() < 2)
      return false;

  memset((void*)lxDevice_Declaration.unknown1, 0,
          sizeof(lxDevice_Declaration.unknown1));

  BrokenDate DeclDate;
  DeclDate.day = 1;
  DeclDate.month = 1;
  DeclDate.year = 2010;

  if (DeclDate.day > 0 && DeclDate.day < 32
      && DeclDate.month > 0 && DeclDate.month < 13) {
    lxDevice_Declaration.dayinput = (unsigned char)DeclDate.day;
    lxDevice_Declaration.monthinput = (unsigned char)DeclDate.month;
    int iCentury = DeclDate.year / 100; // Todo: if no gps fix, use system time
    iCentury *= 100;
    lxDevice_Declaration.yearinput = (unsigned char)(DeclDate.year - iCentury);
  }
  else {
    lxDevice_Declaration.dayinput = (unsigned char)1;
    lxDevice_Declaration.monthinput = (unsigned char)1;
    lxDevice_Declaration.yearinput = (unsigned char)10;
  }
  lxDevice_Declaration.dayuser = lxDevice_Declaration.dayinput;
  lxDevice_Declaration.monthuser = lxDevice_Declaration.monthinput;
  lxDevice_Declaration.yearuser = lxDevice_Declaration.yearinput;
  lxDevice_Declaration.taskid = 0;
  lxDevice_Declaration.numtps = declaration.Size();

  for (unsigned i = 0; i < LX::NUMTPS; i++) {
    if (i == 0) { // takeoff
      lxDevice_Declaration.tptypes[i] = 3;
      lxDevice_Declaration.Latitudes[i] = 0;
      lxDevice_Declaration.Longitudes[i] = 0;
      copy_space_padded(lxDevice_Declaration.WaypointNames[i], _T("TAKEOFF"),
        sizeof(lxDevice_Declaration.WaypointNames[i]));


    } else if (i <= declaration.Size()) {
      lxDevice_Declaration.tptypes[i] = 1;
      lxDevice_Declaration.Longitudes[i] =
        AngleToLX(declaration.GetLocation(i - 1).longitude);
      lxDevice_Declaration.Latitudes[i] =
        AngleToLX(declaration.GetLocation(i - 1).latitude);
      copy_space_padded(lxDevice_Declaration.WaypointNames[i],
                        declaration.GetName(i - 1),
                        sizeof(lxDevice_Declaration.WaypointNames[i]));

    } else if (i == declaration.Size() + 1) { // landing
      lxDevice_Declaration.tptypes[i] = 2;
      lxDevice_Declaration.Longitudes[i] = 0;
      lxDevice_Declaration.Latitudes[i] = 0;
      copy_space_padded(lxDevice_Declaration.WaypointNames[i], _T("LANDING"),
          sizeof(lxDevice_Declaration.WaypointNames[i]));

    } else { // unused
      lxDevice_Declaration.tptypes[i] = 0;
      lxDevice_Declaration.Longitudes[i] = 0;
      lxDevice_Declaration.Latitudes[i] = 0;
      memset((void*)lxDevice_Declaration.WaypointNames[i], 0, 9);
    }
  }

  return true;
}

static void
LoadContestClass(LX::ContestClass &lxDevice_ContestClass,
                 gcc_unused const Declaration &declaration)
{
  copy_space_padded(lxDevice_ContestClass.contest_class, _T(""),
                    sizeof(lxDevice_ContestClass.contest_class));
}

static bool
DeclareInner(Port &port, const Declaration &declaration,
             gcc_unused OperationEnvironment &env)
{
  env.SetProgressRange(5);
  env.SetProgressPosition(0);

  if (!LX::CommandMode(port, env))
      return false;

  if (env.IsCancelled())
    return false;

  env.SetProgressPosition(1);

  LX::Pilot pilot;
  LoadPilotInfo(pilot, declaration);

  LX::Declaration lxDevice_Declaration;
  if (!LoadTask(lxDevice_Declaration, declaration))
    return false;

  LX::ContestClass contest_class;
  LoadContestClass(contest_class, declaration);

  if (env.IsCancelled())
    return false;

  env.SetProgressPosition(2);

  LX::SendCommand(port, LX::WRITE_FLIGHT_INFO); // start declaration

  LX::CRCWriter writer(port);
  writer.Write(&pilot, sizeof(pilot), env);
  env.SetProgressPosition(3);

  if (env.IsCancelled())
    return false;

  writer.Write(&lxDevice_Declaration, sizeof(lxDevice_Declaration), env);
  writer.Flush();
  if (!LX::ExpectACK(port, env))
    return false;

  if (env.IsCancelled())
    return false;

  env.SetProgressPosition(4);
  LX::SendCommand(port, LX::WRITE_CONTEST_CLASS);
  writer.Write(&contest_class, sizeof(contest_class), env);
  env.SetProgressPosition(5);

  writer.Flush();
  return LX::ExpectACK(port, env);
}

bool
LXDevice::Declare(const Declaration &declaration,
                  gcc_unused const Waypoint *home,
                  OperationEnvironment &env)
{
  if (declaration.Size() < 2 || declaration.Size() > 12)
    return false;

  if (!EnableCommandMode(env))
    return false;

  bool success = DeclareInner(port, declaration, env);

  LX::CommandModeQuick(port, env);

  return success;
}
