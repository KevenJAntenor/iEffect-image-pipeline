#ifndef INF3170_THREADPOOL_H_
#define INF3170_THREADPOOL_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pipeline_stage;
typedef void *(*func_t)(void *item, struct pipeline_stage *next_stage);

struct pipeline_stage {
  sem_t free;
  sem_t busy;
  struct list *queue;
  pthread_mutex_t queue_lock;
  func_t func;
  int num_threads;
  pthread_t *threads;
  struct pipeline_stage *next_stage;
  int stage_id;
};

struct pipeline {
  struct list *stages;
};

struct pipeline *pipeline_create();
int pipeline_add_stage(struct pipeline *p, func_t f, int nthread,
                       int queue_limit);
void pipeline_enqueue(struct pipeline *p, void *arg);
void pipeline_stage_enqueue(struct pipeline_stage *s, void *arg);
void pipeline_join(struct pipeline *p);
void pipeline_destroy(struct pipeline *p);
void free_pipeline_stage(void *obj);

#ifdef __cplusplus
}
#endif

#endif /* INF3170_THREADPOOL_H_ */
