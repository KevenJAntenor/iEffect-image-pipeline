/*
 * test.h - Simple C Unit Test Framework interface.
 *
 * Copyright (C) 2014 Andrew Schwartzmeyer
 *
 * This file released under the AGPLv3 license.
 */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#ifndef TEST_DEBUG
#define TEST_DEBUG 1
#endif

extern int status;

void running(const char *format, ...);
void testing(const char *format, ...);
void failure(const char *format, ...);

#endif /* TEST_H */
