/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#if !defined(ANDROID)

#include "SoundQueue.hpp"
#include "Sound.hpp"
#include "Thread/StandbyThread.hpp"
#include <queue>

#include <assert.h>

/**
 * class that stores sounds to play and automatically plays them in order
 */
class PlayQueue : StandbyThread {

  std::queue<SoundQueue::SoundName> sound_name_queue;
  /// protect the push/pop of the queue, while allowing concurrent playing
  Mutex mutex_queue;

public:
  PlayQueue()
    :StandbyThread("PlayQueue") {};

  void Enqueue(SoundQueue::SoundName sound_name);

  void StopAsync() {
    ScopeLock protect(mutex);
    StandbyThread::StopAsync();
  }

  void WaitStopped() {
    ScopeLock protect(mutex);
    StandbyThread::WaitStopped();
  }

protected:
  /* methods from class StandbyThread */
  void Tick() override;
};


void
PlayQueue::Enqueue(SoundQueue::SoundName sound_name)
{
  {
    ScopeLock protect_queue(mutex_queue);
    sound_name_queue.push(sound_name);
  }

  ScopeLock protect(mutex);
  if (!IsBusy())
    Trigger();
}

void
PlayQueue::Tick()
{
  SoundQueue::SoundName sound_name;

  while (!sound_name_queue.empty() && !StandbyThread::IsStopped()) {
    mutex.Unlock();
    mutex.Lock();
    {
      ScopeLock protect_queue(mutex_queue);
      sound_name = sound_name_queue.front();
      sound_name_queue.pop();
    }
    PlayResourceNow(sound_name);
  }
  return;
}


static PlayQueue *play_queue;

bool
SoundQueue::Enqueue(SoundQueue::SoundName sound_name)
{
  assert(play_queue != nullptr);
  play_queue->Enqueue(sound_name);
  return true;
}

void
SoundQueue::Initialise()
{
  assert(play_queue == nullptr);
  play_queue = new PlayQueue();
}

void
SoundQueue::BeginDeinitialise()
{
  assert(play_queue != nullptr);
  play_queue->StopAsync();
}

void
SoundQueue::Deinitialise()
{
  assert(play_queue != nullptr);
  play_queue->WaitStopped();
  delete play_queue;
  play_queue = nullptr;
}

#endif // !ANDROID
