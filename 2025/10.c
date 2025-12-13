#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aoc-array.h>

#define arrlen(array) (sizeof(array) / sizeof((array)[0]))

#define MAX_TARGETS (10)
#define MAX_BUTTONS (20)
#define MAX_CONDITIONS (20)
#define MAX_VARIABLES (40)

typedef struct iterator {
  char const *input;
  uint16_t targets;
  uint16_t indicators;
  uint16_t joltages[MAX_TARGETS];
  uint16_t nbuttons;
  uint16_t buttons[MAX_BUTTONS];
} iterator;

bool next(iterator *iter) {
  uint16_t mask;
  uint64_t value;
  char *end;

  iter->targets = 0;
  iter->indicators = 0;
  iter->nbuttons = 0;

  while (isspace(*iter->input)) {
    iter->input++;
  }

  if (*iter->input != '[') {
    return false;
  }
  iter->input++;

  while (*iter->input != ']') {
    if (*iter->input == '#') {
      mask = 1 << iter->targets;
      iter->indicators |= mask;
    }

    iter->input++;
  }

  assert(*iter->input == ']');
  iter->input++;

  assert(*iter->input == ' ');
  iter->input++;

  while (*iter->input == '(') {
    iter->input++;

    assert(iter->nbuttons < arrlen(iter->buttons));
    iter->buttons[iter->nbuttons] = 0;

    while (*iter->input != ')') {
      assert(*iter->input >= '0');
      assert(*iter->input <= '9');

      mask = 1 << (*iter->input - '0');
      iter->buttons[iter->nbuttons] |= mask;
      iter->input++;

      if (*iter->input == ',') {
        iter->input++;
      }
    }
    iter->nbuttons++;

    assert(*iter->input == ')');
    iter->input++;

    assert(*iter->input == ' ');
    iter->input++;
  }

  assert(*iter->input == '{');
  iter->input++;

  while (*iter->input != '}') {
    value = strtoul(iter->input, &end, 10);
    iter->joltages[iter->targets++] = value;

    iter->input = end;
    if (*iter->input == ',') {
      iter->input++;
    }
  }

  assert(*iter->input == '}');
  iter->input++;

  return true;
}

uint64_t integer_gcd(uint64_t a, uint64_t b) {
  if (a == 0) return b;
  return integer_gcd(b % a, a);
}

uint64_t integer_lcm(uint64_t a, uint64_t b) {
  return a / integer_gcd(a, b) * b;
}

typedef struct rational {
  int32_t num;
  int32_t den;
} rational;

void rational_simplify(rational *value) {
  int32_t sign;
  uint32_t num, den, gcd;

  if (value->den < 0) {
    value->num = -value->num;
    value->den = -value->den;
  }

  sign = (value->num > 0) - (value->num < 0);
  num = value->num * sign;
  den = value->den;

  gcd = integer_gcd(num, den);
  num /= gcd;
  den /= gcd;

  value->num = ((int32_t)num) * sign;
  value->den = ((int32_t)den);
}

bool rational_positive(rational value) {
  assert(value.den > 0);
  return value.num > 0;
}

bool rational_negative(rational value) {
  assert(value.den > 0);
  return value.num < 0;
}

rational rational_fpart(rational value) {
  int32_t n = value.num % value.den;
  value.num = n < 0 ? n + value.den : n;
  return value;
}

rational rational_ipart(rational value) {
  int32_t m = value.num % value.den;
  value.num /= value.den;
  value.num -= (m != 0 && ((m < 0) != (value.den < 0)));
  value.den = 1;
  return value;
}

int rational_cmp(rational lhs, rational rhs) {
  if (rational_negative(lhs)) {
    if (!rational_negative(rhs)) {
      return -1;
    }
    lhs.num = -lhs.num;
    rhs.num = -rhs.num;
    return rational_cmp(rhs, lhs);
  }

  if (rational_negative(rhs)) {
    return 1;
  }

  uint64_t ln, rn, lcm;
  lcm = integer_lcm(lhs.den, rhs.den);
  ln = (lcm / lhs.den) * lhs.num;
  rn = (lcm / rhs.den) * rhs.num;

  return (ln > rn) - (ln < rn);
}

bool rational_gt(rational lhs, rational rhs) {
  return rational_cmp(lhs, rhs) > 0;
}

bool rational_ge(rational lhs, rational rhs) {
  return rational_cmp(lhs, rhs) >= 0;
}

bool rational_lt(rational lhs, rational rhs) {
  return rational_cmp(lhs, rhs) < 0;
}

bool rational_le(rational lhs, rational rhs) {
  return rational_cmp(lhs, rhs) <= 0;
}

rational rational_neg(rational value) {
  value.num = -value.num;
  return value;
}

rational rational_reciprocal(rational value) {
  int32_t temp = value.den;
  value.den = value.num;
  value.num = temp;

  assert(value.den != 0);
  return value;
}

void rational_addeq(rational *dst, rational src) {
  uint64_t lcm;

  assert(dst->den > 0 && src.den > 0);
  lcm = integer_lcm(dst->den, src.den);
  dst->num *= (lcm / dst->den);
  src.num *= (lcm / src.den);

  dst->num += src.num;
  dst->den = lcm;
  rational_simplify(dst);
}

void rational_subeq(rational *dst, rational src) {
  src.num = -src.num;
  rational_addeq(dst, src);
}

void rational_muleq(rational *dst, rational src) {
  dst->num *= src.num;
  dst->den *= src.den;
  rational_simplify(dst);
}

rational rational_mul(rational lhs, rational rhs) {
  rational_muleq(&lhs, rhs);
  return lhs;
}

void rational_diveq(rational *dst, rational src) {
  dst->num *= src.den;
  dst->den *= src.num;
  rational_simplify(dst);
}

rational rational_div(rational lhs, rational rhs) {
  rational_diveq(&lhs, rhs);
  return lhs;
}

typedef struct linprog {
  rational coeff[MAX_CONDITIONS][MAX_VARIABLES];
  uint32_t conditions;
  uint32_t variables;
} linprog;

void linprog_dump(linprog *lp) {
  int w, widths[MAX_VARIABLES] = {0};
  char buffer[50];
  size_t col, row;
  rational r;

  for (col = 0; col <= lp->variables; col++) {
    for (row = 0; row <= lp->conditions; row++) {
      r = lp->coeff[row][col];
      w = snprintf(NULL, 0, "%i/%i", r.num, r.den);
      if (widths[col] < w) widths[col] = w;
    }
  }

  for (col = 0; col < lp->variables; col++) {
    printf(" %*zu |", widths[col], col);
  }
  printf(" RHS\n");

  for (row = 0; row <= lp->conditions; row++) {
    for (col = 0; col <= lp->variables; col++) {
      if (col) printf("|");
      r = lp->coeff[row][col];
      snprintf(buffer, sizeof(buffer), "%i/%i", r.num, r.den);
      printf(" %*s ", widths[col], buffer);
    }
    printf("\n");
  }
  printf("\n");
}

void linprog_ero_scale(linprog *lp, size_t row, rational scalar) {
  size_t col;
  for (col = 0; col <= lp->variables; col++) {
    rational_muleq(&lp->coeff[row][col], scalar);
  }
}

void linprog_ero_add(linprog *lp, size_t src, size_t dst, rational scalar) {
  size_t col;
  rational temp;

  for (col = 0; col <= lp->variables; col++) {
    temp = rational_mul(lp->coeff[src][col], scalar);
    rational_addeq(&lp->coeff[dst][col], temp);
  }
}

void linprog_new_constraint(linprog *lp) {
  size_t row, col;
  rational *src, *dst;
  rational zero = {0, 1};

  assert(lp->conditions + 2 < MAX_CONDITIONS);
  assert(lp->variables + 2 < MAX_VARIABLES);

  for (row = 0; row <= lp->conditions; row++) {
    dst = &lp->coeff[row][lp->variables + 1];
    src = &lp->coeff[row][lp->variables];
    *dst = *src;
    *src = zero;
  }
  lp->variables++;

  for (col = 0; col <= lp->variables; col++) {
    dst = &lp->coeff[lp->conditions + 1][col];
    src = &lp->coeff[lp->conditions][col];
    *dst = *src;
    *src = zero;
  }
  lp->conditions++;
}

void linprog_pivot(linprog *lp, size_t col, size_t row) {
  size_t y;
  rational scalar;

  // Ensure that the pivot cell is 1 by dividing the entire pivot row by the
  // value in the pivot cell.
  scalar = rational_reciprocal(lp->coeff[row][col]);
  linprog_ero_scale(lp, row, scalar);

  for (y = 0; y <= lp->conditions; y++) {
    if (y == row) continue;

    scalar = rational_neg(lp->coeff[y][col]);
    linprog_ero_add(lp, row, y, scalar);
  }
}

bool linprog_optimal(linprog *lp) {
  size_t x;
  rational objective;

  // Our linear program is optimal when every coefficient in the objective
  // function except the constant is <= 0.
  for (x = 0; x < lp->variables; x++) {
    objective = lp->coeff[lp->conditions][x];
    if (rational_positive(objective)) return false;
  }

  return true;
}

size_t linprog_select_pivot_col(linprog *lp) {
  size_t col, best_index = 0;
  rational *objective = lp->coeff[lp->conditions];
  rational best_value = objective[best_index];

  for (col = 1; col < lp->variables; col++) {
    if (rational_lt(best_value, objective[col])) {
      best_value = objective[col];
      best_index = col;
    }
  }

  return best_index;
}

size_t linprog_select_pivot_row(linprog *lp, size_t col) {
  size_t y, best;
  rational best_value = {INT32_MAX, 1};
  rational value;

  for (y = 0; y < lp->conditions; y++) {
    if (!rational_positive(lp->coeff[y][col])) continue;
    value = rational_div(lp->coeff[y][lp->variables], lp->coeff[y][col]);

    if (rational_lt(value, best_value)) {
      best_value = value;
      best = y;
    }
  }

  assert(best_value.num != INT32_MAX);

  return best;
}

size_t linprog_select_cutting_row(linprog *lp) {
  size_t y;
  rational constant;

  for (y = 0; y < lp->conditions; y++) {
    constant = lp->coeff[y][lp->variables];
    if (constant.den != 1) break;
  }

  return y;
}

size_t linprog_select_cutting_col(linprog *lp, size_t row) {
  size_t col, bestidx;
  rational val, bestval;
  rational *objective = lp->coeff[lp->conditions];

  for (bestidx = 0; bestidx < lp->variables; bestidx++) {
    if (rational_negative(lp->coeff[row][bestidx])) break;
  }

  assert(bestidx != lp->variables);
  bestval = rational_div(objective[bestidx], lp->coeff[row][bestidx]);

  for (col = bestidx + 1; col < lp->variables; col++) {
    if (!rational_negative(lp->coeff[row][col])) continue;
    val = rational_div(objective[col], lp->coeff[row][col]);

    if (rational_lt(val, bestval)) {
      bestval = val;
      bestidx = col;
    }
  }

  return bestidx;
}

uint64_t linprog_solve(linprog *lp) {
  size_t row, col, newrow, newcol;
  rational r;

  // Find the rational solution using the Big-M algorithm to handle the precise
  // equalities needed. https://www.youtube.com/watch?v=Y7MM8IbReMY
  while (!linprog_optimal(lp)) {
    col = linprog_select_pivot_col(lp);
    row = linprog_select_pivot_row(lp, col);
    linprog_pivot(lp, col, row);
  }

  // The first algorithm only works for rational linear programs. It might have
  // produced an integer solution by happenstance, but in case it didn't we use
  // the Cutting-Plane algorithm to get the integer solution we need.
  // https://www.youtube.com/watch?v=4Qu4EjsIKI8
  row = linprog_select_cutting_row(lp);
  while (row != lp->conditions) {
    linprog_new_constraint(lp);
    newrow = lp->conditions - 1;
    newcol = lp->variables - 1;

    for (col = 0; col <= lp->variables; col++) {
      r = rational_fpart(lp->coeff[row][col]);
      lp->coeff[newrow][col] = rational_neg(r);
    }
    lp->coeff[newrow][newcol].num = 1;

    col = linprog_select_cutting_col(lp, newrow);
    linprog_pivot(lp, col, newrow);

    row = linprog_select_cutting_row(lp);
  }

  assert(lp->coeff[lp->conditions][lp->variables].den == 1);
  return lp->coeff[lp->conditions][lp->variables].num;
}

void linprog_init(linprog *lp, iterator const *iter) {
  size_t y, x;

  memset(lp, 0, sizeof(linprog));
  for (y = 0; y < MAX_CONDITIONS; y++) {
    for (x = 0; x < MAX_VARIABLES; x++) {
      lp->coeff[y][x].den = 1;
    }
  }

  lp->variables = iter->targets + iter->nbuttons;
  lp->conditions = iter->targets;

  // Set the coefficients of the variables (including the objective function)
  // Each 'button' from the input is a decision variable; the coefficients
  // encode which joltage requirements each button affects.
  for (x = 0; x < iter->nbuttons; x++) {
    for (y = 0; y < iter->targets; y++) {
      if ((1 << y) & iter->buttons[x]) {
        lp->coeff[y][x].num = 1;
      }
    }
    lp->coeff[lp->conditions][x].num = -1;
  }

  for (y = 0; y < iter->targets; y++) {
    // The target joltages are encoded as the constant terms of the
    // inequalities.
    lp->coeff[y][lp->variables].num = iter->joltages[y];

    // Set the coefficients of the artificial variables (including the
    // objecteive function).
    lp->coeff[y][iter->nbuttons + y].num = 1;
    lp->coeff[lp->conditions][iter->nbuttons + y].num = -999999;
  }

  // The initial basic variables (the artificial variables) are not actually
  // basic yet because they have coefficients of -M in the objective function.
  for (x = 0; x < lp->conditions; x++) {
    linprog_pivot(lp, iter->nbuttons + x, x);
  }
}

void part1(char const *input) {
  iterator iter = {input};
  uint64_t total = 0;
  uint64_t src, dst;
  size_t i;

  aoc_array dists = {0};
  aoc_array stack = {0};

  while (next(&iter)) {
    // Initialize the distances to the target to the max value (except the
    // target itself, which has a distance of 0).
    dists.count = (1 << iter.targets);
    aoc_array_ensure_capacity(&dists, dists.count);
    memset(dists.items, 0xff, sizeof(*dists.items) * dists.count);
    dists.items[iter.indicators] = 0;

    aoc_array_push(&stack, iter.indicators);
    while (aoc_array_pop(&stack, &src)) {
      for (i = 0; i < iter.nbuttons; i++) {
        dst = src ^ iter.buttons[i];
        if (dists.items[dst] > dists.items[src] + 1) {
          dists.items[dst] = dists.items[src] + 1;
          aoc_array_push(&stack, dst);
        }
      }
    }

    total += dists.items[0];
  }

  printf("%" PRIu64 "\n", total);

  aoc_array_free(&dists);
  aoc_array_free(&stack);
}

void part2(char const *input) {
  iterator iter = {input};
  uint64_t total = 0;
  linprog lp;

  while (next(&iter)) {
    linprog_init(&lp, &iter);
    total += linprog_solve(&lp);
  }

  printf("%" PRIu64 "\n", total);
}
