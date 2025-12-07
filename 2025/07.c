#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct grid {
  size_t width;
  size_t height;
  size_t stride;
  size_t startx;
} bounds;

bounds measure(char const *data) {
  bounds g = {0};

  while (data[g.startx] != 'S') {
    g.startx++;
    g.width++;
  }

  while (data[g.width] != '\n') {
    g.width++;
  }
  g.stride = g.width + 1;

  for (; *data != '\0'; data++) {
    g.height += (*data == '\n');
  }

  return g;
}

void part1(char const *input) {
  bounds b = measure(input);
  size_t x, y;
  uint64_t total = 0;
  bool *swap, *beams[2];

  beams[0] = calloc(b.width, sizeof(bool));
  beams[1] = calloc(b.width, sizeof(bool));
  if (!beams[0] || !beams[1]) abort();

  beams[0][b.startx] = true;
  for (y = 1; y < b.height; y++) {
    memset(beams[1], 0, b.width * sizeof(bool));

    for (x = 0; x < b.width; x++) {
      if (input[x + y * b.stride] == '^') {
        assert(x > 0);
        assert(x < b.width - 1);

        beams[1][x - 1] |= beams[0][x];
        beams[1][x + 1] |= beams[0][x];
        total += beams[0][x];
      } else {
        beams[1][x] |= beams[0][x];
      }
    }

    swap = beams[0];
    beams[0] = beams[1];
    beams[1] = swap;
  }

  printf("%" PRIu64 "\n", total);
  free(beams[0]);
  free(beams[1]);
}

void part2(char const *input) {
  bounds b = measure(input);
  size_t x, y;
  uint64_t total = 0;
  uint64_t *swap, *beams[2];

  beams[0] = calloc(b.width, sizeof(uint64_t));
  beams[1] = calloc(b.width, sizeof(uint64_t));
  if (!beams[0] || !beams[1]) abort();

  beams[0][b.startx] = 1;
  for (y = 1; y < b.height; y++) {
    memset(beams[1], 0, b.width * sizeof(uint64_t));

    for (x = 0; x < b.width; x++) {
      if (input[x + y * b.stride] == '^') {
        assert(x > 0);
        assert(x < b.width - 1);
        
        beams[1][x - 1] += beams[0][x];
        beams[1][x + 1] += beams[0][x];
      } else {
        beams[1][x] += beams[0][x];
      }
    }

    swap = beams[0];
    beams[0] = beams[1];
    beams[1] = swap;
  }

  for (x = 0; x < b.width; x++) {
    total += beams[0][x];
  }

  printf("%" PRIu64 "\n", total);
  free(beams[0]);
  free(beams[1]);
}
