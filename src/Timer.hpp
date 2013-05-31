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

#ifndef XCSOAR_TIMER_HPP
#define XCSOAR_TIMER_HPP

#ifdef ANDROID
#include "Android/Timer.hpp"
#elif defined(ENABLE_SDL)
#include "Thread/Flag.hpp"
#include <SDL_timer.h>
#else
#include "Screen/Window.hpp"
#include "Screen/Timer.hpp"
#endif

#include <assert.h>
#include <stddef.h>

#ifdef ANDROID
class AndroidTimer;
#endif

/**
 * A timer that, once initialized, periodically calls OnTimer() after
 * a specified amount of time, until Cancel() gets called.
 *
 * Initially, this class does not schedule a timer.
 *
 * This class is not thread safe; all of the methods must be called
 * from the main thread.
 *
 * The class #WindowTimer is cheaper on WIN32; use it instead of this
 * class if you are implementing a #Window.
 */
class Timer
#ifdef USE_GDI
  : private Window, private WindowTimer
#endif
{
#ifdef ANDROID
  friend class AndroidTimer;
  AndroidTimer *timer;
#elif defined(ENABLE_SDL)
  SDL_TimerID id;

  /**
   * True when the timer event has been pushed to the event queue.
   * This is used to prevent duplicate items stacking on the event
   * queue.
   */
  Flag queued;
#endif

public:
  /**
   * Construct a Timer object that is not set initially.
   */
#ifdef ANDROID
  Timer():timer(NULL) {}
#elif defined(ENABLE_SDL)
  Timer():id(NULL) {}
#else
  Timer():WindowTimer(*(Window *)this) {
    Window::CreateMessageWindow();
  }
#endif

  Timer(const Timer &other) = delete;

  ~Timer() {
    /* timer must be cleaned up explicitly */
    assert(!IsActive());
  }

#ifdef USE_GDI
  /* inherit WindowTimer's methods */
  using WindowTimer::IsActive;
  using WindowTimer::Schedule;
  using WindowTimer::Cancel;
#else

  /**
   * Is the timer active, i.e. is it waiting for the current period to
   * end?
   */
  bool IsActive() const {
#ifdef ANDROID
    return timer != NULL;
#elif defined(ENABLE_SDL)
    return id != NULL;
#endif
  }

  /**
   * Schedule the timer.  Cancels the previous setting if there was
   * one.
   */
  void Schedule(unsigned ms);

  /**
   * Cancels the scheduled timer, if any.  This is safe to be called
   * while the timer is running.
   */
  void Cancel();

#endif /* !GDI */

protected:
  /**
   * This method gets called after the configured time has elapsed.
   * Implement it.
   */
  virtual void OnTimer() = 0;

#ifdef ANDROID
public:
  void Invoke() {
    OnTimer();
  }
#elif defined(ENABLE_SDL)
private:
  void Invoke();
  static void Invoke(void *ctx);

  Uint32 Callback(Uint32 interval);
  static Uint32 Callback(Uint32 interval, void *param);
#else
private:
  virtual bool OnTimer(WindowTimer &timer);
#endif
};

#endif
