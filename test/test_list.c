/*
 * test_list.c - Unit test code for doubly linked list with sentinel.
 *
 * Copyright (C) 2014 Andrew Schwartzmeyer
 *
 * This file released under the AGPLv3 license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "simpletest.h"

void test_new(struct list *list);
void test_sentinel(struct list *list);
void test_size(struct list *list, size_t size);
void test_empty(struct list *list);
void test_empty_sentinel(struct list *list);
void test_empty_ends(struct list *list);
void test_non_empty(struct list *list);
void test_head_data(struct list *list, char *data);
void test_tail_data(struct list *list, char *data);
void test_search_data(struct list *list, char *data);
void test_not_search_data(struct list *list, char *data);
void test_iter_forward(struct list *list);
void test_iter_backward(struct list *list);

int main() {
  running("list");

  testing("new");
  struct list *list = list_new(NULL, &free);
  test_new(list);
  test_sentinel(list);
  test_empty(list);
  test_empty_sentinel(list);
  test_empty_ends(list);

  testing("push back (A)");
  char *a = strdup("(A)");
  list_push_back(list, list_node_new(a));

  testing("head and tail");
  test_head_data(list, a);
  test_tail_data(list, a);

  testing("search (A)");
  test_search_data(list, "(A)");

  testing("not search (B)");
  test_not_search_data(list, "(B)");

  testing("push back (B)");
  char *b = strdup("(B)");
  list_push_back(list, list_node_new(b));
  test_tail_data(list, b);

  testing("push front (B)");
  list_push_front(list, list_node_new(b));
  test_head_data(list, b);

  testing("not empty");
  test_non_empty(list);
  test_size(list, 3);

  testing("pop back");
  free(list_pop_back(list));
  test_tail_data(list, a);

  testing("pop front");
  free(list_pop_front(list));
  test_head_data(list, a);

  testing("emptied");
  free(list_pop_back(list));
  test_empty(list);

  testing("(A) concat (B) (C)");
  struct list *list_ = list_new(NULL, &free);
  char *c = strdup("(C)");
  list_push_back(list, list_node_new(a));
  list_push_back(list_, list_node_new(b));
  list_push_back(list_, list_node_new(c));
  test_size(list, 1);
  test_size(list_, 2);
  test_head_data(list, a);
  test_head_data(list_, b);
  test_tail_data(list_, c);
  list_concat(list, list_);
  test_head_data(list, a);
  test_tail_data(list, c);
  test_size(list, 3);

  testing("forward iteration:");
  test_iter_forward(list);

  testing("backward iteration:");
  test_iter_backward(list);

  list_free(list);

  return status;
}

void test_new(struct list *list) {
  if (list == NULL)
    failure("list should not have been null");
}

void test_sentinel(struct list *list) {
  struct list_node *sentinel = list->sentinel;

  if (!sentinel->sentinel)
    failure("sentinel should be marked as such");

  if (!list_end(sentinel))
    failure("sentinel should have been list end");
}

void test_size(struct list *list, size_t size) {
  if (list_size(list) != size)
    failure("size should have been %lu", size);
}

void test_empty(struct list *list) {
  test_size(list, 0);

  if (!list_empty(list))
    failure("empty list should have been empty");
}

void test_empty_sentinel(struct list *list) {
  struct list_node *sentinel = list->sentinel;

  if (sentinel != list_head(list))
    failure("empty list sentinel should equal head");

  if (sentinel != list_tail(list))
    failure("empty list sentinel should equal tail");

  if (sentinel->next != sentinel)
    failure("empty list sentinel next should be itself");

  if (sentinel->prev != sentinel)
    failure("empty list sentinel prev should be itself");
}

void test_empty_ends(struct list *list) {
  struct list_node *head = list_head(list);
  struct list_node *tail = list_tail(list);

  if (head != tail)
    failure("empty list head should equal tail");

  if (!list_end(head))
    failure("empty list head should have been end");
}

void test_non_empty(struct list *list) {
  if (list_empty(list))
    failure("should not have been empty");
}

void test_head_data(struct list *list, char *data) {
  if (strcmp(list_front(list), data) != 0)
    failure("head should have been '%s'", data);
}

void test_tail_data(struct list *list, char *data) {
  if (strcmp(list_back(list), data) != 0)
    failure("tail should have been '%s'", data);
}

void test_search_data(struct list *list, char *data) {
  if (!list_search(list, data))
    failure("list should have had '%s'", data);
}

void test_not_search_data(struct list *list, char *data) {
  if (list_search(list, data))
    failure("list should not have had '%s'", data);
}

void test_iter_forward(struct list *list) {
  struct list_node *iter = list_head(list);
  size_t counter = 0;
  while (!list_end(iter)) {
    ++counter;
    if (LIST_DEBUG)
      printf("%s ", (char *)iter->data);
    iter = iter->next;
  }
  if (LIST_DEBUG)
    printf("\n");
  if (counter != list_size(list))
    failure("iter forward counted %lu, but had size %lu", counter,
            list_size(list));
}

void test_iter_backward(struct list *list) {
  struct list_node *iter = list_tail(list);
  size_t counter = 0;
  while (!list_end(iter)) {
    ++counter;
    if (LIST_DEBUG)
      printf("%s ", (char *)iter->data);
    iter = iter->prev;
  }
  if (LIST_DEBUG)
    printf("\n");
  if (counter != list_size(list))
    failure("iter backward counted %lu, but had size %lu", counter,
            list_size(list));
}
