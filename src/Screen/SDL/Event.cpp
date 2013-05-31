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

#include "Screen/SDL/Event.hpp"
#include "Thread/Debug.hpp"
#include "Thread/Notify.hpp"
#include "Asset.hpp"

#include "Screen/TopWindow.hpp"

bool
EventLoop::Get(SDL_Event &event)
{
  if (bulk) {
    if (::SDL_PollEvent(&event))
      return true;

    /* that was the last event for now, refresh the screen now */
    top_window.Refresh();
    bulk = false;
  }

  if (::SDL_WaitEvent(&event)) {
    bulk = true;
    return true;
  }

  return false;
}

void
EventLoop::Dispatch(SDL_Event &event)
{
  if (event.type == EVENT_USER && event.user.data1 != NULL) {
    Window *window = (Window *)event.user.data1;
    window->OnUser(event.user.code);
  } else if (event.type == EVENT_CALLBACK) {
    Callback callback = (Callback)event.user.data1;
    callback(event.user.data2);
  } else if (event.type == EVENT_NOTIFY && event.user.data1 != NULL) {
    Notify *notify = (Notify *)event.user.data1;
    notify->RunNotification();
  } else
    top_window.OnEvent(event);
}

void
EventQueue::Push(EventLoop::Callback callback, void *ctx)
{
  SDL_Event event;
  event.type = EVENT_CALLBACK;
  event.user.data1 = (void *)callback;
  event.user.data2 = ctx;
  ::SDL_PushEvent(&event);
}

void
EventQueue::Purge(Uint32 mask,
                  bool (*match)(const SDL_Event &event, void *ctx),
                  void *ctx)
{
  SDL_Event events[256]; // is that enough?
  int count = SDL_PeepEvents(events, 256, SDL_GETEVENT, mask);
  assert(count >= 0);

  SDL_Event *dest = events;
  for (const SDL_Event *src = events, *end = src + count; src != end; ++src)
    if (!match(*src, ctx))
      *dest++ = *src;

  SDL_PeepEvents(events, dest - events, SDL_ADDEVENT, mask);
}

struct MatchCallbackData {
  void *data1, *data2;
};

static bool
MatchCallback(const SDL_Event &event, void *ctx)
{
  const MatchCallbackData *data = (const MatchCallbackData *)ctx;
  return event.type == EVENT_CALLBACK && event.user.data1 == data->data1 &&
    event.user.data2 == data->data2;
}

void
EventQueue::Purge(EventLoop::Callback callback, void *ctx)
{
  MatchCallbackData data { (void *)callback, ctx };
  Purge(SDL_EVENTMASK(EVENT_CALLBACK), MatchCallback, (void *)&data);
}

static bool
match_notify(const SDL_Event &event, void *ctx)
{
  return event.type == EVENT_NOTIFY && event.user.data1 == ctx;
}

void
EventQueue::Purge(Notify &notify)
{
  Purge(SDL_EVENTMASK(EVENT_NOTIFY), match_notify, (void *)&notify);
}

static bool
match_window(const SDL_Event &event, void *ctx)
{
  return event.type == EVENT_USER &&
    event.user.data1 == ctx;
}

void
EventQueue::Purge(Window &window)
{
  Purge(SDL_EVENTMASK(EVENT_USER),
        match_window, (void *)&window);
}
