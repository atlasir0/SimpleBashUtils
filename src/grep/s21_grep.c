#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_PATTERNS 4096

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
  int count_files;
} Flags;

void parse_grep_arguments(int *argc, char *argv[], char *pattern[],
                          Flags *flags);
int print_regex(int count_str, int *result, regex_t regex, int len,
                char *argv[], int *stopper, int *count_str_n, char *ch,
                Flags flags);
void print_match(char *argv[], int stopper, int *count_str_n, char *ch,
                 Flags flags);
void print_only_count(char *argv[], int *count_str, Flags flags);
void print_v_flag(char *argv[], char *ch, Flags flags);
void output(int argc, char *argv[], char *pattern[], Flags flags);

int main(int argc, char *argv[]) {
  char *pattern[4096];
  Flags flags = {0};
  parse_grep_arguments(&argc, argv, pattern, &flags);
  output(argc, argv, pattern, flags);
}

void parse_grep_arguments(int *argc, char *argv[], char *pattern[],
                          Flags *flags) {
  int opt, i = 0;
  while ((opt = getopt_long(*argc, argv, "e:ivclnhs", 0, 0)) != -1) {
    switch (opt) {
      case 'e':
        flags->e = 1;
        pattern[i++] = optarg;
        if (i > 1) {
          flags->e = i;
        }
        break;
      case 'i':
        flags->i = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      case 'h':
        flags->h = 1;
        break;
      case 's':
        flags->s = 1;
        break;
    }
    i++;
  }
  if (flags->e && optind + 1 < *argc) {
    flags->count_files = 1;
  }
  if (optind + 2 < *argc) {
    flags->count_files = 1;
  }
  if (flags->l) {
    flags->v = 0;
  }
}

void output(int argc, char *argv[], char *pattern[], Flags flags) {
  char ch[4096];
  int flag = REG_EXTENDED, len = 0, count_str = 0, stopper = 0, count_str_n = 1,
      result = 0;
  int count_stre[MAX_PATTERNS] = {0};
  int count_files = argc - optind;
  int show_filename = count_files > 1;
  regex_t regex;

  if (flags.i) {
    flag = REG_ICASE;
  }

  for (int j = 0; j < flags.e; j++) {
    for (int i = optind; i < argc; i++) {
      FILE *file = fopen(argv[i], "r");
      if (file != NULL) {
        result = regcomp(&regex, pattern[j], flag);
        while (fgets(ch, 4095, file) != NULL) {
          result = regexec(&regex, ch, 0, NULL, 0);
          len = strlen(ch);
          if (ch[len - 1] == '\n') {
            ch[len - 1] = '\0';
          }
          if (!result && flags.v == 0) {
            if (show_filename) {
              printf("%s:", argv[i]);
            }
            printf("%s\n", ch);
            count_stre[j]++;
          }
        }
        fseek(file, 0, SEEK_SET);
        stopper = 0;
        fclose(file);
      } else {
        if (flags.s != 1) {
          printf("grep: %s: No such file or directory\n", argv[i]);
        }
      }
      regfree(&regex);
    }
  }

  regcomp(&regex, argv[optind], flag);
  optind++;

  for (; optind < argc; optind++) {
    FILE *file = fopen(argv[optind], "r");
    if (file != NULL) {
      while (fgets(ch, 4095, file) != NULL && stopper == 0) {
        count_str = print_regex(count_str, &result, regex, len, argv, &stopper,
                                &count_str_n, ch, flags);
        count_str_n++;
      }
      if (flags.c) {
        print_only_count(argv, &count_str, flags);
      }
      stopper = 0;
      count_str_n = 1;
      fclose(file);
    }
    if (file == NULL && flags.s == 0) {
      printf("s21_grep: %s: No such file or directory\n", argv[optind]);
    }
  }
  regfree(&regex);
}

int print_regex(int count_str, int *result, regex_t regex, int len,
                char *argv[], int *stopper, int *count_str_n, char *ch,
                Flags flags) {
  *result = regexec(&regex, ch, 0, NULL, 0);
  len = strlen(ch);
  if (ch[len - 1] == '\n') {
    ch[len - 1] = '\0';
  }
  if (*result) {
    print_v_flag(argv, ch, flags);
  }
  if (!(*result) && flags.v == 0) {
    if (flags.l) {
      printf("%s\n", argv[optind]);
      *stopper = 1;
    }
    if (*stopper == 0) {
      print_match(argv, *stopper, count_str_n, ch, flags);
    }
    count_str++;
  }
  if (flags.v && *result) {
    count_str++;
  }
  return count_str;
}

void print_match(char *argv[], int stopper, int *count_str_n, char *ch,
                 Flags flags) {
  if (flags.n && stopper == 0) {
    if (flags.count_files) {
      printf("%s:", argv[optind]);
    }
    printf("%d:%s\n", *count_str_n, ch);
  }
  if (flags.h && flags.c == 0) {
    printf("%s\n", ch);
  }
  if (flags.count_files && !flags.c && !flags.n && !flags.h && flags.e == 0) {
    printf("%s:", argv[optind]);
  }
  if (!flags.c && !flags.n && !flags.h && flags.o == 0 && flags.e == 0) {
    printf("%s\n", ch);
  }
}

void print_only_count(char *argv[], int *count_str, Flags flags) {
  if (flags.count_files && flags.h == 0) {
    printf("%s:", argv[optind]);
  }
  printf("%d\n", *count_str);
  *count_str = 0;
}

void print_v_flag(char *argv[], char *ch, Flags flags) {
  if (flags.v) {
    if (flags.count_files && flags.c == 0) {
      printf("%s:", argv[optind]);
    }
    if (flags.c == 0) {
      printf("%s\n", ch);
    }
  }
}
