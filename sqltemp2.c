#include <stdio.h>
#include <string.h>
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
	rc = sqlite3_exec(db,queries[ind-1],0,0,0);
	queries[ind++] = "CREATE TABLE IF NOT EXISTS user (userid int_64t, username string, password string)";
	rc = sqlite3_exec(db,queries[ind-1],0,0,0);

sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, 0);

	clock_t start, end;
	double total;
	queries[ind++] = "select * from user where ?";
	printf("%s\n",queries[ind-1]);
	sqlite3_stmt *stmt;
	start = clock();
	rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
 	if(rc != SQLITE_OK){
   		printf("Could not prepare statement.\n");
	}
	printf("The statement has %d wildcards\n", sqlite3_bind_parameter_count(stmt));
	rc = sqlite3_bind_text(stmt,1,"1",-1,SQLITE_STATIC);
  	if(rc != SQLITE_OK){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }

	//rc = sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, 0);
	//rc = sqlite3_exec(db, "PRAGMA journal_mode = OFF", NULL, NULL, 0);

	do {
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW) 
		//printf("uuid is %d.\tlast modifier is %s.\n",sqlite3_column_int(stmt,0),sqlite3_column_text(stmt,5));
		printf("userid is %d.\tlast username is %s.\n",sqlite3_column_int(stmt,0),sqlite3_column_text(stmt,1));
	} while(rc == SQLITE_ROW);
	
sqlite3_exec(db, "END TRANSACTION", NULL, NULL, 0);

	end = clock();
	total = (double)(end-start)/CLOCKS_PER_SEC*1000;
	printf("total time is %f.\n",total);
	printf("start time is %lu.\n",start);
	printf("end time is %lu.\n",end);

        //printf("size of string: %lu\n",sizeof(x));
   	sqlite3_close(db);
   	return 0;
}
