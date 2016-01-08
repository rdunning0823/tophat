/*

This example reads standard from input and writes
to the default PCM device for 5 seconds of data.

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "raw_play.hpp"
#include <unistd.h>
#include <stdlib.h>

int
RawPlayback::playback_chunk(short *buff, int count)
{
  long loops;
  int rc;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  short *p;
  int len;

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  rc = snd_pcm_hw_params_any(handle, params);
  if (rc < 0) {
    rc = -3;
    goto _return;
  }

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  rc = snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);
  if (rc < 0) {
    rc = -4;
    goto _return;
  }

  /* Signed 16-bit little-endian format */
  rc = snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);
  if (rc < 0) {
    rc = -5;
    goto _return;
  }

  /* Two channels (stereo) */
  rc = snd_pcm_hw_params_set_channels(handle, params, 1);
  if (rc < 0) {
    rc = -6;
    goto _return;
  }

  val = PLAYBACK_RATE;
  rc = snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);
  if (rc < 0) {
    rc = -7;
    goto _return;
  }

  /* Set period size to 32 frames. */
  frames = 32;
  rc = snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);
  if (rc < 0) {
    rc = -8;
    goto _return;
  }

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    rc = -9;
    goto _return;
  }

  /* Use a buffer large enough to hold one period */
  rc = snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  if (rc < 0) {
    rc = -10;
    goto _return;
  }

  /* We want to loop for 5 seconds */
  rc = snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
  loops = PLAYBACK_CHUNK / val;

  p = buff;
  while (loops > 0) {
    loops--;
    if (count == 0)
      break;
    if (count > (int)frames)
      len = (int)frames;
    else
      len = count;
    rc = snd_pcm_writei(handle, p, len);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      LogFormat("underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      LogFormat("error from writei: %s\n",
              snd_strerror(rc));
    }  else {
      p += rc;
      count -= rc;
      if (rc != len)
	LogFormat("short write, write %d frames\n", rc);
    }
  }

  snd_pcm_drain(handle);
 _return:
  return rc;
}

int
RawPlayback::playback_mem(short *buff, int count)
{
  int rc, n;

  if (!handle)
    goto _return;

  while (count > 0) {
    if (count > PLAYBACK_RATE *  /* PLAYBACK_CHUNK/1000000usec */
	(PLAYBACK_CHUNK/100000)/(1000000/100000))
      n = PLAYBACK_RATE * 1/2;
    else
      n = count;
    rc = playback_chunk(buff,  n);
    if (rc < 0)
      return rc;
    buff += n;
    count -= n;
  }
 _return:
  return 0;
}

int
RawPlayback::playback_file(const char *name)
{
  int rc = 0;
  void *buffer;
  int raw_file;
  off_t size;

  if (!handle)
    goto _return;

  raw_file = open(name, O_RDONLY);
  if (raw_file < 0) {
    LogFormat("can't open: %s\n", name);
    rc = raw_file;
    goto _return;
  }
  size = lseek(raw_file, 0, SEEK_END);
  if (size < 0) {
    LogFormat("can't seek to end o f file: %s\n", name);
    rc = -errno;
    goto _close_return;
  }
  rc = lseek(raw_file, 0, SEEK_SET);
  if (rc < 0) {
    LogFormat("can't seet to beginning of file: %s\n", name);
    rc = -errno;
    goto _close_return;
  }

  buffer = malloc(size);
  if (!buffer) {
    LogFormat("can't allocate %ld byte\n", size);
    rc = -errno;
    goto _close_return;
  }

  rc = read(raw_file, buffer, size);
  if (rc < size) {
    LogFormat("can't read %ld byte: %d\n", size, rc);
    rc = -errno;
    goto _free_return;
  }

  rc = playback_mem((short *)buffer, size/sizeof(short));
  if (rc < 0)
    LogFormat("can't play: %s\n", name);
  
 _free_return:
  free(buffer);
 _close_return:
  close(raw_file);
 _return:
  return rc;
}
