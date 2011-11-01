#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "reports.h" 
#include "records.h"


int spawn(char **args);

extern size_t query_select_from_summons_bill_for(const char *from_date, const char *to_date); 
extern const SummonBill_t const * get_summon_bill_from_result(void);
extern void free_summon_bill_result(void);
extern const char* db_get_error_msg(void);
extern DBPARAMS * get_conf(void); 
 
size_t report_basic_summ_bill(const char *from_date, const char *to_date, char **msg) {
  DBPARAMS * params = get_conf(); 
  char * args[3];
  char report_name[BUFSIZ];

  sprintf( report_name, "Basic_Summ_Bill_From_%s_To_%s.txt", from_date, to_date );

  args[0] = params->writer;
  args[1] = report_name;
  args[2] = NULL;

 
  FILE * fp = fopen( report_name, "w" );
  if ( fp != NULL ) { 
    if ( query_select_from_summons_bill_for( from_date, to_date ) ) {
      char err_msg[MAX_ERR_MSG];
    
      strncpy( err_msg, db_get_error_msg(), MAX_ERR_MSG );
      *msg = malloc( strlen( err_msg ) + 1 );
      strcpy( *msg, err_msg );
      return 1;
    } else {
      const SummonBill_t const * summ_bill_ptr;
      char str_line[MAX_LINE_SUMM_BILL_REP];

      while( ( summ_bill_ptr = get_summon_bill_from_result() ) != NULL ) {
        int output_len = snprintf( str_line, MAX_LINE_SUMM_BILL_REP, " %s %s %s %s %f", 
                                  summ_bill_ptr->summon_info.case_num,
                                  summ_bill_ptr->summon_info.name,
                                  summ_bill_ptr->summon_info.summon_date,
                                  summ_bill_ptr->city_name,
                                  summ_bill_ptr->amount
                                );
        int result = fputs( str_line, fp );
        if ( result == EOF ) {
          char *err_msg = "An error occurred when writing data to report file.";
          
          *msg = malloc( strlen( err_msg ) + 1 );
          strcpy( *msg, err_msg );
          return 1;
        }
      }
      fclose( fp );
      free_summon_bill_result();
      *msg = malloc( strlen( report_name ) + 1 );
      strcpy( *msg, report_name );
      spawn(args);
    }
  } else {
    char *err_msg = "Unable to create report.";

    *msg = malloc( strlen( err_msg ) + 1 );
    strcpy( *msg, err_msg );
    return 1;
  }
  return 0;
}

int spawn(char ** args) {

  pid_t child_pid;

  child_pid = fork();
  if ( child_pid != 0 )
    return child_pid;
  else { 
    execv( args[0], args );
  }
}
