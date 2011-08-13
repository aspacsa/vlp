#include <ctype.h>
#include <stdio.h>
#include "utils.h"

char str_buffer[BUFSIZ];

char* trim(char *input) {
  char *start = input;
  while (isspace(*start) == 0) {
    start++;
  }

  char *ptr = start;
  char *end = start;
  while (*ptr++ != '\0') {
    if (isspace(*ptr) == 0) {
      end = ptr;
    }
  }

  *end = '\0';
  return start;
}

int is_space(int c) {
  return isspace(c);
}

char* compress_str(char *str) {
  int i;
  for (i = 0; i <= BUFSIZ; i++) {
    if ( isspace(*str) == 0 ) {
      str_buffer[i] = *str;
      str++;
    } else {
      break;
    }
  }
  str_buffer[i] = '\0';  
  return str_buffer;
}
