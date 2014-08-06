extern void insert_data(char* sql,int uuid,int dataid,char* dataname,char* data,int owner,long msec);
extern void insert_user(char* sql,int userid,char* username,char* pass);
extern void append(char* s, char c);
extern void random_user(char* x);
extern void random_password(char* x);
extern void random_dataname(char* x);
extern void random_data(char* x);
extern long current_timestamp(long msec);
extern void select_all(char* sql);
