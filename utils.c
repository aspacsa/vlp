#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "utils.h"

char str_buffer[BUFSIZ];

extern int write_db_log(char *line );

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

int is_empty_str(char * str, int size) {
  int i = 1;

  while( i <= size ) {
    if ( isalnum( *str++ ) )
      break;
    i++;
  }
  if ( i - 1 == size )
    return 1;
  return 0;
}

void get_curr_date(char * date_str) {
  time_t now;
  struct tm *localnow;
  char date_buffer[11] = "";

  time( &now );
  localnow = localtime( &now );

  strftime( date_buffer, 11, "%Y-%m-%d", localnow );
  strncpy( date_str, date_buffer, 11 );
  return;
}
