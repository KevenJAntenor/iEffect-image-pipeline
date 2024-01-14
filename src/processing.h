#ifndef INF3170_PROCESSING_H_
#define INF3170_PROCESSING_H_

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

#include "image.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct work_item {
  char *input_file;
  char *output_file;
  image_t *image;
};

void free_work_item(void *item);

int process_multithread(struct list *items, int nb_thread);
int process_serial(struct list *items);

#ifdef __cplusplus
}
#endif

#endif
