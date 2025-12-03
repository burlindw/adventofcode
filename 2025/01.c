#include <assert.h>
#include <stdio.h>

void part1(char const *input) {
  size_t skip, zeros = 0;
  int clicks, dial = 50;
  char dir;

  while (sscanf(input, "%c%d\n%zn", &dir, &clicks, &skip) == 2) {
    assert(0 <= dial && dial < 100);
    assert(dir == 'L' || dir == 'R');

    dial += (dir == 'L') ? -clicks : clicks;
    dial %= 100;

    if (dial < 0) {
      dial += 100;
    }
    zeros += (dial == 0);
    input += skip;
  }

  printf("%zu\n", zeros);
}

void part2(char const *input) {
  size_t skip, zeros = 0;
  int clicks, dial = 50;
  char dir;

  while (sscanf(input, "%c%d\n%zn", &dir, &clicks, &skip) == 2) {
    assert(0 <= dial && dial < 100);
    assert(dir == 'L' || dir == 'R');

    zeros += clicks / 100;
    clicks %= 100;

    if (dir == 'L') {
      zeros += (dial > 0 && dial - clicks <= 0);
      dial -= clicks;
      if (dial < 0) {
        dial = 100 + dial;
      }
    } else {
      dial += clicks;
      if (dial >= 100) {
        zeros++;
        dial -= 100;
      }
    }

    input += skip;
  }

  printf("%zu\n", zeros);
}
