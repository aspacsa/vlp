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


typedef struct {
  char number[MAX_CANUM];
  char civil[MAX_CINUM];
  char physical_add[MAX_PHYADD];
  char postal_add[MAX_POSADD];
  int status;
  char delivery_date[MAX_DELDATE];
} CASE;

typedef struct {
  int id;
  char case_num[MAX_CANUM];
  char name[MAX_SUMM_NAME];
  int status;
  int reason;
  char city_code[MAX_SUMM_CITY];
  char summon_date[MAX_SUMM_DATE];
} SUMMON;

