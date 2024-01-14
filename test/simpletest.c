/*
 * test.c - Implementation of test framework interface.
 *
 * Copyright (C) 2014 Andrew Schwartzmeyer
 *
 * This file released under the AGPLv3 license.
 */

#include <stdarg.h>
#include <stdlib.h>

#include "simpletest.h"

int status;

void running(const char *format, ...) {
  status = EXIT_SUCCESS;

  if (!TEST_DEBUG)
    return;

  va_list ap;
  va_start(ap, format);

  printf("RUNNING: ");
  vprintf(format, ap);
  printf(" tests\n");

  va_end(ap);
}

void testing(const char *format, ...) {
  if (!TEST_DEBUG)
    return;

  va_list ap;
  va_start(ap, format);

  printf("TESTING: ");
  vprintf(format, ap);
  printf("\n");

  va_end(ap);
}

void failure(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  status = EXIT_FAILURE;
  fprintf(stderr, "FAILURE: ");
  vfprintf(stderr, format, ap);
  fprintf(stderr, "!\n");

  va_end(ap);
}
