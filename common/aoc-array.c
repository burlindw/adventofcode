#include "aoc-array.h"

#include <stdlib.h>
#include <string.h>

static void aoc_array_ensure_capacity(aoc_array *self, size_t target) {
  size_t newcap, newsize;
  uint64_t *newitems;

  if (target > self->capacity) {
    newcap = self->capacity ? self->capacity : 16;
    for (; newcap < target; newcap *= 2) {}

    newsize = sizeof(uint64_t) * newcap;
    newitems = realloc(self->items, newsize);
    if (!newitems) abort();

    self->capacity = newcap;
    self->items = newitems;
  }
}

void aoc_array_append(aoc_array *self, uint64_t const *vals, size_t count) {
  aoc_array_ensure_capacity(self, self->count + count);
  memcpy(&self->items[self->count], vals, sizeof(uint64_t) * count);
  self->count += count;
}

void aoc_array_push(aoc_array *self, uint64_t val) {
  aoc_array_append(self, &val, 1);
}

bool aoc_array_pop(aoc_array *self, uint64_t *val) {
  if (self->count == 0) return false;
  *val = self->items[--self->count];
  return true;
}

bool aoc_array_top(aoc_array *self, uint64_t *val) {
  if (self->count == 0) return false;
  *val = self->items[self->count - 1];
  return true;
}

static void aoc_array_swapelems(aoc_array *self, size_t i, size_t j) {
  uint64_t temp = self->items[i];
  self->items[i] = self->items[j];
  self->items[j] = temp;
}

static void aoc_array_quicksort(aoc_array *self, size_t lo, size_t hi) {
  size_t i, j;
  uint64_t pivot;

  if (lo >= hi || hi >= self->count) return;

  pivot = self->items[hi];
  for (i = j = lo; j < hi; j++) {
    if (self->items[j] <= pivot) {
      aoc_array_swapelems(self, i, j);
      i++;
    }
  }
  aoc_array_swapelems(self, i, hi);

  aoc_array_quicksort(self, lo, i - 1);
  aoc_array_quicksort(self, i + 1, hi);
}

void aoc_array_sort(aoc_array *self) {
  if (self->count < 2) return;
  aoc_array_quicksort(self, 0, self->count - 1);
}

void aoc_array_free(aoc_array *self) {
  free(self->items);
  *self = (aoc_array){0};
}
