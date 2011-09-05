#define MAX_CANUM 11
#define MAX_CINUM 16
#define MAX_PHYADD 51
#define MAX_POSADD 51
#define MAX_STATUS 2
#define MAX_DELDATE 11

#define MAX_SUMM_NAME 51
#define MAX_SUMM_STATUS 2
#define MAX_SUMM_REASON 2
#define MAX_SUMM_CITY 3
#define MAX_SUMM_DATE 11
#define MAX_SUMM_SET 10

#define MAX_ACT_NOTE 255
#define MAX_ACT_TYPE 2
#define MAX_ACT_DATE 11
#define MAX_ACT_SET 40

#define MAX_CODE_DESC 45
#define MAX_CODE_SET 40


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


