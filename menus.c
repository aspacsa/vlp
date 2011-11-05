#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <form.h>
#include "menus.h"
#include "utils.h"
#include "records.h"
#include "reports.h"

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
extern size_t query_create_new_case(Case_t *);
extern size_t query_update_case(Case_t *);
extern size_t query_delete_case(const char *case_num);
extern size_t query_select_count_from_case_for(const char *case_num, size_t *count);
extern size_t query_select_all_from_case_for(const char *case_num, Case_t *ptr);
extern size_t query_select_all_from_summons_for(const char *case_num);
extern Summon_t * get_summon_from_result(void);
extern void free_summon_result(void);
extern size_t query_update_summon(Summon_t *record);
extern size_t query_delete_summon(size_t summ_id);
extern size_t query_select_count_from_actions_for(const char *case_num, size_t *count);
extern size_t query_select_all_from_actions_for(const char *case_num);
extern Action_t * get_action_from_result(void);
extern size_t query_add_action(const Action_t const * record);
extern size_t query_select_all_codes_from_case_status(void);
extern size_t query_select_all_codes_from_summon_status(void); 
extern size_t query_select_all_codes_from_summon_reasons(void); 
extern size_t query_select_all_codes_from_action_types(void);
extern const Code_t const * get_code_from_result(void);
extern void free_code_result(void);
extern size_t query_select_all_codes_from_city_rates(void);
extern const SCode_t const * get_scode_from_result(void);
extern void free_scode_result(void);
extern const char* db_get_error_msg(void);
extern int write_db_log(char *line);


void print_menu(WINDOW *menu_win, char* menu_choices[], int n_choices, int highlight);
void view_db_conf(const char *curr_path);
void set_new_db_conn(const char *curr_path);
void clear_fields(FIELD *fields[], size_t start, size_t end);
void set_visible_fields(FIELD *fields[], size_t start, size_t end);
void set_invisible_fields(FIELD *fields[], size_t start, size_t end);
void clear_line(size_t row, size_t col);
void clear_lines(size_t start, size_t end);
void print_line(char line[], size_t row, size_t col);
void get_cursor_pos(const FIELD const * field, int* row, int* col);
 
void cases_menu(const char *curr_path);
void summons_menu(const char *curr_path);
void actions_menu(const char *curr_path);
void reports_menu(const char *curr_path);
void setup_menu(const char *curr_path);
void dbutils_menu(const char *curr_path);
void billing_menu(const char *curr_path);
void suggestions_menu(const char *curr_path);
void dummy_menu(const char *curr_path);

void summons_dataentry_scr(const char *curr_path, const char *case_num);
void summons_list_scr(Summon_t **summons, size_t count, size_t *selection);

void actions_dataentry_scr(const char *curr_path, const char *case_num);
size_t actions_list(const char *case_num); 
void print_action(const Action_t * action, size_t idx);
void from_to_selector(const char *curr_path, char **from, char **to, int *action); 

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
    choice = getch();
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
  mvprintw(16, 10,  "(F1) = Options | (F2) = Update | (F3) = Delete | (F4) = Exit");
  move(4, 25);
  refresh();

  int ch;
  do {
    ch = getch();
    switch(ch) {
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
        form_driver( my_form, REQ_NEXT_FIELD );
        form_driver( my_form, REQ_END_LINE );
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
              Case_t record;
              if ( query_select_all_from_case_for(case_num, &record) ) {
                mvprintw(20, 10, db_get_error_msg());
                move(4, 25);
              } else {
                char status_buff[4];

                set_field_buffer(field[1], 0, record.civil);
                set_field_buffer(field[2], 0, record.physical_add);
                set_field_buffer(field[3], 0, record.postal_add);
                snprintf( status_buff, 4, "%d", record.status );
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
          set_field_status(field[0], 0);
        }
        break;
      case KEY_BACKSPACE:
        form_driver(my_form, REQ_PREV_CHAR);
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      case ESC:
        {
          FIELD * curr_field = current_field( my_form );
          int row, col;
          get_cursor_pos( curr_field, &row, &col );
          clear_lines( 20, 40 );
          move( row, col );
          set_current_field( my_form, curr_field );
        }
        break;
      case KEY_F(1):
        if ( current_field( my_form ) == field[4] ) {
          clear_lines( 20, 30 );
          if ( query_select_all_codes_from_case_status() ) {
            mvprintw( 20, 10, db_get_error_msg() );
          } else {
            const Code_t const * code_ptr;
            size_t count = 0;

            mvprintw( 20, 5, "Status Options:" );
            while ( ( code_ptr = get_code_from_result() ) != NULL ) {           
              mvprintw( 21 + count, 10, "[%d] %s", code_ptr->code, code_ptr->desc );
              count++;
            }
            free_code_result();
            move( 12, 25 );
            set_current_field( my_form, field[4] );
          }  
        } 
        break;
      case KEY_F(2):
      {
        size_t count = 0;
        char case_num[MAX_CANUM];
        
        clear_lines( 20, 30 );
        strncpy(case_num, compress_str(field_buffer(field[0], 0) ), MAX_CANUM);
        if ( query_select_count_from_case_for(case_num, &count) ) {
          mvprintw( 20, 10, db_get_error_msg() );
        } else {
          Case_t record;
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

          clear_lines(20, 30);
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
  
  set_field_back( field[0], A_UNDERLINE );
  field_opts_off( field[0], O_AUTOSKIP );
  field_opts_on( field[0], O_BLANK );

  my_form = new_form( field );
  post_form( my_form );
  refresh();

  mvprintw( 0, 0, menu_path( curr_path, screen_title ) );
  mvprintw( 2, 10,  "Enter case number then press (Enter) to start. | (F4) = Exit" );
  mvprintw( 4, 10,   "Case Num:      " );
  refresh();
  move( 4, 25 );
  set_current_field( my_form, field[0] );

  int done = 0;
  int ch;
  do {
    ch = getch();

    switch ( ch ) {
      case KEY_LEFT:
        form_driver( my_form, REQ_LEFT_CHAR );
        break;
      case KEY_RIGHT:
        form_driver( my_form, REQ_RIGHT_CHAR );
        break;
      case KEY_BACKSPACE:
        form_driver( my_form, REQ_PREV_CHAR );
        form_driver( my_form, REQ_DEL_CHAR );
        break;
      case DEL:
        form_driver( my_form, REQ_DEL_CHAR );
        break;
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
              done = 1;
              break;
            }
          }
        }
        break;
      default:
        form_driver( my_form, ch );
        break;
    }
  } while ( ( ch != KEY_F(4) ) && ( done != 1 ) );

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
  Summon_t record;
  int width[] = {  MAX_SUMM_NAME, MAX_SUMM_STATUS, MAX_SUMM_REASON,  
                   MAX_SUMM_CITY, MAX_SUMM_DATE
                };
 
  for ( size_t i = 0; i < n_fields - 1; ++i )
    field[i] = new_field(1, width[i], starty + i * 2, startx, 0, 0);
  field[n_fields - 1] = NULL;
  
  set_field_back( field[0], A_UNDERLINE );
  field_opts_off( field[0], O_AUTOSKIP  );
  set_field_back( field[1], A_UNDERLINE );
  field_opts_on(  field[1], O_BLANK     );
  field_opts_off( field[1], O_AUTOSKIP  );
  set_field_back( field[2], A_UNDERLINE );
  field_opts_off( field[2], O_AUTOSKIP  );
  field_opts_on(  field[2], O_BLANK     );
  set_field_back( field[3], A_UNDERLINE );
  field_opts_off( field[3], O_AUTOSKIP  );
  field_opts_on(  field[3], O_BLANK     );
  set_field_back( field[4], A_UNDERLINE );
  field_opts_off( field[4], O_AUTOSKIP  );

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
  mvprintw( 16, 10, "(F1) = Options | (F2) = Update | (F3) = Delete | (F5) = List | (ESC) = Main Menu" );
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
        if ( current_field( my_form ) == field[0] )
          form_driver( my_form, REQ_END_LINE );
        break;
      case KEY_F(1):
        clear_lines( 20, 40 );
        {
          FIELD * curr_fld = current_field( my_form );
          size_t error = 0;
          size_t in_target_fld = 0;
          char fld_name[7];     

          if ( curr_fld == field[1] ) {
            error = query_select_all_codes_from_summon_status();
            in_target_fld = 1;
            strncpy( fld_name, "Status", 7 );
          } else if ( curr_fld == field[2] ) {
            error = query_select_all_codes_from_summon_reasons();
            in_target_fld = 1;
            strncpy( fld_name, "Reason", 7 );
          } else if ( curr_fld == field[3] ) {
            if ( query_select_all_codes_from_city_rates() ) {
              clear_line(20, 10);
              mvprintw( 20, 10, db_get_error_msg() );
              move( 12, 25 );
              set_current_field( my_form, curr_fld );
            } else {
              const SCode_t const * scode_ptr;
              size_t count = 0;
              size_t column = 10;

              mvprintw( 20, 5, "Cities:" );
              while ( ( scode_ptr = get_scode_from_result() ) != NULL ) {
                mvprintw( 21 + count++, column, "[%s] %s", scode_ptr->code, scode_ptr->name );
                if ( count == 30 ) {
                  column += 20;
                  count = 0;
                }
              }
              free_scode_result();
            }
          }
          if ( !error && in_target_fld ) {
            const Code_t const * code_ptr;
            size_t count = 0;

            mvprintw( 20, 5, "%s Options:", fld_name );
            while ( ( code_ptr = get_code_from_result() ) != NULL )          
              mvprintw( 21 + count++, 10, "[%d] %s", code_ptr->code, code_ptr->desc );
            free_code_result();
          }
          int row, col;
          get_cursor_pos( curr_fld, &row, &col );
          move( row, col );
          set_current_field( my_form, curr_fld );
        }
        break;
      case KEY_F(2):
        clear_lines( 20, 40 );

        char person_name[MAX_SUMM_NAME];
        strncpy( person_name, field_buffer(field[0], 0), MAX_SUMM_NAME );
         
        if ( is_empty_str( person_name, MAX_SUMM_NAME ) ) {
          mvprintw( 20, 10, "[!] Summon must at least have the person's name." ); 
          move( 6, 25 );
          set_current_field( my_form, field[0] );
          break;
        }
        strncpy( record.case_num, case_num, MAX_CANUM );
        strncpy( record.name, field_buffer(field[0], 0), MAX_SUMM_NAME );
        record.status = atoi( compress_str( field_buffer(field[1], 0) ) );
        record.reason = atoi( compress_str( field_buffer(field[2], 0) ) );
        strncpy( record.city_code, compress_str( field_buffer(field[3], 0) ), MAX_SUMM_CITY );
        strncpy( record.summon_date, compress_str( field_buffer(field[4], 0) ), MAX_SUMM_DATE );
        if ( query_update_summon( &record ) ) {
          mvprintw( 20, 10, db_get_error_msg() );
          move( 6, 25 );
          set_current_field( my_form, field[0] );
        } else {
          clear_fields( field, 0, 4 );
          mvprintw( 20, 10, "[!] Summon has been updated." );
          move( 6, 25 );
          set_current_field( my_form, field[0] );
          record.id = 0;
        }
        break;
      case KEY_F(3):
        clear_lines( 20, 40 );
        if ( record.id > 0 ) {
          mvprintw( 20, 10, "[?] Delete summon '%u' ? [Y/n]", record.id );
          int ch = toupper( getch() );
          if ( ch == 'Y' ) {
            if ( query_delete_summon( record.id ) ) {
              mvprintw( 20, 10, db_get_error_msg() );
            } else {
              clear_fields( field, 0, 4 );    
              mvprintw( 20, 10, "[!] Summon '%u' has been deleted.", record.id );
              move( 6, 25 );
              set_current_field( my_form, field[0] );
              record.id = 0;
            }
          }
        }
        break;
      case KEY_F(5):
        clear_lines( 20, 40 );
        if ( query_select_all_from_summons_for( case_num ) ) {
          mvprintw( 20, 10, db_get_error_msg() );
        } else {
          Summon_t *summ_ptr;
          Summon_t *summons[MAX_SUMM_SET];
          size_t count = 0;

          while ( ( summ_ptr = get_summon_from_result() ) != NULL ) {           
            summons[count] = summ_ptr;
            count++;
          }
          if ( count ) {
            size_t selection;
            char code_buff[4];

            summons_list_scr( summons, count, &selection );
            if ( selection > 0 ) {
              summ_ptr = summons[selection - 1];
              record.id = summ_ptr->id;
              set_field_buffer( field[0], 0, summ_ptr->name );
              snprintf( code_buff, 4, "%d", summ_ptr->status );
              set_field_buffer( field[1], 0, code_buff );
              snprintf( code_buff, 4, "%d", summ_ptr->reason );
              set_field_buffer( field[2], 0, code_buff );
              set_field_buffer( field[3], 0, summ_ptr->city_code );
               set_field_buffer( field[4], 0, summ_ptr->summon_date );
            }
            free_summon_result();
          } else {
            mvprintw( 20, 10, "[!] Case %s has no summons.", case_num );
          }
        }
        set_current_field( my_form, field[0] );
        move( 6, 25 );
        break;
      default:
        {
          FIELD * curr_fld = current_field( my_form );

          if ( curr_fld == field[1] || curr_fld == field[2] ) {
            if ( !isdigit( ch ) )
              break;
          } else if ( curr_fld == field[3] ) {
            if ( !isalpha( ch ) )
              break;
            else
              ch = toupper( ch );
          }
          form_driver( my_form, ch );
          break;
        } 
    }
  } while ( ch != ESC );

  unpost_form( my_form );
  free_form( my_form );
  for ( size_t i = 0; i < n_fields - 1; ++i )
    free_field( field[i] );
  return;
}

void summons_list_scr(Summon_t *summons[], size_t count, size_t *selection) {
  Summon_t *summon;

  clear_lines( 20, 40 );
  mvprintw( 20, 5, "List:" );
  for ( size_t i = 0; i < count; ++i ) {
    summon = *summons++;
    mvprintw( 21 + i, 10, "%u. %s", i == 10 ? 0 : i + 1, summon->name );
  }
  *selection = 0;
  int ch;

  do {
    ch = getch();

    switch (ch) {
      case '1':  *selection = 1;
                 break;
      case '2':  *selection = 2;
                 break;
      case '3':  *selection = 3;
                 break;
      case '4':  *selection = 4;
                 break;
      case '5':  *selection = 5;
                 break;
      case '6':  *selection = 6;
                 break;
      case '7':  *selection = 7;
                 break;
      case '8':  *selection = 8;
                 break;
      case '9':  *selection = 9;
                 break;
      case '0':  *selection = 10;        
                 break;
    } 
  } while ( ch != ESC && ( *selection == 0 || *selection > count ) );
  clear_line( 20, 5);
  for ( size_t i = 0; i < count; ++i )
    clear_line( 21 + i, 10 );
  return;
}

void actions_menu(const char *curr_path) {
  const char *screen_title = "Actions";
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
              actions_dataentry_scr( menu_path( curr_path, screen_title ), case_num );
              break;
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

void actions_dataentry_scr(const char *curr_path, const char *case_num) {
  const size_t n_fields = 4;
  const size_t starty = 6;
  const size_t startx = 25;
  FIELD *field[n_fields];
  FORM *my_form;
  Action_t record;
  int width[] = {  MAX_ACT_DATE, MAX_ACT_TYPE, MAX_ACT_NOTE - 200 };
  int height[] = { 1, 1, 4 };
 
  for ( size_t i = 0; i < n_fields - 1; ++i )
    field[i] = new_field(height[i], width[i], starty + i * 2, startx, 0, 0);
  field[n_fields - 1] = NULL;

  set_field_back( field[0], A_UNDERLINE  );
  field_opts_off( field[0], O_AUTOSKIP   );
  set_field_back( field[1], A_UNDERLINE  );
  field_opts_off( field[1], O_AUTOSKIP   );
  set_field_back( field[2], A_UNDERLINE  );
  field_opts_off( field[2], O_AUTOSKIP   );
  field_opts_off( field[2], O_STATIC     );
  set_max_field(  field[2], MAX_ACT_NOTE );

  my_form = new_form(field);
  post_form(my_form);
  refresh();
  
  mvprintw( 0, 0,   curr_path );
  mvprintw( 4, 10,  "Case Number:   %s", case_num );
  mvprintw( 6, 10,  "Entry Date:    " );
  mvprintw( 8, 10,  "Type:          " );
  mvprintw( 10, 10, "Note:          " );
  mvprintw( 16, 10, "(F2) = Add | (ESC) = Previous Screen" );
  set_visible_fields( field, 1, 3 );
  size_t actions_count = actions_list( case_num );
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
      case KEY_F(1):
        {
          FIELD * curr_fld = current_field( my_form );

          if ( curr_fld == field[1] ) {
            if ( query_select_all_codes_from_action_types()  ) {
              clear_line(20, 10);
              mvprintw( 20, 10, db_get_error_msg() );
            } else {
              const Code_t const * code_ptr;
              size_t count = 0;
              
              mvprintw( 8, 75, "Type Options:" );
              while ( ( code_ptr = get_code_from_result() ) != NULL )      
                mvprintw( 9 + count++, 81, "[%d] %s", code_ptr->code, code_ptr->desc );
              free_code_result();

              int row, col;
              get_cursor_pos( curr_fld, &row, &col );
              move( row, col );
              set_current_field( my_form, curr_fld );
            }
          }
        }
        break;
      case KEY_F(2):
        strncpy( record.case_num, case_num, MAX_CANUM );
        strncpy( record.entry_date, compress_str( field_buffer(field[0], 0) ), MAX_ACT_DATE );
        record.type = atoi( compress_str( field_buffer(field[1], 0) ) );
        strncpy( record.note, field_buffer(field[2], 0), MAX_ACT_NOTE );

        if ( query_add_action( &record ) ) {
          clear_line(20, 10);
          mvprintw( 20, 10, db_get_error_msg() );
        } else {
          clear_fields( field, 0, 2 );
          actions_count++;
          print_action( &record, actions_count );
        }
        move( 6, 25 );
        set_current_field( my_form, field[0] );
        break;
      default:
        form_driver( my_form, ch );
        break;
    }
  } while ( ch != ESC );

  unpost_form( my_form );
  free_form( my_form );
  for ( size_t i = 0; i < n_fields - 1; ++i )
    free_field( field[i] );
  return;
}

size_t actions_list(const char *case_num) {
  size_t count = 0;

  if ( query_select_count_from_actions_for( case_num, &count ) ) {
    clear_line(20, 10);
    mvprintw( 20, 10, db_get_error_msg() );
    move( 6, 25 );
  } else {
    if ( count ) {
      if ( query_select_all_from_actions_for( case_num ) ) {
        clear_line(20, 10);
        mvprintw( 20, 10, db_get_error_msg() );
        move( 6, 25 );
      } else {
        Action_t *act_ptr;
        size_t idx = 0;
  
        mvprintw( 20, 10, "Actions:" );
        while ( ( act_ptr = get_action_from_result() ) != NULL )           
          print_action( act_ptr, ++idx );
      }
    } 
  }
  return count;
}

void print_action(const Action_t *action, size_t idx) {
  char note[51];
 
  strncpy( note, action->note, 51 );
  mvprintw( 21 + idx, 10, "%u. [%s] %s", idx, action->entry_date, note );
  return;
}

void reports_menu(const char *curr_path) {
  const char *screen_title = "Reports";
  int choice;
  void ( *farray[] )( const char* ) = { dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        dummy_menu,
                                        billing_menu,
                                        suggestions_menu
                                  }; 
  initscr();
  raw();
  do {
    choice = 0;
    noecho();
    curs_set(0);
    clear();
    mvprintw( 0, 0, menu_path( curr_path, screen_title ) );
    mvprintw(4, 10, "a. Billing");
    mvprintw(6, 10, "b. Suggestions");
    mvprintw(18, 10, "Select an option from the above menu or press (ESC) for previous screen.");
    refresh();
    choice = getch();
    switch( choice ) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':  
        break;
      case 'a':
      case 'A':  choice = 10;
                 break;  
      case 'b':
      case 'B':  choice = 11;
                 break;
      default:
        break;
    }
    if ( choice >= 0 && choice <= 11 )
      farray[choice]( curr_path );
  } while ( choice != ESC );
  endwin();
  return; 
}

void from_to_selector(const char *curr_path, char **from, char **to, int *action) {
  const size_t n_fields = 3;
  const size_t starty = 6;
  const size_t startx = 25;
  FIELD *field[n_fields];
  FORM *my_form;
  int width[] = {  MAX_FROM_DATE, MAX_TO_DATE };
  int height[] = { 1, 1 };

  initscr();
  curs_set(1);
  cbreak();
  clear();
  noecho();
  keypad(stdscr, TRUE);
 
  for ( size_t i = 0; i < n_fields - 1; ++i )
    field[i] = new_field(height[i], width[i], starty + i * 2, startx, 0, 0);
  field[n_fields - 1] = NULL;

  set_field_back( field[0], A_UNDERLINE  );
  field_opts_off( field[0], O_AUTOSKIP   );
  set_field_back( field[1], A_UNDERLINE  );
  field_opts_off( field[1], O_AUTOSKIP   );

  my_form = new_form(field);
  post_form(my_form);
  refresh();
  
  mvprintw( 0, 0,   curr_path );
  mvprintw( 4, 10,  "Enter period for report:" );
  mvprintw( 6, 10,  "From:          " );
  mvprintw( 8, 10,  "To:            " );
  mvprintw( 10, 10, "(F2) = Execute | (ESC) = Previous Screen" );
  move( 6, 25 );
  set_current_field( my_form, field[0] );

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
        {
          char my_from[MAX_FROM_DATE];
          strncpy( my_from, compress_str(field_buffer( field[0], 0 )), MAX_FROM_DATE ); 
          *from = malloc( strlen( my_from ) + 1 );
          strcpy( *from, my_from );

          char my_to[MAX_TO_DATE];
          strncpy( my_to, compress_str(field_buffer( field[1], 0 )), MAX_TO_DATE ); 
          *to = malloc( strlen( my_to ) + 1 );
          strcpy( *to, my_to );
        }
        break;
      case DEL:
        form_driver(my_form, REQ_DEL_CHAR);
        break;
      default:
        form_driver( my_form, ch );
        break;
    }
  } while ( (ch != ESC) && (ch != KEY_F(2)) );
  *action = ch;
  unpost_form( my_form );
  free_form( my_form );
  for ( size_t i = 0; i < n_fields - 1; ++i )
    free_field( field[i] );
  return;
}

void billing_menu(const char *curr_path) {
  int action;
  char * from_date, * to_date;

  from_to_selector(curr_path, &from_date, &to_date, &action);
  if ( action == KEY_F(2) ) { 
    char * msg;
    size_t result = report_basic_summ_bill( from_date, to_date, &msg );
    if ( result )
      mvprintw( 20, 10, "[!] %s", msg );
    else
      mvprintw( 20, 10, "Report '%s'has been created. ", msg );
    mvprintw( 25, 10, "Press any key to continue..." );
    int ch = getch();
    free( from_date );
    free( to_date );
    free( msg );
  }
  return;
}

void suggestions_menu(const char *curr_path) {
  return;
}

void dummy_menu(const char *curr_path) {
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

void clear_lines(size_t start, size_t end) {
  for (size_t i = start; i < end; i++)
   clear_line( i, 0 );
  return;
}

void print_line(char line[], size_t row, size_t col) {
  strncpy( scr_line, line, MAX_SCR_LINE );
  mvprintw( row, col, scr_line );
  return;
}

void get_cursor_pos(const FIELD const * field, int* row, int* col) {
  int rows, cols, nrow, nbuf, trow, tcol;
  field_info(field, &rows, &cols,
             &trow, &tcol, &nrow, &nbuf);
  *row = trow;
  *col = tcol;
  return;
}
