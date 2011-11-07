#include "database.h"
#include "records.h"
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

DBPARAMS param;
char params_list[PARAMS_SIZE];
char error_message[100];

Summon_t * summ_set[MAX_SUMM_SET] = {NULL};
Summon_t ** summ_ptr = summ_set;   // = &summ_set[0]
size_t summ_set_count = 0;

Action_t * act_set[MAX_ACT_SET] = {NULL};
Action_t ** act_ptr = act_set;
size_t act_set_count = 0;

Code_t * code_set[MAX_CODE_SET] = {NULL};
Code_t ** code_ptr = code_set;
size_t code_set_count = 0;

SCode_t * scode_set[MAX_SCODE_SET] = {NULL};
SCode_t ** scode_ptr = scode_set;
size_t scode_set_count = 0;

SummonBill_t * summ_bill_set[MAX_SUMM_BILL_SET] = {NULL};
SummonBill_t ** summ_bill_ptr = summ_bill_set;
size_t summ_bill_set_count = 0;

int read_db_cnf();
int write_db_log(char *line);
DBPARAMS * get_conf(void);
void prepare_line(char*);
size_t query_create_new_case(Case_t *);
size_t query_update_case(Case_t *);
size_t query_delete_case(const char *case_num);
size_t query_select_all_from_summons_for(const char *case_num);
const Summon_t const * get_summon_from_result(void);
void free_summon_result(void);
size_t query_update_summon(Summon_t*);
size_t query_delete_summon(size_t summ_id);
size_t query_select_all_from_actions_for(const char *case_num);
const Action_t const * get_action_from_result(void);
void free_action_result(void);
size_t query_add_action(const Action_t const * record);
size_t query_select_all_codes_from_case_status(void);
size_t query_select_all_codes_from_summon_status(void); 
size_t query_select_all_codes_from_summon_reasons(void);
size_t query_select_all_codes_from_action_types(void); 
size_t query_select_all_codes_for(char * query);
const Code_t const * get_code_from_result(void);
void free_code_result(void);

size_t query_select_from_summons_bill_for(const char *from_date, const char *to_date);
const SummonBill_t const * get_summon_bill_from_result(void);
void free_summon_bill_result(void); 

size_t query_select_all_codes_from_action_types(void);
const SCode_t const * get_scode_from_result(void);
void free_scode_result(void);

size_t db_error_number(void);
const char* db_error_message(void);
size_t query_select_count_from_case_for(const char *case_num, size_t *count);
size_t query_select_count_from_actions_for(const char *case_num, size_t *count);
const char* db_get_error_msg(void);

extern size_t dev_mode;

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
    } else if ( strcmp( token, WRITER_PARAM ) == 0 ) {
      strncpy( param.writer, strtok( NULL, delimiter ), MAX_PARAM );
    }
  }
  fclose( fp );
  return 0;
}

int write_db_log(char *line ) {
  FILE *fp;
  time_t eventtime;
  
  time(&eventtime);
  fp = fopen( DB_LOG_FILE, "a" );
  if ( fp == NULL )
    return 1;
  fprintf( fp, "\n\nEntry: %s{\n %s \n}", ctime(&eventtime), line );

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

DBPARAMS * get_conf(void) {
  return &param;
}

void prepare_line(char *line) {
  strcpy( line, strtok( line, "\n" ) );
}

size_t query_create_new_case(Case_t *ptr) {
  char query[BUFSIZ];

  sprintf( query, "INSERT INTO case_headers (CaseNumber, CivilNumber,"
                                  " PhysicalAdd, PostalAdd,"
                                  " Status, DeliveryDate)"
                                  " VALUES ( TRIM('%s'), TRIM('%s'),"
                                  "          TRIM(SUBSTRING('%s' FROM 1 FOR 45)),"
                                  "          TRIM(SUBSTRING('%s' FROM 1 FOR 45)),"
                                  "          %d, TRIM('%s') )", 
                                  ptr->number, ptr->civil, ptr->physical_add,
                                  ptr->postal_add, ptr->status, ptr->delivery_date );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_update_case(Case_t *ptr) {
  char query[BUFSIZ];

  sprintf( query, "UPDATE case_headers SET CivilNumber = TRIM('%s'),"
                  "                       PhysicalAdd = TRIM(SUBSTRING('%s' FROM 1 FOR 45)),"
                  "                       PostalAdd = TRIM(SUBSTRING('%s' FROM 1 FOR 45)),"
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

size_t query_select_all_from_case_for(const char *case_num, Case_t *ptr) {
  char query[BUFSIZ];

  sprintf( query, "SELECT CaseNumber, CivilNumber,"
                 "       TRIM(SUBSTRING(PhysicalAdd FROM 1 FOR 45)),"
                 "       TRIM(SUBSTRING(PostalAdd FROM 1 FOR 45)),"
                 "       Status, DeliveryDate"
                 " FROM case_headers"
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
    strncpy( ptr->number, row[0], MAX_CANUM );
    strncpy( ptr->civil, row[1], MAX_CINUM );
    strncpy( ptr->physical_add, row[2], MAX_PHYADD );
    strncpy( ptr->postal_add, row[3], MAX_POSADD );
    ptr->status = atoi( row[4] );
    strncpy( ptr->delivery_date, row[5], MAX_DELDATE );
    mysql_free_result( res );
    if ( dev_mode ) {
      char buffer[BUFSIZ];
      sprintf( buffer, "Result: '%s', '%s', '%s', '%s', %d, '%s'", 
                       ptr->number, ptr->civil, ptr->physical_add,
                       ptr->postal_add, ptr->status, ptr->delivery_date, row[4]
             );
      write_db_log( buffer );
    }
  } 
  return 0;
}

size_t query_select_all_from_summons_for(const char *case_num) {
  char query[BUFSIZ];
  size_t result = 0;
  size_t count = 0;

  summ_ptr = summ_set;
  for ( int i = 0; i <= MAX_SUMM_SET - 1; ++i )
    summ_set[i] = NULL;

  sprintf( query, "SELECT id, TRIM(CaseNumber), TRIM(Name)," 
                  " Status, Reason, TRIM(CityCode), SummonDate"
                  " FROM summons WHERE CaseNumber = '%s'", case_num );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result(conn);
    while( ( ( row = mysql_fetch_row(res) ) != NULL ) && ( count <= MAX_SUMM_SET ) ) {
      Summon_t *summPtr = malloc( sizeof(Summon_t) );
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
      summ_set[count++] = summPtr;
    }
    mysql_free_result( res );
  }
  summ_set_count = count;
  return result;
}

const Summon_t const * get_summon_from_result(void) {
  return *summ_ptr++; 
}

void free_summon_result(void) {
  size_t i = 0;

  summ_ptr = summ_set;
  for ( i = 0; i <= summ_set_count - 1; ++i )
    free( *summ_ptr++ );
  if ( dev_mode ) {
    char msg[40];
    sprintf( msg, "Elements freed from summ_set: %u", i );
    write_db_log( msg );
  }
  summ_set_count = 0;
  return;
}

size_t query_update_summon(Summon_t *record) {
  char query[BUFSIZ];

  if ( record->id > 0 ) {
    sprintf( query, "UPDATE summons" 
                    " SET CaseNumber = TRIM('%s'),"
                    "     Name = TRIM(SUBSTRING('%s' FROM 1 FOR 45)),"
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
                    " VALUES ( TRIM('%s'), TRIM(SUBSTRING('%s' FROM 1 FOR 45)), %d, %d, TRIM('%s'), TRIM('%s') )", 
                             record->case_num, record->name, record->status, 
                             record->reason, record->city_code, record->summon_date
           );
  }
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_delete_summon(size_t summ_id) {
  char query[BUFSIZ];

  sprintf( query, "DELETE FROM summons"
                  " WHERE id = %u", summ_id );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_select_from_summons_bill_for(const char *from_date, const char *to_date) {
  char query[BUFSIZ];
  size_t result = 0;
  size_t count = 0;

  summ_bill_ptr = summ_bill_set;
  for ( int i = 0; i <= MAX_SUMM_BILL_SET - 1; ++i )
    summ_bill_set[i] = NULL;

  sprintf( query, 
           "SELECT s.CaseNumber, s.Name, s.SummonDate, c.CityName, IFNULL(c.First, 0.00) AS Amount"
           " FROM summons s LEFT OUTER JOIN city_rates c"
           " ON s.CityCode = c.CityCode"
           " WHERE s.Status = 30 AND s.Reason = 50"
           " AND s.SummonDate BETWEEN '%s' AND '%s'"
           " AND s.id = (SELECT MIN(id) FROM summons WHERE  CaseNumber = s.CaseNumber AND CityCode = s.CityCode)"
           " UNION"
           " SELECT s.CaseNumber, s.Name, s.SummonDate, c.CityName, IFNULL(c.Second, 0.00) AS Amount"
           " FROM summons s LEFT OUTER JOIN city_rates c"
           " ON s.CityCode = c.CityCode"
           " WHERE s.Status = 30 AND s.Reason = 50 "
           " AND s.SummonDate BETWEEN '%s' AND '%s'"
           " AND s.id <> (SELECT MIN(id) FROM summons WHERE  CaseNumber = s.CaseNumber AND CityCode = s.CityCode)"
           " ORDER BY CaseNumber", from_date, to_date, from_date, to_date );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result(conn);
    while( ( ( row = mysql_fetch_row(res) ) != NULL ) && ( count <= MAX_SUMM_BILL_SET ) ) {
      SummonBill_t *summBill_Ptr = malloc( sizeof(SummonBill_t) );
      if ( summBill_Ptr == NULL ) {
        result = 1;
        break;
      } else {
        strncpy( summBill_Ptr->summon_info.case_num, row[0], MAX_CANUM );
        strncpy( summBill_Ptr->summon_info.name, row[1], MAX_SUMM_NAME );
        strncpy( summBill_Ptr->summon_info.summon_date, row[2], MAX_SUMM_DATE );
        strncpy( summBill_Ptr->city_name, row[3], MAX_CITY_NAME );
        summBill_Ptr->amount = atoi( row[4] );
      }
      summ_bill_set[count++] = summBill_Ptr;
    }
    mysql_free_result( res );
  }
  summ_bill_set_count = count;
  return result;
}

const SummonBill_t const * get_summon_bill_from_result(void) {
  return *summ_bill_ptr++; 
}

void free_summon_bill_result(void) {
  size_t i = 0;

  summ_bill_ptr = summ_bill_set;
  for ( i = 0; i <= summ_bill_set_count - 1; ++i )
    free( *summ_bill_ptr++ );
  if ( dev_mode ) {
    char msg[40];
    sprintf( msg, "Elements freed from summ_bill_set: %u", i );
    write_db_log( msg );
  }
  summ_bill_set_count = 0;
  return;
}


size_t query_add_action(const Action_t const * record) {
  char query[BUFSIZ];

  sprintf( query, "INSERT INTO actions (CaseNumber, Note, EntryAt, Type, CreatedAt)"
                  " VALUES ('%s', '%s', '%s', %d, NOW() ) ", 
                  record->case_num, record->note, record->entry_date, record->type
         );
  if ( dev_mode ) 
    write_db_log( query );
  db_error_number();
  return mysql_query( conn, query );
}

size_t query_select_all_from_actions_for(const char *case_num) {
  char query[BUFSIZ];
  size_t result = 0;
  size_t count = 0;

  act_ptr = act_set;
  for ( int i = 0; i <= MAX_ACT_SET - 1; ++i )
    act_set[i] = NULL;

  sprintf( query, "SELECT id, CaseNumber, Note, Type, EntryAt"
                  " FROM actions WHERE CaseNumber = '%s' ORDER BY EntryAt DESC", case_num );
  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result(conn);
    while( ( ( row = mysql_fetch_row(res) ) != NULL ) && ( count <= MAX_ACT_SET ) ) {
      Action_t *actPtr = malloc( sizeof(Action_t) );
      if ( actPtr == NULL ) {
        result = 1;
        break;
      } else {
        actPtr->id = atoi( row[0] );
        strncpy( actPtr->case_num, row[1], MAX_CANUM );
        strncpy( actPtr->note, row[2], MAX_ACT_NOTE );
        actPtr->type =  atoi( row[3] );
        strncpy( actPtr->entry_date, row[4], MAX_ACT_DATE );
      }
      act_set[count++] = actPtr;
    }
    mysql_free_result( res );
  }
  act_set_count = count;
  return result;
}

const Action_t const * get_action_from_result(void) {
  return *act_ptr++;
}

void free_action_result(void) {
  size_t i = 0;

  act_ptr = act_set;
  for ( i = 0; i <= act_set_count - 1; ++i )
    free( *act_ptr++ );
  if ( dev_mode ) {
    char msg[40];
    sprintf( msg, "Elements freed from act_set: %u", i );
    write_db_log( msg );
  }
  act_set_count = 0;
  return;

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

size_t query_select_count_from_actions_for(const char *case_num, size_t *count) {
  char query[BUFSIZ];

  sprintf( query, "SELECT COUNT(id) FROM actions"
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

size_t query_select_all_codes_from_case_status(void) {
  return query_select_all_codes_for( "SELECT id, description FROM case_status" 
                                     " ORDER BY id"
                                   );
}

size_t query_select_all_codes_from_summon_status(void) {
  return query_select_all_codes_for( "SELECT id, description FROM summon_status" 
                                     " ORDER BY id"
                                   );
}

size_t query_select_all_codes_from_summon_reasons(void) {
  return query_select_all_codes_for( "SELECT id, description FROM summon_reasons" 
                                     " ORDER BY id"
                                   );
}

size_t query_select_all_codes_from_action_types(void) {
  return query_select_all_codes_for( "SELECT id, Description FROM action_types"
                                     " ORDER BY id"
                                   );
}

size_t query_select_all_codes_from_city_rates(void) {
  char *query = "SELECT CityCode, CityName FROM city_rates ORDER BY CityName";
  size_t result = 0;
  size_t count = 0;

  scode_ptr = scode_set;
  for ( int i = 0; i <= MAX_SCODE_SET - 1; ++i )
    scode_set[i] = NULL;

  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result( conn );
    while( ( ( row = mysql_fetch_row( res ) ) != NULL ) && ( count <= MAX_SCODE_SET ) ) {
      SCode_t *scodePtr = malloc( sizeof(SCode_t) );
      if ( scodePtr == NULL ) {
        result = 1;
      } else {
        strncpy( scodePtr->code, row[0], MAX_SCODE_CODE );
        strncpy( scodePtr->name, row[1], MAX_SCODE_NAME );
        if ( dev_mode ) {
          char row_buf[BUFSIZ];
          sprintf(row_buf, "Result: CityCode{'%s'}, CityName{'%s'}", 
                                    row[0], row[1]
                 );
          write_db_log( row_buf );
        }
      }
      scode_set[count++] = scodePtr;
    }
    mysql_free_result( res );
  }
  scode_set_count = count;
  return result;
}

const SCode_t const * get_scode_from_result(void) {
  return *scode_ptr++; 
}

void free_scode_result(void) {
  size_t i = 0;

  scode_ptr = scode_set;
  for ( i = 0; i <= scode_set_count - 1; ++i )
    free( *scode_ptr++ );
  if ( dev_mode ) {
    char msg[40];
    sprintf( msg, "Elements freed from scode_set: %u", i );
    write_db_log( msg );
  }
  scode_set_count = 0;
  return;
}

size_t query_select_all_codes_for(char * query) {
  size_t result = 0;
  size_t count = 0;

  code_ptr = code_set;
  for ( int i = 0; i <= MAX_CODE_SET - 1; ++i )
    code_set[i] = NULL;

  if ( dev_mode )
    write_db_log( query );
  db_error_number();
  size_t error = mysql_query( conn, query );
  if ( error ) {
    return error;
  } else {
    res = mysql_use_result( conn );
    while( ( ( row = mysql_fetch_row( res ) ) != NULL ) && ( count <= MAX_CODE_SET ) ) {
      Code_t *codePtr = malloc( sizeof(Code_t) );
      if ( codePtr == NULL ) {
        result = 1;
      } else {
        codePtr->code = atoi( row[0] );
        strncpy( codePtr->desc, row[1], MAX_CODE_DESC );
      }
      code_set[count++] = codePtr;
    }
    mysql_free_result( res );
  }
  code_set_count = count;
  return result;
}

const Code_t const * get_code_from_result(void) {
  return *code_ptr++; 
}

void free_code_result(void) {
  size_t i = 0;

  code_ptr = code_set;
  for ( i = 0; i <= code_set_count - 1; ++i )
    free( *code_ptr++ );
  if ( dev_mode ) {
    char msg[40];
    sprintf( msg, "Elements freed from code_set: %u", i );
    write_db_log( msg );
  }
  code_set_count = 0;
  return;
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
