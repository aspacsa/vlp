#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <form.h>
#include "menus.h"
#include "utils.h"
#include "records.h"

#define MENU_ROOT "Main Menu"
#define SETUP_MENU_WIDTH 30
#define SETUP_MENU_HEIGHT 10
#define MAX_SCR_LINE 81

#define ENTER 10
#define DEL 330
#define ESC 27

char new_menu_path[BUFSIZ];
char scr_line[MAX_SCR_LINE];

extern const char* get_db_cnf(void);
extern int set_db_cnf(const char *server, const char *user, const char *pass, const char *db);
extern int init_db(void);
extern int connect_to_db(void);
extern void close_db(void);
extern size_t query_create_new_case(CASE*);
extern size_t query_update_case(CASE*);
extern size_t query_delete_case(const char *case_num);
extern size_t query_select_count_from_case_for(const char *case_num, size_t *count);
extern size_t query_select_all_from_case_for(const char *case_num, CASE *ptr);
extern size_t query_select_all_from_summons_for(const char *case_num, SUMMON *summ_set[], int *count); 
extern size_t query_update_summon(SUMMON *record);
extern const char* db_get_error_msg(void);
extern int write_db_log(char *line);


void print_menu(WINDOW *menu_win, char* menu_choices[], int n_choices, int highlight);
void view_db_conf(const char *curr_path);
void set_new_db_conn(const char *curr_path);
void clear_fields(FIELD *fields[], size_t start, size_t end);
void set_visible_fields(FIELD *fields[], size_t start, size_t end);
void set_invisible_fields(FIELD *fields[], size_t start, size_t end);
void clear_line(size_t row, size_t col);
void print_line(char line[], size_t row, size_t col);
 
void cases_menu(const char *curr_path);
void summons_menu(const char *curr_path);
void actions_menu(const char *curr_path);
void reports_menu(const char *curr_path);
void setup_menu(const char *curr_path);
void dbutils_menu(const char *curr_path);

void summons_dataentry_scr(const char *curr_path, const char *case_num);

char* menu_path(const char* curr_path, const char* sub_menu) {
  sprintf(new_menu_path, "%s > %s", curr_path, sub_menu);
  return new_menu_path;
}

void main_menu(const char *curr_path) {
  const char *menu_title = "Main Menu";
  int choice;
  char curr_menu_path[BUFSIZ];
  sprintf(curr_menu_path, "%s > %s", curr_path, menu_title );

  void (*farray[])(const char*) = { cases_menu,
                                    summons_menu,
                                    actions_menu,
                                    reports_menu,
                                    setup_menu,
                                    dbutils_menu
                                  }; 
  initscr();
  raw();
  while(choice != 7) {
    choice = 0;
    noecho();
    curs_set(0);
    clear();
    mvprintw(0, 0, curr_menu_path);
    mvprintw(4, 10, "1. Cases");
    mvprintw(6, 10, "2. Summons");
    mvprintw(8, 10, "3. Actions");
    mvprintw(10, 10, "4. Reports");
    mvprintw(12, 10, "5. Setup");
    mvprintw(14, 10, "6. DB Utils");
    mvprintw(16, 10, "7. Exit");
    mvprintw(18, 10, "Select an option from the above menu, use only the numbers for selection.");
    refresh();
    choice = toupper(getch());
    switch(choice) {
      case '1':
        choice = 0;
        break;  
      case '2':
        choice = 1;
        break;
      case '3':
        choice = 2;
        break;
      case '4':
        choice = 3;
        break;
      case '5':
        choice = 4;
        break;
      case '6':
        choice = 5;
        break;
      case '7':
        choice = 7;
        break;
      default:
        beep();
        break;
    }
    if (choice < 7)
      farray[choice](curr_menu_path);
  }
  endwin();
  return;
}

void cases_menu(const char *curr_path) {
  const char *screen_title = "Cases";
  const size_t n_fields = 7;
  const size_t starty = 4;
  const size_t startx = 25;
  FIELD *field[n_fields];
  FORM *my_form;
  int width[] = { MAX_CANUM, MAX_CINUM, 
                  MAX_PHYADD, MAX_POSADD, 
                  MAX_STATUS, MAX_DELDATE 
                };
  
  initscr();
  curs_set(1);
  cbreak();
  clear();
  noecho();
  keypad(stdscr, TRUE);

  for (size_t i = 0; i < n_fields - 1; ++i) {
    field[i] = new_field(1, width[i], starty + i * 2, startx, 0, 0);
  }
  field[n_fields - 1] = NULL;

  set_field_back(field[0], A_UNDERLINE);
  field_opts_off(field[0], O_AUTOSKIP);
  field_opts_on(field[0], O_BLANK);
  set_field_back(field[1], A_UNDERLINE);
  field_opts_off(field[1], O_AUTOSKIP);
  set_field_back(field[2], A_UNDERLINE);
  field_opts_off(field[2], O_AUTOSKIP);
  set_field_back(field[3], A_UNDERLINE);
  field_opts_off(field[3], O_AUTOSKIP);
  set_field_back(field[4], A_UNDERLINE);
  field_opts_off(field[4], O_AUTOSKIP);
  set_field_back(field[5], A_UNDERLINE);
  field_opts_off(field[5], O_AUTOSKIP);

  my_form = new_form(field);
  post_form(my_form);
  refresh();

  mvprintw(0, 0, menu_path(curr_path, screen_title));
  mvprintw(4, 10,   "Case Num:      ");
  mvprintw(6, 10,   "Civil Num:     ");
  mvprintw(8, 10,   "Physical Add:  ");
  mvprintw(10, 10,  "Postal Add:    ");
  mvprintw(12, 10,  "Status:        ");
  mvprintw(14, 10,  "Delivery Date: ");
  mvprintw(16, 10,  "(F2) = Update | (F3) = Delete | (F4) = Exit");
  move(4, 25);
  refresh();

  int ch;
  do {
    ch = getch();
    switch(ch) {
      /*case KEY_DOWN:
        form_driver(my_form, REQ_NEXT_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;*/
      case KEY_UP:
        form_driver(my_form, REQ_PREV_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_LEFT:
        form_driver(my_form, REQ_LEFT_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(my_form, REQ_RIGHT_CHAR);
        break;
      case ENTER:
        form_driver(my_form, REQ_NEXT_FIELD);
        form_driver(my_form, REQ_END_LINE);
        if (field_status(field[0])) {
          size_t count = 0;
          char case_num[MAX_CANUM];

          clear_line(20, 10);
          strcpy(case_num, compress_str(field_buffer(field[0], 0) ));
          if ( query_select_count_from_case_for(case_num, &count) ) {
            mvprintw( 20, 10, db_get_error_msg() );
            move(4, 25);
          } else {
            if (count) {
              //call routine to fill in fields
              CASE record;
              if ( query_select_all_from_case_for(case_num, &record) ) {
                mvprintw(20, 10, db_get_error_msg());
                move(4, 25);
              } else {
                char status_buff[4];

                set_field_buffer(field[1], 0, record.civil);
                set_field_buffer(field[2], 0, record.physical_add);
                set_field_buffer(field[3], 0, record.postal_add);
                snprintf(status_buff, 4, "%c", record.status);
                set_field_buffer(field[4], 0, status_buff);
                set_field_buffer(field[5], 0, record.delivery_date);
              }
            } else {            
              clear_fields(field, 1, 5);    
              mvprintw(20, 10, "[!] Case %s does not exist.", case_num);
              move(6, 25);
              set_current_field(my_form, field[0]);
            }
          }
          //mvprintw(20, 10, "Case Number changed.");
          set_field_status(field[0], 0);
        }
        break;
      case KEY_BACKSPACE:
        form_driver(my_form, REQ_PREV_CHAR);
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      case KEY_F(2):
      {
        size_t count = 0;
        char case_num[MAX_CANUM];
        
        clear_line(20, 10);
        strncpy(case_num, compress_str(field_buffer(field[0], 0) ), MAX_CANUM);
        if ( query_select_count_from_case_for(case_num, &count) ) {
          mvprintw( 20, 10, db_get_error_msg() );
         } else {
           CASE record;
           strncpy( record.number, compress_str(field_buffer(field[0], 0)), MAX_CANUM );
           strncpy( record.civil, compress_str(field_buffer(field[1], 0)), MAX_CINUM );
           strncpy( record.physical_add, field_buffer(field[2], 0), MAX_PHYADD );
           strncpy( record.postal_add, field_buffer(field[3], 0), MAX_POSADD );
           record.status = atoi( compress_str(field_buffer(field[4], 0)) );
           strncpy( record.delivery_date, compress_str(field_buffer(field[5], 0)), MAX_DELDATE );
           if (count) {
             // update existing record
             if ( query_update_case(&record) == 0 ) {
               mvprintw(20, 10, "[!] Case has been updated.");
             } else {
               mvprintw( 20, 10, db_get_error_msg() );
             }
           } else {
             // create new record
             if ( query_create_new_case(&record) == 0 ) {
               mvprintw(20, 10, "[!] Case has been created successfully.");
             } else {
               mvprintw( 20, 10, db_get_error_msg() );
              }
           }
        }
        move(4, 25);
        set_current_field(my_form, field[0]);
       }
        break;
      case KEY_F(3):
        {
          size_t count = 0;
          char case_num[MAX_CANUM];

          clear_line(20, 10);
          strncpy(case_num, compress_str(field_buffer(field[0], 0) ), MAX_CANUM);
          if ( strlen(case_num) ) {
            if ( query_select_count_from_case_for(case_num, &count) ) {
              mvprintw( 20, 10, db_get_error_msg() );
            } else {
              if (count) {
                mvprintw(20, 10, "[?] Delete case '%s' ? [Y/n]", case_num);
                int ch = toupper(getch());
                if (ch == 'Y') {
                  if ( query_delete_case(case_num) ) {
                    mvprintw( 20, 10, db_get_error_msg() );
                  } else {
                    clear_fields(field, 0, 5);    
                    mvprintw(20, 10, "[!] Case '%s' has been deleted.");
                  }
                }
              } else {
                mvprintw(20, 10, "[!] Case '%s' does not exist.", case_num);
              }
            }
          } else {
            mvprintw(20, 10, "[!] Must enter a valid Case Number to be deleted.");
          }
        }
        move(4, 25);
        set_current_field(my_form, field[0]);
        break;
      case DEL:
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      default:
        form_driver(my_form, ch);
        break;
    } 
  } while( ch != KEY_F(4) ); 

  unpost_form(my_form);
  free_form(my_form);
  for (size_t i = 0; i < n_fields -1; ++i) {
    free_field(field[i]);
  }
  endwin();
  return;
}

void summons_menu(const char *curr_path) {
  const char *screen_title = "Summons";
  FIELD *field[2];
  FORM *my_form;
 
  initscr();
  curs_set(1);
  cbreak();
  clear();
  noecho();
  keypad(stdscr, TRUE);

  field[0] = new_field( 1, MAX_CANUM, 4, 25, 0, 0 );
  field[1] = NULL;
  
  set_field_back(field[0], A_UNDERLINE);
  field_opts_off(field[0], O_AUTOSKIP);
  field_opts_on(field[0], O_BLANK);

  my_form = new_form(field);
  post_form(my_form);
  refresh();

  int ch;
  do {
    mvprintw( 0, 0, menu_path( curr_path, screen_title ) );
    mvprintw( 2, 10,  "Enter case number then press (Enter) to start. | (F4) = Exit" );
    mvprintw( 4, 10,   "Case Num:      " );
    refresh();
    move( 4, 25 );
    set_current_field( my_form, field[0] );
  
    ch = getch();

    switch ( ch ) {
      case ENTER: 
        {
          size_t count = 0;
          char case_num[MAX_CANUM];

          form_driver(my_form, REQ_NEXT_FIELD);
          strcpy(case_num, compress_str(field_buffer(field[0], 0) ));
          if ( query_select_count_from_case_for(case_num, &count) ) {
            clear_line(20, 10);
            mvprintw( 20, 10, db_get_error_msg() );
            move( 4, 25 );
          } else {
            if ( count ) {
              summons_dataentry_scr( menu_path( curr_path, screen_title ), case_num );
              break;

              SUMMON *summ_set[MAX_SUMM_SET];
              int scount;
              if (query_select_all_from_summons_for(case_num, summ_set, &scount)) {
                clear_line(20, 10);
                mvprintw( 20, 10, db_get_error_msg() );
                move(4, 25);
              } else {
                set_visible_fields(field, 1, 5);
                clear_line(6, 10);
                print_line("(A) = Add | (D) = Delete | (F4) = Exit", 20, 10);
                for (int i = 0; i < scount && i <= MAX_SUMM_SET; ++i)
                  free(summ_set[i]);
              }
            }
          }
        }
        break;
      default:
        form_driver( my_form, ch );
        break;
    }
  } while ( ch != KEY_F(4) );

  unpost_form( my_form );
  free_form( my_form );
  free_field( field[0] );
  endwin();
  return;
}

void summons_dataentry_scr(const char *curr_path, const char *case_num) {
  const size_t n_fields = 6;
  const size_t starty = 6;
  const size_t startx = 25;
  FIELD *field[n_fields];
  FORM *my_form;
  SUMMON record;
  int width[] = {  MAX_SUMM_NAME, MAX_SUMM_STATUS, MAX_SUMM_REASON,  
                   MAX_SUMM_CITY, MAX_SUMM_DATE
                };
 
  for ( size_t i = 0; i < n_fields - 1; ++i )
    field[i] = new_field(1, width[i], starty + i * 2, startx, 0, 0);
  field[n_fields - 1] = NULL;
  
  set_field_back(field[0], A_UNDERLINE);
  field_opts_off(field[0], O_AUTOSKIP);
  set_field_back(field[1], A_UNDERLINE);
  field_opts_off(field[1], O_AUTOSKIP);
  set_field_back(field[2], A_UNDERLINE);
  field_opts_off(field[2], O_AUTOSKIP);
  set_field_back(field[3], A_UNDERLINE);
  field_opts_off(field[3], O_AUTOSKIP);
  set_field_back(field[4], A_UNDERLINE);
  field_opts_off(field[4], O_AUTOSKIP);

  my_form = new_form(field);
  post_form(my_form);
  refresh();
 
  mvprintw( 0, 0,   curr_path );
  mvprintw( 4, 10,  "Case Number:   %s", case_num );
  mvprintw( 6, 10,  "Person:        " );
  mvprintw( 8, 10,  "Status:        " );
  mvprintw( 10, 10, "Reason:        " );
  mvprintw( 12, 10, "City:          " );
  mvprintw( 14, 10, "Date Summoned: " );
  mvprintw( 16, 10, "(F2) = Update | (F3) = Delete | (F5) = List | (ESC) = Previous Screen" );
  set_visible_fields( field, 1, 5 );
  move( 6, 25 );
  set_current_field( my_form, field[0] );
 
  record.id = 0;
  int ch;
  do {
    ch = getch();
   
    switch ( ch ) {
      case KEY_UP:
        form_driver(my_form, REQ_PREV_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_LEFT:
        form_driver(my_form, REQ_LEFT_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(my_form, REQ_RIGHT_CHAR);
        break;
      case KEY_BACKSPACE:
        form_driver(my_form, REQ_PREV_CHAR);
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      case ENTER:
        form_driver( my_form, REQ_NEXT_FIELD );
        form_driver( my_form, REQ_END_LINE );
        break;
      case KEY_F(2):
        strncpy( record.case_num, case_num, MAX_CANUM );
        strncpy( record.name, field_buffer(field[0], 0), MAX_SUMM_NAME );
        record.status = atoi( compress_str( field_buffer(field[1], 0) ) );
        record.reason = atoi( compress_str( field_buffer(field[2], 0) ) );
        strncpy( record.city_code, compress_str( field_buffer(field[3], 0) ), MAX_SUMM_CITY );
        strncpy( record.summon_date, compress_str( field_buffer(field[4], 0) ), MAX_SUMM_DATE );

        if ( query_update_summon( &record ) ) {
          mvprintw( 18, 10, db_get_error_msg() );
          move( 6, 25 );
          set_current_field( my_form, field[0] );
        } else {
          clear_fields( field, 0, 4 );
          mvprintw( 18, 10, "[!] Summon has been updated." );
          move( 6, 25 );
          set_current_field( my_form, field[0] );
        }
        break;
      default:
        form_driver( my_form, ch );
        break;
    }

  } while ( ch != ESC );

  unpost_form( my_form );
  free_form( my_form );
  for ( size_t i = 0; i < n_fields -1; ++i )
    free_field( field[i] );
  return;
}

void actions_menu(const char *curr_path) {
  return;
}

void reports_menu(const char *curr_path) {
  return;
}

void setup_menu(const char *curr_path) {
  char *setup_choices[] = { "1. View configuration", 
                            "2. New DB Connection",
                            "3. Quit" 
                          };
  void (*farray[])(const char*) = { view_db_conf,
                                    set_new_db_conn, 
                                  };  
  
  size_t n_choices = sizeof setup_choices / sizeof(char *);
  const char *menu_title = "Setup Menu";
  WINDOW *menu_win;
  int startx, starty, highlight, choice;

  initscr();
  startx = (80 - SETUP_MENU_WIDTH) / 2;
  starty = (24 - SETUP_MENU_HEIGHT) / 2;
  
  menu_win = newwin(SETUP_MENU_HEIGHT, SETUP_MENU_WIDTH, starty, startx);
  keypad(menu_win, TRUE);
  while(1) {
    noecho();
    curs_set(0);
    cbreak();
    clear();
    highlight = 1;
    mvprintw(0, 0, menu_path(curr_path, menu_title));
    refresh();
    print_menu(menu_win, setup_choices, n_choices, highlight);
    
    choice = 0;
    while(1) {
      int c = wgetch(menu_win);
      switch(c) {
        case KEY_UP:
          if (highlight == 1) {
            highlight = n_choices;
          } else {
            --highlight;
          }
          break;
        case KEY_DOWN:
          if (highlight == n_choices) {
            highlight = 1;
          } else {
            ++highlight;
          }
          break;
        case ENTER:
          choice = highlight;
          break;
      }
      print_menu(menu_win, setup_choices, n_choices, highlight);
      if (choice != 0)
        break;
    }

    if (choice >= 1 && choice <= 2) 
      farray[choice-1](menu_path(curr_path, menu_title));
    else if (choice == 3)
       break;
  }
  clrtoeol();
  refresh();
  endwin();
  return;
}

void dbutils_menu(const char *curr_path) {
  return;
}

void view_db_conf(const char *curr_path) {
  char *screen_title = "Configuration";

  initscr();
  clear();
  noecho();
  raw();
  printw(menu_path(curr_path, screen_title));
  printw(get_db_cnf());
  printw("\n\nPress any key to continue...");
  getch();
  endwin();
}

void set_new_db_conn(const char *curr_path) {
  const int n_fields = 5;
  const int width = 20;
  const int starty = 4;
  const int startx = 20;
  FIELD *field[n_fields];
  FORM *my_form;
  char *screen_title = "New DB Connection";
  
  initscr();
  curs_set(1);
  cbreak();
  clear();
  noecho();
  keypad(stdscr, TRUE);

  for (size_t i = 0; i < n_fields - 1; ++i) {
    field[i] = new_field(1, width, starty + i * 2, startx, 0, 0);
  }
  field[n_fields - 1] = NULL;

  set_field_back(field[0], A_UNDERLINE);
  field_opts_off(field[0], O_AUTOSKIP);
  set_field_back(field[1], A_UNDERLINE);
  field_opts_off(field[1], O_AUTOSKIP);
  set_field_back(field[2], A_UNDERLINE);
  field_opts_off(field[2], O_AUTOSKIP);
  field_opts_off(field[2], O_PUBLIC);
  set_field_back(field[3], A_UNDERLINE);
  field_opts_off(field[3], O_AUTOSKIP);

  my_form = new_form(field);
  post_form(my_form);
  refresh();

  mvprintw(0, 0, menu_path(curr_path, screen_title));
  mvprintw(4, 10, "Server:   ");
  mvprintw(6, 10, "User:     ");
  mvprintw(8, 10, "Password: ");
  mvprintw(10, 10, "DB:       ");
  mvprintw(14, 10, "(F2) = Save and Exit | (F4) = Exit without saving");
  move(4, 20);
  refresh();

  int ch;
  do {
    ch = getch();
    switch(ch) {
      case KEY_DOWN:
        form_driver(my_form, REQ_NEXT_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_UP:
        form_driver(my_form, REQ_PREV_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_LEFT:
        form_driver(my_form, REQ_LEFT_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(my_form, REQ_RIGHT_CHAR);
        break;
      case ENTER:
        form_driver(my_form, REQ_NEXT_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_BACKSPACE:
        form_driver(my_form, REQ_PREV_CHAR);
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      case DEL:
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      default:
        if (is_space(ch) == 0)
          form_driver(my_form, ch);
        break;
    } 
  } while( (ch != KEY_F(2)) && (ch != KEY_F(4)) ); 
  if (ch == KEY_F(2)) {
    char *server = field_buffer(field[0], 0);
    char *admin = field_buffer(field[1], 0);
    char *pass = field_buffer(field[2], 0);
    char *db = field_buffer(field[3], 0);

    strcpy(server, compress_str(server));
    strcpy(admin, compress_str(admin));
    strcpy(pass, compress_str(pass));
    strcpy(db, compress_str(db));
    
    int result = set_db_cnf(server, admin, pass, db);

    mvprintw(16, 10, "**Warning: The system will apply changes to the configuration now.");
    close_db();
    init_db();
    mvprintw(18, 10, "Press any key to continue...");
    ch = getch();
    if (connect_to_db() == 1) {
      mvprintw(20, 10, "[!] Unable to connect to database with new configuration.");
    } else {
      mvprintw(20, 10, "[!] Connection successful.");
    }
    ch = getch();
  }
  unpost_form(my_form);
  free_form(my_form);
  for (size_t i = 0; i < n_fields -1; ++i) {
    free_field(field[i]);
  }
  endwin();
  return;
}

void print_menu(WINDOW *menu_win, char *menu_choices[], int n_choices, int highlight) {
  size_t x, y;
  x = 2;
  y = 2;

  box(menu_win, 0, 0);
  for(size_t i = 0; i < n_choices; ++i) { 
    if (highlight == i + 1) { /* High light the present choice */
      wattron(menu_win, A_REVERSE);
      mvwprintw(menu_win, y, x, "%s", menu_choices[i]);
      wattroff(menu_win, A_REVERSE);
    } else {
      mvwprintw(menu_win, y, x, "%s", menu_choices[i]);
    }
    ++y;
  }
  wrefresh(menu_win);
  return;
}

void clear_fields(FIELD *fields[], size_t start, size_t end) {
  for ( size_t i = start; i <= end; i++ )
    set_field_buffer( fields[i], 0, "" );
  return;
}

void set_visible_fields(FIELD *fields[], size_t start, size_t end) {
  for ( size_t i = start; i <= end; ++i )
    field_opts_on( fields[i], O_VISIBLE );
  return;
}

void set_invisible_fields(FIELD *fields[], size_t start, size_t end) {
  for ( size_t i = start; i <= end; ++i )
    field_opts_off( fields[i], O_VISIBLE );
  return;
}

void clear_line(size_t row, size_t col) {
  memset( scr_line, ' ', 80 );
  scr_line[81] = '\0';
  mvprintw( row, col, scr_line );
  refresh();
  return;
}

void print_line(char line[], size_t row, size_t col) {
  strncpy( scr_line, line, MAX_SCR_LINE );
  mvprintw( row, col, scr_line );
  return;
}
