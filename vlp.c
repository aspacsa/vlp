#include <stdio.h>
#include "menus.h"
#include "database.h"

#define DEVELOPMENT_MODE "-d"

size_t dev_mode = 0;

int initialize(void);
void terminate(void);

int main(int argc, char *argv[]) {
  int done = 0;

  if (argc > 1) {
    if ( strcmp(argv[1], DEVELOPMENT_MODE) == 0 ) {
      dev_mode = 1;
      puts("[Development Mode]");
    }
  }

  done = initialize();
  if (done == 1) {
    puts("Unable to connect to the database.");
  } else {
    puts("VLP Started.");
    main_menu("VLP 1.0");
  }
  printf("\n Bye! \n\n");
  terminate();
  return done;
}

int initialize(void) {
  int result = 0;
  init_db();

  if ( (result = read_db_cnf()) == 0 ) {
    result = connect_to_db();
  } else {
    puts("Unable to access the configuration file.");
  }
  return result;
}

void terminate(void) {
  close_db();
}


