#ifndef INF3170_UTILS_H_
#define INF3170_UTILS_H_

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

int is_regular_file(const char* path);
int is_dir(const char* path);
int ends_with(const char* str, const char* suffix);

#ifdef __cplusplus
}
#endif

#endif
