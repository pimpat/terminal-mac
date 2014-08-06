#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/time.h>
#include "chatlib.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(void){

   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   int q_cnt=103000,q_size=200,ind=0;
   char **queries = malloc(sizeof(char) * q_cnt * q_size);
   sqlite3_stmt *stmt;

// open data.db & user.db
   rc = sqlite3_open("zmq.db", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }

// create sql
        queries[ind++] = "CREATE TABLE IF NOT EXISTS status (name string NOT NULL PRIMARY KEY, status string)";
        rc = sqlite3_exec(db,queries[ind-1],callback,0,0);

	// send msg 
    void *context = zmq_ctx_new ();
    void *sender = zmq_socket (context, ZMQ_PUSH);
    zmq_connect (sender, "tcp://localhost:5501");

    	// receive msg 
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_bind (receiver, "tcp://*:5502");

        printf("[----Resolver----]\n");
        while(1){
		char *string = s_recv(receiver);
        	printf("Status: ");
        	if(strstr(string,"online")!=NULL)printf ("%s\n", string);     //  Show progress
        	if(strstr(string,"offline")!=NULL)printf ("%s\n", string);     //  Show progress

char *token=strtok(string," ");
char tstatus[10];
sprintf(tstatus,"%s",token);
printf("%s\n",tstatus);

token = strtok(NULL," ");
//printf("%s\n",token);
char tname[10];
sprintf(tname,"%s",token);
printf("%s\n",tname);

//database
   char sql[100];
   insert_data(sql,tname,tstatus);
   queries[ind++] = sql;
   rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
   if(rc != SQLITE_OK){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
        printf("SQL prepared OK\n");
        rc = sqlite3_step(stmt);

        	fflush (stdout);
        	s_sleep (atoi (string));    //  Do the work
        	free (string);
	}
	

    zmq_close (sender);
    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}
