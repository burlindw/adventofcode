#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aoc-array.h>

typedef struct vec3 {
  uint32_t x;
  uint32_t y;
  uint32_t z;
} vec3;

typedef struct vec3_list {
  vec3 *items;
  size_t count;
  size_t capacity;
} vec3_list;

typedef struct vec3_pair {
  uint64_t distance;
  uint32_t lid;
  uint32_t rid;
} vec3_pair;

typedef struct vec3_pair_list {
  vec3_pair *items;
  size_t count;
} vec3_pair_list;

uint64_t vec3_distance(vec3 a, vec3 b) {
  uint64_t dx = (a.x > b.x) ? (a.x - b.x) : (b.x - a.x);
  uint64_t dy = (a.y > b.y) ? (a.y - b.y) : (b.y - a.y);
  uint64_t dz = (a.z > b.z) ? (a.z - b.z) : (b.z - a.z);
  return dx * dx + dy * dy + dz * dz;
}

void vec3_list_ensure_capacity(vec3_list *list, size_t target) {
  size_t capacity;
  vec3 *items;

  if (list->capacity < target) {
    capacity = list->capacity ? list->capacity : 16;
    for (; capacity < target; capacity *= 2) {}

    items = realloc(list->items, capacity * sizeof(vec3));
    if (!items) abort();

    list->items = items;
    list->capacity = capacity;
  }
}

void vec3_list_append(vec3_list *list, vec3 item) {
  vec3_list_ensure_capacity(list, list->count + 1);
  list->items[list->count++] = item;
}

void vec3_list_free(vec3_list *list) {
  free(list->items);
  memset(list, 0, sizeof(vec3_list));
}

vec3_list vec3_list_new(char const *input) {
  vec3 v;
  size_t n;
  vec3_list list = {0};

  while (sscanf(input, "%u,%u,%u\n%zn", &v.x, &v.y, &v.z, &n) == 3) {
    vec3_list_append(&list, v);
    input += n;
  }

  return list;
}

vec3_pair_list vec3_pair_list_new(vec3_list const *vecs) {
  vec3_pair_list pairs = {0};
  vec3_pair *pair;
  size_t lid, rid;

  pairs.count = ((vecs->count - 1) * vecs->count) / 2;
  pairs.items = malloc(pairs.count * sizeof(*pairs.items));
  pair = pairs.items;

  for (lid = 0; lid < vecs->count; lid++) {
    for (rid = lid + 1; rid < vecs->count; rid++) {
      pair->distance = vec3_distance(vecs->items[lid], vecs->items[rid]);
      pair->lid = lid;
      pair->rid = rid;
      pair++;
    }
  }

  return pairs;
}

void vec3_pair_swap(vec3_pair *a, vec3_pair *b) {
  vec3_pair temp = *a;
  *a = *b;
  *b = temp;
}

void vec3_pair_list_quicksort(vec3_pair_list *list, size_t lo, size_t hi) {
  size_t i, j;
  uint64_t pivot;

  if (lo >= hi || hi > list->count) return;

  pivot = list->items[hi].distance;

  for (i = j = lo; j < hi; j++) {
    if (list->items[j].distance <= pivot) {
      vec3_pair_swap(list->items + j, list->items + i);
      i++;
    }
  }
  vec3_pair_swap(list->items + i, list->items + hi);

  vec3_pair_list_quicksort(list, lo, i - 1);
  vec3_pair_list_quicksort(list, i + 1, hi);
}

void vec3_pair_list_sort(vec3_pair_list *list) {
  if (list->count == 0) return;
  vec3_pair_list_quicksort(list, 0, list->count - 1);
}

void vec3_pair_list_free(vec3_pair_list *list) {
  free(list->items);
  memset(list, 0, sizeof(vec3_pair_list));
}

bool all(bool const *conds, size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    if (!conds[i]) return false;
  }
  return true;
}

void part1(char const *input) {
  size_t i, j;
  uint32_t old, new;
  uint64_t total = 1;
  vec3_pair *pair;
  vec3_list vecs = vec3_list_new(input);
  vec3_pair_list pairs = vec3_pair_list_new(&vecs);
  aoc_array circuits = {0};
  uint32_t *assign = malloc(vecs.count * sizeof(*assign));

  for (i = 0; i < vecs.count; i++) {
    aoc_array_push(&circuits, 1);
    assign[i] = i;
  }

  vec3_pair_list_sort(&pairs);
  for (i = 0; i < 1000; i++) {
    pair = pairs.items + i;

    // Whenever the two vectors are not assigned to the same circuit, update
    // everything that is currently assigned to the right circuit so that it
    // belongs to the left circuit instead.
    if (assign[pair->lid] != assign[pair->rid]) {
      old = assign[pair->rid];
      new = assign[pair->lid];

      for (j = 0; j < vecs.count; j++) {
        if (old == assign[j]) {
          assign[j] = new;
        }
      }
      circuits.items[new] += circuits.items[old];
      circuits.items[old] = 0;
    }
  }

  aoc_array_sort(&circuits);
  for (i = 1; i <= 3; i++) {
    total *= circuits.items[circuits.count - i];
  }
  printf("%" PRIu64 "\n", total);

  free(assign);
  aoc_array_free(&circuits);
  vec3_pair_list_free(&pairs);
  vec3_list_free(&vecs);
}

void part2(char const *input) {
  size_t i, j;
  uint32_t old, new;
  uint64_t total = 1;
  vec3_pair *pair;
  vec3_list vecs = vec3_list_new(input);
  vec3_pair_list pairs = vec3_pair_list_new(&vecs);

  uint32_t ncircuits = vecs.count;
  uint32_t *assign = malloc(vecs.count * sizeof(*assign));
  for (i = 0; i < vecs.count; i++) {
    assign[i] = i;
  }

  vec3_pair_list_sort(&pairs);
  for (i = 0; i < pairs.count; i++) {
    pair = pairs.items + i;

    if (assign[pair->lid] != assign[pair->rid]) {
      old = assign[pair->rid];
      new = assign[pair->lid];
      for (j = 0; j < vecs.count; j++) {
        if (old == assign[j]) {
          assign[j] = new;
        }
      }
      ncircuits--;
    }

    if (ncircuits == 1) break;
  }

  total *= vecs.items[pair->lid].x;
  total *= vecs.items[pair->rid].x;
  printf("%" PRIu64 "\n", total);

  free(assign);
  vec3_pair_list_free(&pairs);
  vec3_list_free(&vecs);
}
