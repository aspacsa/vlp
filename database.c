#include "database.h"
#include "records.h"
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VLP_CNF_FILE "vlp.cnf"
#define DB_LOG_FILE "db.log"
#define SERVER_PARAM "server"
#define USER_PARAM "user"
#define PASS_PARAM "pass"
#define DB_PARAM "db"
#define MAX_PARAM_LINE 80
#define MAX_PARAM 21
#define PARAMS_SIZE 320

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

typedef struct params {
  char server[MAX_PARAM];
  char user[MAX_PARAM];
  char pass[MAX_PARAM];
  char db[MAX_PARAM];
} DBPARAMS;

DBPARAMS param;
char params_list[PARAMS_SIZE];
char error_message[100];

int read_db_cnf();
int write_db_log(char *line);
void prepare_line(char*);
size_t query_create_new_case(CASE*);
size_t query_update_case(CASE*);
size_t query_delete_case(const char *case_num);
size_t query_select_all_from_summons_for(const char *case_num, SUMMON *summ_set[], int *count); 
size_t query_update_summon(SUMMON*);
size_t db_error_number(void);
const char* db_error_message(void);
size_t query_select_count_from_case_for(const char *case_num, size_t *count);
const char* db_get_error_msg(void);

extern dev_mode;

int init_db(void) {
  conn = mysql_init( NULL );
  return 0;
}

int connect_to_db(void) {
  int result = 0;
    
  if ( !mysql_real_connect( conn, param.server, param.user, param.pass, param.db, 0, NULL, 0 ) ) {
    fprintf( stderr, "\n %s\n", mysql_error( conn ) );
    result = 1;
  }
  return result;
}

void close_db(void) {
  mysql_close( conn );
}

const char* get_db_cnf(void) {
  strcpy( params_list, "\n\nServer: [" );
  strcat( params_list, param.server );
  strcat( params_list, "]\n" );
  strcat( params_list, "User:   [" );
  strcat( params_list, param.user );
  strcat( params_list, "]\n" );
  strcat( params_list, "DB:     [" );
  strcat( params_list, param.db );
  strcat( params_list, "]\n" );
  return params_list;
}

int read_db_cnf() {
  FILE *fp;
  char line[MAX_PARAM_LINE];
  char *delimiter = "\t";
  char *token = NULL;
 
  fp = fopen( VLP_CNF_FILE, "r" );
  if ( fp == NULL )
    return 1;
  while ( fgets( line, MAX_PARAM_LINE, fp ) != NULL ) {
    prepare_line( line );
    token = strtok( line, delimiter );
    if ( strcmp(token, SERVER_PARAM ) == 0) {
      strncpy( param.server, strtok(NULL, delimiter ), MAX_PARAM );
     } else if ( strcmp(token, USER_PARAM ) == 0 ) {
      strncpy( param.user, strtok( NULL, delimiter ), MAX_PARAM );
    } else if ( strcmp( token, PASS_PARAM ) == 0 ) {
      strncpy( param.pass, strtok( NULL, delimiter ), MAX_PARAM );
    } else if ( strcmp( token, DB_PARAM ) == 0 ) {
      strncpy( param.db, strtok( NULL, delimiter ), MAX_PARAM );
    }
  }
  fclose( fp );
  return 0;
}

int write_db_log(char *line ) {
  FILE *fp;
  
  fp = fopen( DB_LOG_FILE, "a" );
  if ( fp == NULL )
    return 1;
  fprintf( fp, "\n\nEntry: {\n %s \n}", line );

  fclose( fp );
  return 0;
}

int set_db_cnf(const char *server, const char *user, const char *pass, const char *db) {
  strncpy( param.server, server, MAX_PARAM );
  strncpy( param.user, user, MAX_PARAM );
  strncpy( param.pass, pass, MAX_PARAM );
  strncpy( param.db, db, MAX_PARAM );
  return 0;
}

void prepare_line(char *line) {
  strcpy( line, strtok( line, "\n" ) );
}

size_t query_create_new_case(CASE *ptr) {
  char query[BUFSIZ];

  sprintf( query, "INSERT INTO case_headers (CaseNumber, CivilNumber,"
                                  " PhysicalAdd, PostalAdd,"
                                  " Status, DeliveryDate)"
                                  " VALUES ( TRIM('%s'), TRIM('%s'),"
                                  "          TRIM('%s'), TRIM('%s'), "
                                  "          %d, TRIM('%s') )", 
                                  ptr->number, ptr->civil, ptr->physical_add,
                                  ptr->postal_add, ptr->status, ptr->delivery_date );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_update_case(CASE *ptr) {
  char query[BUFSIZ];

  sprintf( query, "UPDATE case_headers SET CivilNumber = TRIM('%s'),"
                  "                       PhysicalAdd = TRIM('%s'),"
                  "                       PostalAdd = TRIM('%s'),"
                  "                       Status = %d,"
                  "                       DeliveryDate = TRIM('%s')"
                  " WHERE CaseNumber = '%s'", ptr->civil, ptr->physical_add,
                                              ptr->postal_add, ptr->status,
                                              ptr->delivery_date, ptr->number);
  if ( dev_mode ) 
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_delete_case(const char *case_num) {
  char query[BUFSIZ];

  sprintf( query, "DELETE FROM case_headers"
                  " WHERE CaseNumber = '%s'", case_num );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_select_all_from_case_for(const char *case_num, CASE *ptr) {
  char query[BUFSIZ];

  sprintf( query, "SELECT CaseNumber, CivilNumber,"
                 "       PhysicalAdd, PostalAdd,"
                 "       Status, DeliveryDate"
                 " FROM case_headers"
                 " WHERE CaseNumber = '%s'", case_num );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result( conn );
    row = mysql_fetch_row( res );
    strncpy( ptr->number, row[0], MAX_CANUM );
    strncpy( ptr->civil, row[1], MAX_CINUM );
    strncpy( ptr->physical_add, row[2], MAX_PHYADD );
    strncpy( ptr->postal_add, row[3], MAX_POSADD );
    ptr->status = (char)*row[4];
    strncpy( ptr->delivery_date, row[5], MAX_DELDATE );
    mysql_free_result( res );
  }
  return 0;
}

size_t query_select_all_from_summons_for(const char *case_num, SUMMON *summ_set[], int *count) {
  char query[BUFSIZ];
  size_t result = 0;
  *count = 0;

  sprintf( query, "SELECT id, CaseNumber, Name, Status, Reason, CityCode, SummonDate"
                 " FROM summons WHERE CaseNumber = '%s'", case_num );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result(conn);
    while( ( ( row = mysql_fetch_row(res) ) != NULL ) && ( *count <= MAX_SUMM_SET ) ) {
      SUMMON *summPtr = malloc( sizeof(SUMMON) );
      if ( summPtr == NULL ) {
        result = 1;
        break;
      } else {
        summPtr->id = atoi( row[0] );
        strncpy( summPtr->case_num, row[1], MAX_CANUM );
        strncpy( summPtr->name, row[2], MAX_SUMM_NAME );
        summPtr->status = atoi( row[3] );
        summPtr->reason = atoi( row[4] );
        strncpy( summPtr->city_code, row[5], MAX_SUMM_CITY );
        strncpy( summPtr->summon_date, row[6], MAX_SUMM_DATE );
      }
      summ_set[*count++] = summPtr;
    }
    mysql_free_result( res );
  }
  return result;
}

size_t query_update_summon(SUMMON *record) {
  char query[BUFSIZ];

  if ( record->id > 0 ) {
    sprintf( query, "UPDATE summons" 
                    " SET CaseNumber = TRIM('%s'),"
                    "     Name = TRIM('%s'),"
                    "     Status = %d,"
                    "     Reason = %d,"
                    "     CityCode = TRIM('%s'),"
                    "     SummonDate = TRIM('%s')"
                    " WHERE id = %d", record->case_num, record->name,
                                      record->status, record->reason,
                                      record->city_code, record->summon_date,
                                      record->id
           );
  } else {
    sprintf( query, "INSERT INTO summons (CaseNumber, Name, Status, Reason, CityCode, SummonDate)"
                    " VALUES ( TRIM('%s'), TRIM('%s'), %d, %d, TRIM('%s'), TRIM('%s') )", 
                             record->case_num, record->name, record->status, 
                             record->reason, record->city_code, record->summon_date
           );
  }
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_select_count_from_case_for(const char *case_num, size_t *count) {
  char query[BUFSIZ];

  sprintf( query, "SELECT COUNT(CaseNumber) FROM case_headers"
                 " WHERE CaseNumber = '%s'", case_num );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result( conn );
    row = mysql_fetch_row( res );
    size_t c = atoi( row[0] );
    *count = c;
    mysql_free_result( res );
  }
  return 0;
}

size_t db_error_number(void) { 
  return mysql_errno(conn); 
}

const char* db_error_message(void) {
  return mysql_error(conn);
}

const char* db_get_error_msg(void) { 
  sprintf(error_message, "Error: [%u] %s", mysql_errno(conn), mysql_error(conn) );
  return error_message;
}
