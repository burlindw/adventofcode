#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cairo/cairo.h>
#include <unistd.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct vec2 {
  uint64_t x;
  uint64_t y;
} vec2;

typedef struct vec2_list {
  vec2 *items;
  size_t count;
  size_t capacity;
} vec2_list;

typedef struct rect {
  uint64_t x0;
  uint64_t y0;
  uint64_t x1;
  uint64_t y1;
} rect;

typedef struct rect_list {
  rect *items;
  size_t count;
} rect_list;

void vec2_list_ensure_capacity(vec2_list *list, size_t target) {
  size_t capacity;
  vec2 *items;

  if (list->capacity < target) {
    capacity = list->capacity ? list->capacity : 16;
    for (; capacity < target; capacity *= 2) {}

    items = realloc(list->items, capacity * sizeof(*items));
    if (!items) abort();

    list->items = items;
    list->capacity = capacity;
  }
}

vec2_list vec2_list_new(char const *input) {
  size_t n;
  vec2 v;
  vec2_list list = {0};

  while (sscanf(input, "%lu,%lu\n%zn", &v.x, &v.y, &n) == 2) {
    vec2_list_ensure_capacity(&list, list.count + 1);
    list.items[list.count++] = v;
    input += n;
  }

  return list;
}

void vec2_list_free(vec2_list *list) {
  free(list->items);
  memset(list, 0, sizeof(*list));
}

void rect_swap(rect *a, rect *b) {
  rect temp = *a;
  *a = *b;
  *b = temp;
}

uint64_t rect_area(rect r) {
  return (r.x1 - r.x0 + 1) * (r.y1 - r.y0 + 1);
}

void rect_list_quicksort(rect_list *list, size_t lo, size_t hi) {
  size_t i, j;
  uint64_t pivot;

  if (lo >= hi || hi >= list->count) return;
  pivot = rect_area(list->items[hi]);

  for (i = j = lo; j < hi; j++) {
    if (rect_area(list->items[j]) >= pivot) {
      rect_swap(list->items + i, list->items + j);
      i++;
    }
  }
  rect_swap(list->items + i, list->items + hi);
  rect_list_quicksort(list, lo, i - 1);
  rect_list_quicksort(list, i + 1, hi);
}

rect_list rect_list_candidates(vec2_list const *vecs) {
  size_t i, j;
  rect_list rects;
  rect *r;

  rects.count = (vecs->count - 1) * vecs->count / 2;
  rects.items = malloc(rects.count * sizeof(*rects.items));
  r = rects.items;

  for (i = 0; i < vecs->count; i++) {
    for (j = i + 1; j < vecs->count; j++) {
      r->x0 = min(vecs->items[i].x, vecs->items[j].x);
      r->x1 = max(vecs->items[i].x, vecs->items[j].x);
      r->y0 = min(vecs->items[i].y, vecs->items[j].y);
      r->y1 = max(vecs->items[i].y, vecs->items[j].y);
      r++;
    }
  }

  rect_list_quicksort(&rects, 0, rects.count - 1);
  return rects;
}

rect_list rect_list_lines(vec2_list const *vecs) {
  size_t i;
  rect_list rects;
  rect *r;

  rects.count = vecs->count;
  rects.items = malloc(rects.count * sizeof(*rects.items));

  for (i = 1; i < vecs->count; i++) {
    rects.items[i].x0 = min(vecs->items[i].x, vecs->items[i - 1].x);
    rects.items[i].y0 = min(vecs->items[i].y, vecs->items[i - 1].y);
    rects.items[i].x1 = max(vecs->items[i].x, vecs->items[i - 1].x);
    rects.items[i].y1 = max(vecs->items[i].y, vecs->items[i - 1].y);
  }

  rects.items[0].x0 = min(vecs->items[0].x, vecs->items[i - 1].x);
  rects.items[0].y0 = min(vecs->items[0].y, vecs->items[i - 1].y);
  rects.items[0].x1 = max(vecs->items[0].x, vecs->items[i - 1].x);
  rects.items[0].y1 = max(vecs->items[0].y, vecs->items[i - 1].y);

  return rects;
}

void rect_list_free(rect_list *list) {
  free(list->items);
  memset(list, 0, sizeof(*list));
}

bool rect_overlaps(rect cand, rect line) {
  bool outx = cand.x1 <= line.x0 || line.x1 <= cand.x0;
  bool outy = cand.y1 <= line.y0 || line.y1 <= cand.y0;
  return !outx && !outy;
}

bool rect_validate(rect cand, rect_list const *lines) {
  rect *l = lines->items;
  rect *end = l + lines->count;

  for (; l < end; l++) {
    if (rect_overlaps(cand, *l)) return false;
  }

  return true;
}

void part1(char const *input) {
  vec2_list vecs = vec2_list_new(input);
  rect_list rects = rect_list_candidates(&vecs);
  uint64_t area = rect_area(rects.items[0]);

  printf("%" PRIu64 "\n", area);

  rect_list_free(&rects);
  vec2_list_free(&vecs);
}

void part2(char const *input) {
  vec2_list vecs = vec2_list_new(input);
  rect_list rects = rect_list_candidates(&vecs);
  rect_list lines = rect_list_lines(&vecs);
  uint64_t area;
  size_t r, l;

  for (r = 0; r < rects.count; r++) {
    if (rect_validate(rects.items[r], &lines)) break;
  }

  area = rect_area(rects.items[r]);
  printf("%" PRIu64 "\n", area);

  rect_list_free(&lines);
  rect_list_free(&rects);
  vec2_list_free(&vecs);
}
