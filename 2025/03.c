#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Iterator {
  char const *input;
  char const *line;
  size_t length;
} Iterator;

bool next(Iterator *iter) {
  iter->line = iter->input;
  while (isdigit(*iter->input)) {
    iter->input++;
  }

  if (iter->line == iter->input) {
    return false;
  }

  iter->length = iter->input - iter->line;

  while (isspace(*iter->input)) {
    iter->input++;
  }

  return true;
}

// Scan the input line for the first occurence of the highest digit in the
// provided range. Return the index of the digit.
size_t maxchar(char const *line, size_t minidx, size_t maxidx) {
  size_t i, maxpos = minidx;
  char maxval = line[minidx];

  for (i = minidx; i < maxidx; i++) {
    if (line[i] > maxval) {
      maxval = line[i];
      maxpos = i;
    }
  }

  return maxpos;
}

// Find the highest possible number with `count` digits in the line.
uint64_t maxvalue(char const *line, size_t length, size_t count) {
  uint64_t value = 0;
  size_t i, pos = 0;

  assert(length >= count);

  for (i = 1; i <= count; i++) {
    // Find the highest digit in the line, but exclude enough of the end to
    // ensure that we have enough digits left to finish the value.
    pos = maxchar(line, pos, length - count + i);

    // Add the digit to the final value.
    value *= 10;
    value += line[pos] - '0';

    // Step past the digit we just found to ensure that it isn't included
    // multiple times.
    pos++;
  }

  return value;
}

void part1(char const *input) {
  Iterator iter = {input};
  uint64_t total = 0;

  while (next(&iter)) {
    total += maxvalue(iter.line, iter.length, 2);
  }

  printf("%" PRIu64 "\n", total);
}

void part2(char const *input) {
  Iterator iter = {input};
  uint64_t total = 0;

  while (next(&iter)) {
    total += maxvalue(iter.line, iter.length, 12);
  }

  printf("%" PRIu64 "\n", total);
}
