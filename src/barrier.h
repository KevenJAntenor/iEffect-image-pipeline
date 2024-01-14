#ifndef INF3170_BARRIER_H_
#define INF3170_BARRIER_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct barrier {
  pthread_mutex_t lock;
  pthread_cond_t all_in;
  pthread_cond_t all_out;
  struct timespec delay;
  struct timespec ts;
  int size;
  int in;
  int out;
};

void barrier_init(struct barrier* b, int n, const struct timespec* delay);
void barrier_reset(struct barrier* b);
int barrier_timewait(struct barrier* b);

#ifdef __cplusplus
}
#endif

#endif
