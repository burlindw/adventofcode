#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct aoc_array {
  uint64_t *items;
  uint32_t count;
  uint32_t capacity;
} aoc_array;

void aoc_array_append(aoc_array *self, uint64_t const *vals, size_t count);

void aoc_array_push(aoc_array *self, uint64_t val);

bool aoc_array_pop(aoc_array *self, uint64_t *val);

bool aoc_array_top(aoc_array *self, uint64_t *val);

void aoc_array_sort(aoc_array *self);

void aoc_array_free(aoc_array *self);
