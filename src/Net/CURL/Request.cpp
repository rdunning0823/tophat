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

#include "Net/Request.hpp"
#include "Net/Session.hpp"
#include "Version.hpp"

#include <assert.h>

Net::Request::Request(Session &_session, const TCHAR *url,
                      unsigned timeout_ms)
  :session(_session), handle(curl_easy_init())
{
  // XXX implement timeout

  if (handle == NULL)
    return;

  char user_agent[32];
  snprintf(user_agent, 32, "XCSoar/%s", XCSoar_Version);

  curl_easy_setopt(handle, CURLOPT_USERAGENT, user_agent);
  curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

  CURLcode code = curl_easy_setopt(handle, CURLOPT_URL, url);
  if (code != CURLE_OK) {
    curl_easy_cleanup(handle);
    handle = NULL;
    return;
  }

  if (!session.Add(handle)) {
    curl_easy_cleanup(handle);
    handle = NULL;
    return;
  }
}

Net::Request::~Request()
{
  if (handle != NULL) {
    session.Remove(handle);
    curl_easy_cleanup(handle);
  }
}

size_t
Net::Request::ResponseData(const uint8_t *ptr, size_t size)
{
  Buffer::Range range = buffer.Write();
  if ((size_t)range.length < size)
    /* buffer is full, pause CURL */
    return CURL_WRITEFUNC_PAUSE;

  std::copy(ptr, ptr + size, range.data);
  buffer.Append(size);
  return size;
}

size_t
Net::Request::WriteCallback(void *ptr, size_t size, size_t nmemb,
                            void *userdata)
{
  Request *request = (Request *)userdata;
  return request->ResponseData((const uint8_t *)ptr, size * nmemb);
}

bool
Net::Request::Send(unsigned _timeout_ms)
{
  if (handle == NULL)
    return false;

  const int timeout_ms = _timeout_ms == INFINITE ? -1 : _timeout_ms;

  CURLMcode mcode = CURLM_CALL_MULTI_PERFORM;
  while (buffer.IsEmpty()) {
    CURLcode code = session.InfoRead(handle);
    if (code != CURLE_AGAIN)
      return code == CURLE_OK;

    if (mcode != CURLM_CALL_MULTI_PERFORM &&
        !session.Select(timeout_ms))
      return false;

    mcode = session.Perform();
    if (mcode != CURLM_OK && mcode != CURLM_CALL_MULTI_PERFORM)
      return false;
  }

  return true;
}

int64_t
Net::Request::GetLength() const
{
  assert(handle != NULL);

  double value;
  return curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                           &value) == CURLE_OK
    ? (int64_t)value
    : -1;
}

ssize_t
Net::Request::Read(void *_buffer, size_t buffer_size, unsigned _timeout_ms)
{
  assert(handle != NULL);

  const int timeout_ms = _timeout_ms == INFINITE ? -1 : _timeout_ms;

  Buffer::Range range;
  CURLMcode mcode = CURLM_CALL_MULTI_PERFORM;
  while (true) {
    range = buffer.Read();
    if (!range.IsEmpty())
      break;

    CURLcode code = session.InfoRead(handle);
    if (code != CURLE_AGAIN)
      return code == CURLE_OK ? 0 : -1;

    if (mcode != CURLM_CALL_MULTI_PERFORM &&
        !session.Select(timeout_ms))
      return -1;

    mcode = session.Perform();
    if (mcode != CURLM_OK && mcode != CURLM_CALL_MULTI_PERFORM)
      return -1;
  }

  --buffer_size;
  if (buffer_size > range.length)
    buffer_size = range.length;

  uint8_t *p = (uint8_t *)_buffer;
  std::copy(range.data, range.data + buffer_size, p);
  p[buffer_size] = 0;

  buffer.Consume(buffer_size);
  curl_easy_pause(handle, CURLPAUSE_CONT);

  return buffer_size;
}
