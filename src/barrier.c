#define _GNU_SOURCE

#include "barrier.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*
 * Barrier implementation with timeout.
 */

static void timespec_add_ns(struct timespec *ts, const struct timespec *delay) {
  ts->tv_sec += delay->tv_sec;
  ts->tv_nsec += delay->tv_nsec;
  if (ts->tv_nsec >= 1000000000) {
    ts->tv_sec++;
    ts->tv_nsec -= 1000000000;
  }
}

void barrier_init(struct barrier *b, int n, const struct timespec *delay) {
  pthread_mutex_init(&b->lock, NULL);
  pthread_cond_init(&b->all_in, NULL);
  pthread_cond_init(&b->all_out, NULL);
  b->size = n;
  b->in = 0;
  b->out = 0;
  b->delay = *delay;
  barrier_reset(b);
}

void barrier_reset(struct barrier *b) {
  pthread_mutex_lock(&b->lock);
  clock_gettime(CLOCK_REALTIME, &b->ts);
  timespec_add_ns(&b->ts, &b->delay);
  pthread_mutex_unlock(&b->lock);
}

int barrier_timewait(struct barrier *b) {
  pthread_mutex_lock(&b->lock);
  b->in++;

  // Last thread to reach the barrier
  if (b->in == b->size) {
    pthread_cond_broadcast(&b->all_in);
  } else {
    // Waiting...
    while (b->in != b->size) {
      if (pthread_cond_timedwait(&b->all_in, &b->lock, &b->ts) == ETIMEDOUT) {
        pthread_mutex_unlock(&b->lock);
        return ETIMEDOUT;
      }
    }
  }

  b->out++;

  // Last thread to exit
  if (b->out == b->size) {
    b->in = 0;
    b->out = 0;
    pthread_cond_broadcast(&b->all_out);
  } else {
    while (b->out != 0) {
      pthread_cond_wait(&b->all_out, &b->lock);
    }
  }

  pthread_mutex_unlock(&b->lock);
  return 0;
}
