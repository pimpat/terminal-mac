#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "lib_db.h"

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: \n", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char sql1[200];
   char sql2[200];
   const char* data = "Callback function called";

// open data.db & user.db
   rc = sqlite3_open("pimtemp.db", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
// create sql
	sprintf(sql1, "%s", "CREATE TABLE IF NOT EXISTS data (uuid int_64t, dataid int_64t, dataname string, data blob, owner int_64t,last modifier long);");
	sprintf(sql2, "%s", "CREATE TABLE IF NOT EXISTS user (userid int_64t, username string, password string);");
   	rc = sqlite3_exec(db, sql1, callback, 0, &zErrMsg);
   	rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
/*
sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, 0); 

srand(time(NULL));
for(int i=1;i<100001;i++){
   //char name[10]="";
   //char password[10]="";
   char fdata[60]="";
   char ndata[10]="";
   long msec;
   //random_user(name);
   //random_password(password);
   random_dataname(ndata);
   random_data(fdata);
   msec = current_timestamp(msec);
   printf("msec: %ld\n",msec);

   if(i<2001){
   char name[10]="";
   char password[10]="";
   random_user(name);
   random_password(password);
   insert_user(sql2,i,name,password);
   rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   }

   insert_data(sql1,i,i,ndata,fdata,i,msec);
   rc = sqlite3_exec(db, sql1, callback, 0, &zErrMsg);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
}

sqlite3_exec(db, "END TRANSACTION", NULL, NULL, 0); 
*/
	clock_t start, end;
        double total;
        char sql[200];
        start = clock();
	select_all(sql);
	printf("sql value: %s\n",sql);
   	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
        if(rc != SQLITE_OK){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }
	end = clock();
        printf("SQL prepared OK\n");
	total = (double)(end-start)/CLOCKS_PER_SEC*1000;
        printf("total time is %f.\n",total);
        printf("start time is %lu.\n",start);
        printf("end time is %lu.\n",end);

/*
// execute sql
   rc = sqlite3_exec(db, sql1, callback, 0, &zErrMsg);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table created successfully\n");
   }
   rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table created successfully\n");
   }
*/
   sqlite3_close(db);
   return 0;
}
