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

#include "Device/Driver/LX/Protocol.hpp"
#include "Operation/Operation.hpp"

bool
LX::CommandMode(Port &port, OperationEnvironment &env)
{
  /* switch to command mode, first attempt */

  if (!SendSYN(port) || !port.FullFlush(env, 10, 20))
    return false;

  /* the port is clean now; try the SYN/ACK procedure up to three
     times */
  for (unsigned i = 0; i < 100 && !env.IsCancelled(); ++i)
    if (Connect(port, env))
      return true;

  return false;
}

void
LX::CommandModeQuick(Port &port, OperationEnvironment &env)
{
  SendSYN(port);
  env.Sleep(500);
  SendSYN(port);
  env.Sleep(500);
  SendSYN(port);
  env.Sleep(500);
}

bool
LX::SendPacket(Port &port, Command command,
               const void *data, size_t length,
               OperationEnvironment &env, unsigned timeout_ms)
{
  return SendCommand(port, command) &&
    port.FullWrite(data, length, env, timeout_ms) &&
    port.Write(calc_crc(data, length, 0xff));
}

bool
LX::ReceivePacket(Port &port, Command command,
                  void *data, size_t length, OperationEnvironment &env,
                  unsigned timeout_ms)
{
  port.Flush();
  return SendCommand(port, command) &&
    ReadCRC(port, data, length, env, timeout_ms);
}

uint8_t
LX::calc_crc_char(uint8_t d, uint8_t crc)
{
  uint8_t tmp;
  const uint8_t crcpoly = 0x69;
  int count;

  for (count = 8; --count >= 0; d <<= 1) {
    tmp = crc ^ d;
    crc <<= 1;
    if (tmp & 0x80)
      crc ^= crcpoly;
  }
  return crc;
}

uint8_t
LX::calc_crc(const void *p0, size_t len, uint8_t crc)
{
  const uint8_t *p = (const uint8_t *)p0;
  size_t i;

  for (i = 0; i < len; i++)
    crc = calc_crc_char(p[i], crc);

  return crc;
}

bool
LX::ReadCRC(Port &port, void *buffer, size_t length, OperationEnvironment &env,
            unsigned timeout_ms)
{
  uint8_t crc;

  return port.FullRead(buffer, length, env, timeout_ms) &&
    port.FullRead(&crc, sizeof(crc), env, timeout_ms) &&
    calc_crc(buffer, length, 0xff) == crc;
}
