#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIDS (26 * 26 * 26)

typedef struct iterator {
  char const *input;
} iterator;

uint16_t id_from_text(char const text[3]) {
  uint16_t id = 0;
  assert('a' <= text[0] && text[0] <= 'z');
  assert('a' <= text[1] && text[1] <= 'z');
  assert('a' <= text[2] && text[2] <= 'z');
  id += text[0] - 'a';
  id *= 26;
  id += text[1] - 'a';
  id *= 26;
  id += text[2] - 'a';
  return id;
}

bool nextsrc(iterator *it, uint16_t *id) {
  while (isspace(*it->input)) {
    it->input++;
  }

  if (*it->input < 'a' || 'z' < *it->input) {
    return false;
  }

  *id = id_from_text(it->input);
  it->input += 3;

  assert(*it->input == ':');
  it->input++;

  return true;
}

bool nextdst(iterator *it, uint16_t *id) {
  while (*it->input == ' ') {
    it->input++;
  }

  if (*it->input < 'a' || 'z' < *it->input) {
    return false;
  }

  *id = id_from_text(it->input);
  it->input += 3;

  return true;
}

typedef struct slice {
  size_t begin;
  size_t end;
} slice;

typedef struct graph {
  slice *slices;
  uint16_t *dsts;
  size_t count;
  size_t capacity;
} graph;

void graph_init(graph *self) {
  memset(self, 0, sizeof(*self));

  self->slices = malloc(MAXIDS * sizeof(slice));
  memset(self->slices, 0xff, MAXIDS * sizeof(slice));
}

void graph_free(graph *self) {
  free(self->slices);
  free(self->dsts);
  memset(self, 0, sizeof(*self));
}

void graph_ensure_capacity(graph *self, size_t target) {
  size_t capacity;
  uint16_t *items;

  if (self->capacity < target) {
    for (capacity = 1; capacity < target; capacity *= 2) {}

    items = realloc(self->dsts, capacity * sizeof(*items));
    if (!items) abort();

    self->dsts = items;
    self->capacity = capacity;
  }
}

void graph_new_src(graph *self, uint16_t src) {
  assert(self->slices[src].begin == SIZE_MAX);
  self->slices[src].begin = self->count;
  self->slices[src].end = self->count;
}

void graph_add_dst(graph *self, uint16_t src, uint16_t dst) {
  assert(self->slices[src].end == self->count);
  graph_ensure_capacity(self, self->count + 1);

  self->dsts[self->count++] = dst;
  self->slices[src].end++;
}

void graph_populate(graph *self, char const *input) {
  iterator it = {input};
  uint16_t src, dst;

  while (nextsrc(&it, &src)) {
    graph_new_src(self, src);
    while (nextdst(&it, &dst)) {
      graph_add_dst(self, src, dst);
    }
  }
}

typedef struct frame {
  uint16_t src;
  slice dsts;
} frame;

uint64_t graph_paths(graph *g, char const *origin, char const *target) {
  frame *stack, top;
  size_t nstack;
  uint64_t *paths, total;
  uint16_t oid = id_from_text(origin);
  uint16_t tid = id_from_text(target);

  paths = malloc(MAXIDS * sizeof(*paths));
  memset(paths, 0xff, MAXIDS * sizeof(*paths));
  paths[tid] = 1;
  paths[oid] = 0;

  stack = malloc(MAXIDS * sizeof(*stack));
  stack[0].src = oid;
  stack[0].dsts = g->slices[oid];
  nstack = 1;

  while (nstack != 0) {
    top = stack[nstack - 1];

    if (top.dsts.begin == top.dsts.end) {
      // If there are no more destinations for the current node, pop it.
      nstack--;
    } else if (paths[g->dsts[top.dsts.begin]] == UINT64_MAX) {
      // If the current destination HAS NOT been visited, visit it.
      paths[g->dsts[top.dsts.begin]] = 0;
      stack[nstack].src = g->dsts[top.dsts.begin];
      stack[nstack].dsts = g->slices[g->dsts[top.dsts.begin]];
      nstack++;
    } else {
      // If the current destination HAS been visited, add its number of paths to
      // the current node's, then move on to the next destination.
      paths[top.src] += paths[g->dsts[top.dsts.begin]];
      stack[nstack - 1].dsts.begin++;
    }
  }

  total = paths[oid];
  free(stack);
  free(paths);
  return total;
}

void part1(char const *input) {
  graph g;
  uint64_t total;

  graph_init(&g);
  graph_populate(&g, input);

  total = graph_paths(&g, "you", "out");
  printf("%" PRIu64 "\n", total);

  graph_free(&g);
}

void part2(char const *input) {
  graph g;
  uint64_t total;
  uint64_t svr_fst, fst_snd, snd_out;

  graph_init(&g);
  graph_populate(&g, input);

  // The graph is acyclic, which means that the "in any order" hint is a bit of
  // a misdirection. Data can only flow in one direction. We try going from fft
  // to dac first; if it wasn't able to find a path, then we know the data has
  // to flow from dac to fft instead.
  fst_snd = graph_paths(&g, "fft", "dac");
  if (fst_snd == 0) {
    fst_snd = graph_paths(&g, "dac", "fft");
    svr_fst = graph_paths(&g, "svr", "dac");
    snd_out = graph_paths(&g, "fft", "out");
  } else {
    svr_fst = graph_paths(&g, "svr", "fft");
    snd_out = graph_paths(&g, "dac", "out");
  }

  // The requirement of going through the dac and fft nodes places "chokepoints"
  // in the paths. Assume data flows from fft to dac.
  //
  // Every path between svr and fft will have the option to take any path
  // between fft and dac afterward. Therefore, we can get the total number of
  // paths from svr to dac that pass through fft by multiplying the number of
  // paths from svr to fft by the number of paths from fft to dac.
  //
  // Once we have the number of paths from svr to dac, we can get apply our
  // earlier logic again to go from dac to out.

  total = svr_fst * fst_snd * snd_out;
  printf("%" PRIu64 "\n", total);

  graph_free(&g);
}
