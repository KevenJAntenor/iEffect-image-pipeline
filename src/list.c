/*
 * list.c - Source code for doubly linked circular list.
 *
 * Copyright (C) 2014 Andrew Schwartzmeyer
 *
 * This file released under the AGPLv3 license.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

static void list_debug(const char *format, ...);
static bool list_default_compare(void *a, void *b);

/*
 * Returns allocated list with uncounted sentinel element.
 */
struct list *list_new(bool (*compare)(void *a, void *b),
                      void (*delete_)(void *data))
{
	struct list *l = malloc(sizeof(*l));
	if (l == NULL) {
		perror("list_new()");
		return NULL;
	}

  /* A sentinel is used to eliminate edge cases everywhere else */
	struct list_node *sentinel = list_node_new(NULL);
	if (sentinel == NULL) {
		free(l);
		return NULL;
	}

	l->sentinel = sentinel;
	l->size = 0;
  l->max_size = 0;

	sentinel->sentinel = true;
	sentinel->next = sentinel;
	sentinel->prev = sentinel;

	l->compare = (compare == NULL)
		? &list_default_compare
		: compare;

	l->delete_ = delete_;

	return l;
}

/*
 * Inserts n at pos in O(n/2). Returns n if successful, else NULL.
 *
 * Position 0 inserts at the front and n inserts at the end in O(1).
 */
struct list_node *list_insert(struct list *self, int pos, struct list_node *n)
{
	if (self == NULL) {
		list_debug("list_insert(): self was NULL");
		return NULL;
	}

	n = list_node_link(n, list_index(self, pos));
  if (n) {
		++self->size;
    if (self->size > self->max_size) {
      self->max_size = self->size;
    }
  }
	return n;
}

/*
 * Use compare function to return found node, else NULL.
 */
struct list_node *list_search(struct list *self, void *data) {
	struct list_node *n = list_head(self);
	while (!list_end(n)) {
		if (self->compare(data, n->data))
			return n;
		n = n->next;
	}
	return NULL;
}

/*
 * Unlinks node from list at pos, returns node (to be freed).
 *
 * 0 is front, -1 (or n - 1), both are done in O(1). Else O(n/2).
 */
struct list_node *list_delete(struct list *self, int pos)
{
	if (self == NULL) {
		list_debug("list_delete(): self was NULL");
		return NULL;
	}

	struct list_node *n = list_node_unlink(list_index(self, pos));
	if (n)
		--self->size;
	return n;
}

/*
 * Pushes n to back of list in O(1).
 */
struct list_node *list_push_back(struct list *self, struct list_node *n)
{
	return list_insert(self, list_size(self), n);
}

/*
 * Pushes n to front of list in O(1).
 */
struct list_node *list_push_front(struct list *self, struct list_node *n)
{
	return list_insert(self, 0, n);
}

/*
 * Unlinks tail node of list in O(1). Returns node.
 */
struct list_node *list_pop_back(struct list *self)
{
	return list_delete(self, -1);
}

/*
 * Unlinks head node of list in O(1). Returns node.
 */
struct list_node *list_pop_front(struct list *self)
{
	return list_delete(self, 0);
}

/*
 * Returns pointer to data at tail of list in O(1).
 */
void *list_back(struct list *self)
{
	return list_tail(self)->data;
}

/*
 * Returns pointer to data at front of list in O(1).
 */
void *list_front(struct list *self)
{
	return list_head(self)->data;
}

/*
 * Returns pointer to head node of list in O(1).
 */
struct list_node *list_head(struct list *self)
{
	if (self == NULL) {
		list_debug("list_head(): self was NULL");
		return NULL;
	}

	if (!list_end(self->sentinel)) {
		list_debug("list_head(): sentinel was malformed");
		return NULL;
	}

	return self->sentinel->next;
}

/*
 * Returns pointer to tail node of list in O(1).
 */
struct list_node *list_tail(struct list *self)
{
	if (self == NULL) {
		list_debug("list_tail(): self was NULL");
		return NULL;
	}

	if (!list_end(self->sentinel)) {
		list_debug("list_tail(): sentinel was malformed");
		return NULL;
	}

	return self->sentinel->prev;
}

/*
 * Returns node at pos in O(n/2).
 *
 * Iterates from the closest end. Supports negative pos arguments.
 */
struct list_node *list_index(struct list *self, int pos)
{
	int s = list_size(self);

	/* handle negative positions */
	if (pos < 0)
		pos += s;

	struct list_node *n = NULL;


	if (pos <= s/2) {
		n = list_head(self);
		for (int i = 0; i < pos; ++i)
			n = n->next;
	} else {
		n = self->sentinel; /* for push_back */
		for (int i = s; i > pos; --i)
			n = n->prev;
	}

	return n;
}

/*
 * Returns the number of nodes in list. Does not count the sentinel.
 */
size_t list_size(struct list *self)
{
	if (self == NULL) {
		list_debug("list_size(): self was NULL");
		return 0;
	}

	return self->size;
}

/*
 * Helper to check if size is 0.
 */
bool list_empty(struct list *self)
{
	return (list_size(self) == 0);
}

/*
 * Returns true if n was the sentinel.
 *
 * This is an indication that an iteration has reached the end of the
 * list. *Not* the last data-carrying node of the list.
 */
bool list_end(struct list_node *n)
{
	if (n == NULL) {
		list_debug("list_end(): n was NULL");
		return false;
	}

	return n->sentinel;
}

/*
 * Concatenates list b to the end of list a destructively.
 *
 * usage: a = list_concat(a, b);
 *
 * List b (and its sentinel) will be freed. The nodes of list b will
 * therefore be accessible only through their new place in list a.
 *
 * Require that compare and delete functions are the same. Relative
 * assurance that lists contain same kinds of items.
 *
 * If either is an empty list, the other is returned. Hence the
 * recommended usage with assignment.
 *
 * Returns list a on success, NULL on failure.
 */
struct list *list_concat(struct list *a, struct list *b)
{
	if (a && !b)
		return a;
	if (b && !a)
		return b;

	if (!a && !b) {
		list_debug("list_concat(): no list given");
		return NULL;
	}

	if (a->compare != b->compare) {
		list_debug("list_concat(): compare functions unequal");
		return NULL;
	}

	if (a->delete_ != b->delete_) {
		list_debug("list_concat(): delete functions unequal");
		return NULL;
	}

	/* link head of b to tail of a */
	list_tail(a)->next = list_head(b);
	list_head(b)->prev = list_tail(a);

	/* link tail of b to sentinal of a */
	list_tail(b)->next = a->sentinel;
	a->sentinel->prev = list_tail(b);

	a->size += b->size;
  if (a->size > a->max_size)
    a->size = a->max_size;

	free(b->sentinel);
	free(b);

	return a;
}

/*
 * Use function to free data of each node, then free said node,
 * finally free the sentinel and the list.
 */
void list_free(struct list *self)
{
	while (!list_empty(self)) {
		struct list_node *n = list_pop_back(self);
		if (self->delete_)
			self->delete_(n->data);
		free(n);
	}

	free(self->sentinel);
	free(self);
}

/*
 * Default comparison for list of strings.
 */
static bool list_default_compare(void *a, void *b)
{
	return (strcmp((char *)a, (char *)b) == 0);
}

/*
 * Allocates new list_node with data.
 *
 * Sentinel flag is false. The next and prev pointers are null.
 */
struct list_node *list_node_new(void *data)
{
	struct list_node *n = malloc(sizeof(*n));
	if (n == NULL) {
		perror("list_node_new()");
		return NULL;
	}

	n->sentinel = false;
	n->next = NULL;
	n->prev = NULL;
	n->data = data;

	return n;
}

/*
 * Given (a _ c), links b (new) leaving (a b c) in O(1).
 *
 * Node a is found from c, so with b and c as parameters, this
 * prepends (think cons).
 *
 * Size is not incremented!
 */
struct list_node *list_node_link(struct list_node *b, struct list_node *c)
{
	if (b == NULL) {
		list_debug("list_node_link(): b was NULL");
		return NULL;
	}

	if (c == NULL) {
		list_debug("list_node_link(): c was NULL");
		return NULL;
	}

	struct list_node *a = c->prev;

	a->next = b;
	b->prev = a;
	b->next = c;
	c->prev = b;

	return b;
}

/*
 * Given (a b c), unlinks b leaving (a _ c) in O(1).
 *
 * Nodes a and c are found from b. Yay double links.
 *
 * Size is not decremented!
 */
struct list_node *list_node_unlink(struct list_node *b)
{
	if (list_end(b)) {
		return NULL;
	}

	struct list_node *a = b->prev;
	struct list_node *c = b->next;

	a->next = c;
	c->prev = a;

	return b;
}

static void list_debug(const char *format, ...)
{
	if (!LIST_DEBUG)
		return;

	va_list ap;
	va_start(ap, format);

	fprintf(stderr, "debug: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");

	va_end(ap);
}
