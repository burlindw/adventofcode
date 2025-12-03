#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN (32)

typedef struct Iterator {
  char const *input;
  uint64_t min, max;
} Iterator;

bool next(Iterator *iter) {
  char buffer[BUFLEN];
  size_t length = 0;

  while (isdigit(*iter->input)) {
    buffer[length++] = *iter->input++;
    assert(length < BUFLEN);
  }

  if (length == 0) {
    return false;
  }

  buffer[length] = '\0';
  iter->min = strtoul(buffer, NULL, 10);

  assert(*iter->input == '-');
  iter->input++;

  length = 0;
  while (isdigit(*iter->input)) {
    buffer[length++] = *iter->input++;
    assert(length < BUFLEN);
  }

  if (length == 0) {
    return false;
  }

  buffer[length] = '\0';
  iter->max = strtoul(buffer, NULL, 10);
  assert(iter->min < iter->max);

  if (*iter->input == ',') {
    iter->input++;
  }

  return true;
}

typedef struct Node {
  uint64_t value;
  struct Node *left;
  struct Node *right;
} Node;

Node *tree = NULL;

bool insert(Node **tree, uint64_t value) {
  Node *parent = NULL;
  Node **slot = tree;

  while (*slot) {
    parent = *slot;
    if (value == parent->value) {
      return false;
    } else if (value < parent->value) {
      slot = &parent->left;
    } else {
      slot = &parent->right;
    }
  }

  *slot = malloc(sizeof(Node));
  **slot = (Node){value};

  return true;
}

void delete(Node **tree) {
  if (!tree || !*tree) {
    return;
  }

  delete (&(*tree)->left);
  delete (&(*tree)->right);
  free(*tree);
  *tree = NULL;
}

// Return the number of digits in `val`.
uint64_t length10(uint64_t val) {
  uint64_t len = 0;
  while (val != 0) {
    val /= 10;
    len += 1;
  }
  return len;
}

// Return 10 to the `exp` power.
uint64_t power10(uint64_t exp) {
  uint64_t val = 1;
  for (; exp != 0; exp--) {
    val *= 10;
  }
  return val;
}

// Shift `val` right by `shift` decimal digits.
uint64_t rshift10(uint64_t val, uint64_t shift) {
  for (uint64_t i = 0; i < shift; i++) {
    val /= 10;
  }
  return val;
}

// Repeat the digits of `rep` `count` times.
uint64_t repeat(uint64_t rep, uint64_t count) {
  uint64_t power = 1;
  while (power <= rep) {
    power *= 10;
  }

  uint64_t total = rep;
  for (uint64_t i = 1; i < count; i++) {
    total *= power;
    total += rep;
  }
  return total;
}

// Calculate the sum of all numbers that can be broken into `parts` segments
// of repeating digits in the (inclusive) range from `min` to `max`.
uint64_t range(uint64_t min, uint64_t max, size_t parts) {
  uint64_t minrep, maxrep;
  uint64_t i, j, value;
  uint64_t total = 0;
  uint64_t replen, shift;

  uint64_t minlen = length10(min);
  uint64_t maxlen = length10(max);

  if (minlen % parts == 0) {
    // Get the length of the repetition. For example, '123456' divided into
    // three `parts` has a repetition length of two.
    replen = minlen / parts;

    // We need to extract the `replen` most significant digits from minlen to
    // get our unit of repetition. In our earlier example, this is '12'.
    shift = minlen - replen;
    minrep = rshift10(min, shift);

    // We _might_ need to adjust the unit. If we actually repeated '12' three
    // times, we would get '121212'. This is less than our minimum, so we need
    // to bump it up to '13' and '131313'.
    if (repeat(minrep, parts) < min) {
      minrep++;
    }
  } else {
    // If the minlen doesn't divide nicely into the number of parts we're
    // looking for (e.g. '12345' cannot be split into three parts) then our unit
    // of repetion is '1[zeros]' where '[zeros]' is floor(len / parts). In our
    // example, the unit is '10' for a complete number of '101010'.
    minrep = power10(minlen / parts);
  }

  if (maxlen % parts == 0) {
    // This part works the same as the minimum length.
    replen = maxlen / parts;
    shift = maxlen - replen;
    maxrep = rshift10(max, shift);

    // However, instead of undershooting the minimum, we need to make sure we
    // aren't overshooting our maximum.
    if (repeat(maxrep, parts) > max) {
      maxrep--;
    }
  } else {
    // We also need to drop down to '[nines]' instead of up to '1[zeros]' to
    // find the unit. For example, if our max is '12345' and we need three
    // parts, then our unit is '9' and the complete number is '999'.
    maxrep = power10(maxlen / parts) - 1;
  }

  // Then we step through each possible unit of repetition, repeat it the
  // correct number of times, and add it to the total.
  //
  // An important caveat is that we cannot repeat numbers. So '222222', which is
  // either two '222's, three '22's, or six '2's, must only be counted once.
  for (i = minrep; i <= maxrep; i++) {
    value = repeat(i, parts);
    if (insert(&tree, value)) {
      total += value;
    }
  }

  return total;
}

void part1(char const *input) {
  Iterator iter = {input};
  uint64_t total = 0;

  while (next(&iter)) {
    total += range(iter.min, iter.max, 2);
  }

  printf("%" PRIu64 "\n", total);
  delete (&tree);
}

void part2(char const *input) {
  Iterator iter = {input};
  uint64_t maxlen, parts;
  uint64_t total = 0;

  while (next(&iter)) {
    maxlen = length10(iter.max);
    for (parts = 2; parts <= maxlen; parts++) {
      total += range(iter.min, iter.max, parts);
    }
  }

  printf("%" PRIu64 "\n", total);
  delete (&tree);
}
