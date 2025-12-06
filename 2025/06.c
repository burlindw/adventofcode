#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define arrlen(array) (sizeof(array) / sizeof(*(array)))

typedef struct grid {
  char const *data;
  size_t stride;
  size_t height;
  size_t width;
} grid;

grid measure(char const *input) {
  grid g = {input};

  for (; *input != '\n'; input++) {
    g.width++;
  }
  g.stride = g.width + 1;

  for (; *input != '\0'; input++) {
    g.height += (*input == '\n');
  }

  return g;
}

char get(grid const *g, size_t x, size_t y) {
  if (x >= g->width || y >= g->height) return '\0';
  return g->data[x + y * g->stride];
}

void part1(char const *input) {
  uint64_t stack[20];
  uint64_t total = 0;
  size_t nstack = 0;
  size_t offset = 0;
  size_t line;
  char op;
  char const *text;

  grid g = measure(input);
  assert(g.height < arrlen(stack));

  while ((op = get(&g, offset, g.height - 1))) {
    // For each of the lines with numbers in them, parse the number and push it
    // onto a stack for later.
    for (line = 0; line < g.height - 1; line++) {
      text = &g.data[offset + line * g.stride];
      stack[nstack++] = strtoul(text, NULL, 10);
    }

    // Once we have all of the numbers parsed, accumulate everything in the
    // stack together and add it to the grand total. Then reset the stack.
    if (op == '+') {
      while (nstack > 1) {
        stack[0] += stack[--nstack];
      }
    } else {
      assert(op == '*');
      while (nstack > 1) {
        stack[0] *= stack[--nstack];
      }
    }
    total += stack[0];
    nstack = 0;

    // Scan across the last line to find the next operator.
    for (offset++; isspace(get(&g, offset, g.height - 1)); offset++) {}
  }

  printf("%" PRIu64 "\n", total);
}

void part2(char const *input) {
  uint64_t stack[20];
  uint64_t total = 0;
  size_t nstack = 0;
  size_t row, col;
  char c;

  grid g = measure(input);
  assert(g.height < arrlen(stack));

  for (col = g.width - 1; col < g.width; col--) {
    // Scan down the rows, building up the current slot in the stack with the
    // digits as we find them.
    stack[nstack] = 0;
    for (row = 0; row < g.height - 1; row++) {
      c = get(&g, col, row);
      if ('0' <= c && c <= '9') {
        stack[nstack] *= 10;
        stack[nstack] += c - '0';
      }
    }

    // If this wasn't an empty column, move onto the next slot in the stack.
    if (stack[nstack] != 0) nstack++;

    // Check if the last row has an operator in it. If it does, then we
    // accumulate everything in the stack and add it to the grand total, before
    // resetting the stack.
    switch (get(&g, col, g.height - 1)) {
      case '+':
        while (nstack > 1) {
          stack[0] += stack[--nstack];
        }
        total += stack[0];
        nstack = 0;
        break;

      case '*':
        while (nstack > 1) {
          stack[0] *= stack[--nstack];
        }
        total += stack[0];
        nstack = 0;
        break;

      default:
        break;
    }
  }

  printf("%" PRIu64 "\n", total);
}
