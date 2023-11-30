#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int b;
  int e;
  int v;
  int n;
  int s;
  int t;
} Flags;

void cat_parser(int *argc, char *argv[], Flags *flags);
void sym(int *now, Flags *flags);
void output(int argc, char *argv[], Flags flags);
void printLineNumbers(int *pred_n, Flags flags);

int main(int argc, char *argv[]) {
  Flags flags = {0, 0, 0, 0, 0, 0};
  cat_parser(&argc, argv, &flags);
  output(argc, argv, flags);
}

void cat_parser(int *argc, char *argv[], Flags *flags) {
  int opt;
  int long_index = 0;
  struct option const long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'}};

  while ((opt = getopt_long(*argc, argv, "bevEsntT", long_options,
                            &long_index)) != -1) {
    switch (opt) {
      case 'b':
        flags->b = 1;
        break;
      case 'e':
        flags->v = 1;
        flags->e = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'E':
        flags->e = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      case 's':
        flags->s = 1;
        break;
      case 't':
        flags->v = 1;
        flags->t = 1;
        break;
      case 'T':
        flags->t = 1;
        break;
    }
    if (flags->b == 1) {
      flags->n = 0;
    }
  }
}

void output(int argc, char *argv[], Flags flags) {
  char pred_char = '\0';
  int str = 1, now;
  int blank_line = 0;

  for (; optind < argc; optind++) {
    FILE *file = fopen(argv[optind], "rb");
    if (file != NULL) {
      while ((now = fgetc(file)) != EOF) {
        if (flags.s) {
          if (pred_char == '\n' && now == '\n') {
            if (blank_line) {
              continue;
            }
            blank_line = 1;
          } else {
            blank_line = 0;
          }
        }

        if (flags.b &&
            ((pred_char == '\n' && now != '\n') || (str == 1 && now != '\n'))) {
          printLineNumbers(&str, flags);
        }

        if (flags.n && (pred_char == '\n' || str == 1)) {
          printLineNumbers(&str, flags);
        }

        if (flags.t || flags.e || flags.v) {
          sym(&now, &flags);
        }

        pred_char = now;
        putchar(now);
      }

      fclose(file);
    } else {
      printf("s21_cat: %s: No such file or directory\n", argv[optind]);
    }
  }
}

void sym(int *now, Flags *flags) {
  if (flags->e && *now == '\n') {
    printf("$");
  }
  if (flags->t && *now == '\t') {
    printf("^");
    *now = 'I';
  }

  if (*now < 32 && *now != '\n' && *now != '\t') {
    printf("^");
    *now += 64;
  }
  if (*now == 127) {
    printf("^");
    *now -= 64;
  }
  if (*now > 127) {
    printf("M-");
    if (*now >= 160 && *now < 255) {
      *now -= 128;
    } else {
      printf("^");
      *now = *now - 64;
    }
  }
}

void printLineNumbers(int *pred_n, Flags flags) {
  if (flags.b || flags.n) {
    printf("%6d\t", *pred_n);
    (*pred_n)++;
  }
}
