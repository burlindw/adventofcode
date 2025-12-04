#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Grid {
  char *data;
  size_t width;
  size_t height;
  size_t stride;
} Grid;

Grid measure(char *input) {
  Grid grid = {input};

  for (; !isspace(*input); input++) {
    grid.width++;
    grid.stride++;
  }

  for (; isspace(*input); input++) {
    grid.stride++;
  }

  grid.height = 1;
  for (; *input != 0; input++) {
    grid.height += *input == '\n';
  }

  return grid;
}

char get(Grid *grid, size_t x, size_t y) {
  if (x >= grid->width || y >= grid->height) {
    return '\0';
  }

  return grid->data[x + y * grid->stride];
}

bool accessible(Grid *grid, size_t x, size_t y) {
  if (get(grid, x, y) != '@') {
    return false;
  }

  size_t nearby = 0;
  nearby += ('@' == get(grid, x - 1, y - 1));
  nearby += ('@' == get(grid, x - 1, y));
  nearby += ('@' == get(grid, x - 1, y + 1));
  nearby += ('@' == get(grid, x + 1, y - 1));
  nearby += ('@' == get(grid, x + 1, y));
  nearby += ('@' == get(grid, x + 1, y + 1));
  nearby += ('@' == get(grid, x, y - 1));
  nearby += ('@' == get(grid, x, y + 1));
  return nearby < 4;
}

void part1(char const *input) {
  Grid grid = measure(strdup(input));
  uint64_t total = 0;
  size_t x, y;

  for (y = 0; y < grid.height; y++) {
    for (x = 0; x < grid.width; x++) {
      total += accessible(&grid, x, y);
    }
  }

  printf("%" PRIu64 "\n", total);
  free(grid.data);
}

void part2(char const *input) {
  Grid active, source;
  uint64_t removed, total = 0;
  size_t x, y;

  active = measure(strdup(input));
  source = measure(strdup(input));

  do {
    removed = 0;
    for (y = 0; y < source.height; y++) {
      for (x = 0; x < source.width; x++) {
        if (accessible(&source, x, y)) {
          active.data[x + y * active.stride] = '.';
          removed++;
        }
      }
    }
    total += removed;
    strcpy(source.data, active.data);
  } while (removed != 0);

  printf("%" PRIu64 "\n", total);
  free(active.data);
  free(source.data);
}
