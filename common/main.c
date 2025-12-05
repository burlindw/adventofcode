#include <stdio.h>
#include <stdlib.h>

void part1(char const *input);
void part2(char const *input);

int main(int argc, char **argv) {
  int status = EXIT_FAILURE;
  char *input = NULL;
  FILE *file = NULL;
  long told;
  size_t length;
  
  if (argc < 3) {
    fprintf(stderr, "invalid number of arguments\n");
    fprintf(stderr, "usage: %s [input] [part]\n", argv[0]);
    goto defer;
  }

  if (!(file = fopen(argv[1], "r"))) {
    fprintf(stderr, "could not open file '%s'\n", argv[1]);
    perror("reason");
    goto defer;
  }

  if (fseek(file, 0, SEEK_END) < 0) {
    fprintf(stderr, "seek failed for '%s'\n", argv[1]);
    perror("reason");
    goto defer;
  }

  if ((told = ftell(file)) < 0) {
    fprintf(stderr, "failed to get size of '%s'\n", argv[1]);
    perror("reason");
    goto defer;
  }

  length = (size_t)told;
  rewind(file);

  if (!(input = malloc(length + 1))) {
    fprintf(stderr, "failed to allocate %zu bytes\n", (size_t)(told + 1));
    goto defer;
  }

  if (fread(input, 1, length, file) != length) {
    fprintf(stderr, "failed to read '%s'\n", argv[1]);
    perror("reason");
    goto defer;
  }

  input[length] = '\0';


  switch (argv[2][0]) {
    case '1':
      printf("\n\033[32m");
      part1(input);
      printf("\033[0m\n");
      break;

    case '2':
      printf("\n\033[32m");
      part2(input);
      printf("\033[0m\n");
      break;

    default:
      fprintf(stderr, "invalid part specifier '%s'\n", argv[2]);
      fprintf(stderr, "expected either '1' or '2'\n");
      goto defer;
  }

  status = EXIT_SUCCESS;

defer:
  if (file) fclose(file);
  free(input);
  return status;
}
