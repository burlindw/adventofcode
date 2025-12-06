#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t min64(uint64_t a, uint64_t b) {
  return a < b ? a : b;
}

uint64_t max64(uint64_t a, uint64_t b) {
  return a > b ? a : b;
}

typedef struct _iterator {
  char const *input;
  uint64_t min;
  uint64_t max;
} iterator;

bool readint(char const **input, uint64_t *value) {
  char buffer[50];
  size_t length = 0;

  while (isdigit(**input)) {
    buffer[length++] = *((*input)++);
    assert(length < sizeof(buffer));
  }

  if (length == 0) return false;

  buffer[length] = '\0';
  *value = strtoul(buffer, NULL, 10);
  return true;
}

bool next_range(iterator *it) {
  char const *input = it->input;

  if (!readint(&input, &it->min)) goto fail;
  if (*input++ != '-') goto fail;
  if (!readint(&input, &it->max)) goto fail;
  while (isspace(*input))
    input++;

  it->input = input;
  return true;

fail:
  return false;
}

bool next_value(iterator *it) {
  char const *input = it->input;

  if (!readint(&input, &it->min)) goto fail;
  while (isspace(*input))
    input++;

  it->max = it->min;
  it->input = input;
  return true;

fail:
  return false;
}

typedef struct range_tree_node {
  uint64_t min;
  uint64_t max;
  struct range_tree_node *left;
  struct range_tree_node *right;
} range_tree_node;

typedef struct range_tree {
  range_tree_node *root;
} range_tree;

void range_tree_node_free(range_tree_node *node) {
  if (!node) return;
  range_tree_node_free(node->left);
  range_tree_node_free(node->right);
  free(node);
}

void range_tree_free(range_tree *tree) {
  range_tree_node_free(tree->root);
  tree->root = NULL;
}

range_tree_node *range_tree_node_delete_ge(range_tree_node *node, uint64_t min) {
  range_tree_node *spare;
  if (!node) return NULL;

  if (node->max < min) {
    node->right = range_tree_node_delete_ge(node->right, min);
    return node;
  }

  if (node->min < min) {
    node->max = min - 1;
    return node;
  }

  spare = node->left;
  node->left = NULL;

  range_tree_node_free(node);
  return range_tree_node_delete_ge(spare, min);
}

range_tree_node *range_tree_node_delete_le(range_tree_node *node, uint64_t max) {
  range_tree_node *spare;
  if (!node) return NULL;

  if (max < node->min) {
    node->left = range_tree_node_delete_le(node->left, max);
    return node;
  }

  if (max < node->max) {
    node->min = max + 1;
    return node;
  }

  spare = node->right;
  node->right = NULL;

  range_tree_node_free(node);
  return range_tree_node_delete_le(spare, max);
}

range_tree_node *range_tree_node_insert(
  range_tree_node *node,
  uint64_t min,
  uint64_t max
) {
  if (!node) {
    node = malloc(sizeof(range_tree_node));
    *node = (range_tree_node){min, max};
    return node;
  }

  if (max < node->min) {
    node->left = range_tree_node_insert(node->left, min, max);
    return node;
  }

  if (node->max < min) {
    node->right = range_tree_node_insert(node->right, min, max);
    return node;
  }

  if (node->min <= min && max <= node->max) {
    return node;
  }

  if (min < node->min) {
    node->left = range_tree_node_delete_ge(node->left, min);
    node->min = min;
  }

  if (node->max < max) {
    node->right = range_tree_node_delete_le(node->right, max);
    node->max = max;
  }

  return node;
}

void range_tree_insert(range_tree *tree, uint64_t min, uint64_t max) {
  tree->root = range_tree_node_insert(tree->root, min, max);
}

bool range_tree_node_contains(range_tree_node *node, uint64_t value) {
  if (!node) return false;

  if (value < node->min) {
    return range_tree_node_contains(node->left, value);
  } else if (node->max < value) {
    return range_tree_node_contains(node->right, value);
  } else {
    return true;
  }
}

bool range_tree_contains(range_tree *tree, uint64_t value) {
  return range_tree_node_contains(tree->root, value);
}

uint64_t range_tree_node_count(range_tree_node *node) {
  uint64_t count = 0;
  if (!node) return 0;
  
  count += range_tree_node_count(node->left);
  count += node->max - node->min + 1;
  count += range_tree_node_count(node->right);
  return count;
}

uint64_t range_tree_count(range_tree *tree) {
  return range_tree_node_count(tree->root);
}

void part1(char const *input) {
  iterator it = {input};
  range_tree tree = {0};
  uint64_t total = 0;

  while (next_range(&it)) {
    range_tree_insert(&tree, it.min, it.max);
  }

  while (next_value(&it)) {
    total += range_tree_contains(&tree, it.min);
  }

  printf("%" PRIu64 "\n", total);
  range_tree_free(&tree);
}

void part2(char const *input) {
  iterator it = {input};
  range_tree tree = {0};
  uint64_t total;

  while (next_range(&it)) {
    range_tree_insert(&tree, it.min, it.max);
  }

  total = range_tree_count(&tree);
  printf("%" PRIu64 "\n", total);
  range_tree_free(&tree);
}
