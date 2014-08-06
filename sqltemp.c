#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "lib_db.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
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
   int q_cnt=103000,q_size=200,ind=0;
   //q_cnt: number of queries
   //qsize: length of sql
   //ind: pointer
   char **queries = malloc(sizeof(char) * q_cnt * q_size); 
   sqlite3_stmt *stmt;

// open data.db & user.db
   rc = sqlite3_open("temp.db", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }

// create sql
	queries[ind++] = "CREATE TABLE IF NOT EXISTS data (uuid int_64t, dataid int_64t, dataname string, data blob, owner int_64t,last modifier long)"; 
	rc = sqlite3_exec(db,queries[ind-1],callback,0,0);
	queries[ind++] = "CREATE TABLE IF NOT EXISTS user (userid int_64t, username string, password string)";
	rc = sqlite3_exec(db,queries[ind-1],callback,0,0);
/*
sqlite3_config("SQLITE_CONFIG_SINGLETHREAD");
//sqlite3_config("SQLITE_CONFIG_MULTITHREAD");
//sqlite3_config("SQLITE_CONFIG_SERIALIZED");
sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, 0);

srand(time(NULL));
char sql[200];
for(int i=1;i<100001;i++){
   char fdata[60]="";
   char ndata[10]="";
   long msec;
   random_dataname(ndata);
   random_data(fdata);
   msec = current_timestamp(msec);
   printf("msec: %ld\n",msec);
   sql[0] = '\0';

   if(i<2001){
   char name[10]="";
   char password[10]="";
   random_user(name);
   random_password(password);
   insert_user(sql,i,name,password);
   queries[ind++] = sql;
   rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
	printf("SQL prepared OK\n");
        rc = sqlite3_step(stmt);
   }
   
   sql[0] = '\0';
   insert_data(sql,i,i,ndata,fdata,i,msec);
   queries[ind++] = sql;
   rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
        printf("SQL prepared OK\n");
        rc = sqlite3_step(stmt);
}

sqlite3_exec(db, "END TRANSACTION", NULL, NULL, 0);
*/
	clock_t start, end;
	double total;
	queries[ind++] = "select * from user";
	start = clock();
	rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
 	if(rc != SQLITE_OK){
   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
	}
	printf("SQL prepared OK\n");

	do {
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW) 
		//printf("uuid is %d.\tlast modifier is %s.\n",sqlite3_column_int(stmt,0),sqlite3_column_text(stmt,5));
		printf("userid is %d.\tlast username is %s.\n",sqlite3_column_int(stmt,0),sqlite3_column_text(stmt,1));
	} while(rc == SQLITE_ROW);
	end = clock();
	total = (double)(end-start)/CLOCKS_PER_SEC*1000;
	printf("total time is %f.\n",total);
	printf("start time is %lu.\n",start);
	printf("end time is %lu.\n",end);
   	sqlite3_close(db);
   	return 0;
}
