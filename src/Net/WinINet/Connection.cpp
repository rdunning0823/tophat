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

#include "Net/Connection.hpp"
#include "Net/Session.hpp"

static void CALLBACK
ConnectionCallback(HINTERNET hInternet,
                   DWORD_PTR dwContext,
                   DWORD dwInternetStatus,
                   LPVOID lpvStatusInformation,
                   DWORD dwStatusInformationLength)
{
  Net::Connection *connection = (Net::Connection *)dwContext;

  connection->Callback(dwInternetStatus,
                       lpvStatusInformation, dwStatusInformationLength);
}

Net::Connection::Connection(Session &session, const char *server,
                            unsigned timeout_ms)
{
  INTERNET_STATUS_CALLBACK old_callback =
    session.handle.SetStatusCallback(ConnectionCallback);
  HINTERNET h = session.handle.Connect(server,
                                       INTERNET_DEFAULT_HTTP_PORT, NULL, NULL,
                                       INTERNET_SERVICE_HTTP, 0,
                                       (DWORD_PTR)this);
  session.handle.SetStatusCallback(old_callback);

  if (h == NULL && GetLastError() == ERROR_IO_PENDING)
    // Wait until we get the connection handle
    event.Wait(timeout_ms);
}

bool
Net::Connection::Connected() const
{
  return handle.IsDefined();
}

void
Net::Connection::Callback(DWORD status, LPVOID info, DWORD info_length)
{
  if (status == INTERNET_STATUS_HANDLE_CREATED) {
    INTERNET_ASYNC_RESULT *res = (INTERNET_ASYNC_RESULT *)info;
    handle.Set((HINTERNET)res->dwResult);
    event.Signal();
  }
}
