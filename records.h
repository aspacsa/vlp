#define MAX_CANUM 11
#define MAX_CINUM 16
#define MAX_PHYADD 51
#define MAX_POSADD 51
#define MAX_STATUS 2
#define MAX_DELDATE 11

#define MAX_SUMM_NAME 45
#define MAX_SUMM_STATUS 2
#define MAX_SUMM_REASON 2
#define MAX_SUMM_CITY 3
#define MAX_SUMM_DATE 11
#define MAX_SUMM_SET 10
#define MAX_SUMM_BILL_SET 250

#define MAX_CITY_NAME 45

#define MAX_ACT_NOTE 255
#define MAX_ACT_TYPE 2
#define MAX_ACT_DATE 11
#define MAX_ACT_SET 40

#define MAX_FROM_DATE 11
#define MAX_TO_DATE 11

#define MAX_CODE_DESC 46
#define MAX_CODE_SET 40

#define MAX_SCODE_NAME 46
#define MAX_SCODE_CODE 4
#define MAX_SCODE_SET 80

#define VLP_CNF_FILE "vlp.cnf"
#define DB_LOG_FILE "db.log"
#define SERVER_PARAM "server"
#define USER_PARAM "user"
#define PASS_PARAM "pass"
#define DB_PARAM "db"
#define WRITER_PARAM "writer"
#define MAX_PARAM_LINE 81
#define MAX_PARAM 81
#define PARAMS_SIZE 320
#define MAX_ERR_MSG 520

typedef struct {
  char number[MAX_CANUM];
  char civil[MAX_CINUM];
  char physical_add[MAX_PHYADD];
  char postal_add[MAX_POSADD];
  int status;
  char delivery_date[MAX_DELDATE];
} Case_t;

typedef struct {
  int id;
  char case_num[MAX_CANUM];
  char name[MAX_SUMM_NAME];
  int status;
  int reason;
  char city_code[MAX_SUMM_CITY];
  char summon_date[MAX_SUMM_DATE];
} Summon_t;

typedef struct {
  int id;
  char case_num[MAX_CANUM];
  char note[MAX_ACT_NOTE];
  int type;
  char entry_date[MAX_ACT_DATE];
} Action_t;

typedef struct {
  int code;
  char desc[MAX_CODE_DESC];
} Code_t;

typedef struct {
  char code[MAX_SCODE_CODE];
  char name[MAX_SCODE_NAME];
} SCode_t;

typedef struct {
  Summon_t summon_info;
  char city_name[MAX_CITY_NAME];
  double amount;
} SummonBill_t;

typedef struct {
  char server[MAX_PARAM];
  char user[MAX_PARAM];
  char pass[MAX_PARAM];
  char db[MAX_PARAM];
  char writer[MAX_PARAM];
} DBPARAMS;

