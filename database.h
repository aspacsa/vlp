int connect_to_db(void);
int init_db(void);
void close_db(void);
const char* get_db_cnf(void);
int set_db_cnf(const char *server, const char *user, const char *pass, const char *db);

