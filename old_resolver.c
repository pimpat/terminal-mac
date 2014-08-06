#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/time.h>
#include "chatlib.h"

int main(void){
    
   	sqlite3 *db;
   	char *zErrMsg = 0;
   	int rc;
   	int q_cnt=103000,q_size=200,ind=0;
   	char **queries = malloc(sizeof(char) * q_cnt * q_size);
   	sqlite3_stmt *stmt;
    
	// open data.db & user.db
   	rc = sqlite3_open("/Users/pimpat/zmq.db", &db);
	if(rc != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
  	}
	else fprintf(stderr, "Opened database successfully\n");
    
	// create sql
    queries[ind++] = "CREATE TABLE IF NOT EXISTS data (name string NOT NULL PRIMARY KEY, status string, ip string)";
    rc = sqlite3_exec(db,queries[ind-1],0,0,0);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    
    //----------------------------------------------------------
    
    // receive
    void *context = zmq_ctx_new ();
    void *receiver = zmq_socket (context, ZMQ_REP);
    zmq_bind (receiver, "tcp://*:5502");
 


    printf("[----Resolver----]\n");
	//char cl_ip[30];
    
    //zmq_pollitem_t items [] = {{ receiver, 0, ZMQ_POLLIN, 0 }};
    while(1){
        
        //zmq_poll (items, 1, -1);
        //if (items [0].revents & ZMQ_POLLIN) {
        
		char *string = s_recv(receiver);
		char num[2];
        strncpy(num,string,1);
		//memcpy(num,string,1);
		//printf("%s\n",num);
		if(strcmp(num,"O")==0){
			printf("Status: ");
			char *token=strtok(string," ");
			char tstatus[10];
			sprintf(tstatus,"%s",token);
			printf("%s\n",tstatus);
            
			token = strtok(NULL," ");
			//printf("%s\n",token);
			char tname[10];
			sprintf(tname,"%s",token);
			//printf("%s\n",tname);
            
			token = strtok(NULL," ");
			char ip[30];
			sprintf(ip,"%s",token);
			//printf("%s\n",ip);
            
			//database
   			char sql[100];
   			insert_data(sql,tname,tstatus,ip);
   			queries[ind++] = sql;
   			rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
   			if(rc != SQLITE_OK){
   				fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
   			}
            printf("SQL prepared OK\n");
            rc = sqlite3_step(stmt);
			if(rc != SQLITE_DONE){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            fflush (stdout);
            s_sleep (atoi (string));    //  Do the work
            //free (string);
            s_send(receiver,"received");      //###########
            
		}
		if(strcmp(num,"1")==0){
			// send ip back
			printf("%s\n",string);
			char *token=strtok(string," ");
			token = strtok(NULL," ");
			token = strtok(NULL," ");	// at this line, token is name.
   			char sql[100];
			select_user(sql,token);
            queries[ind++] = sql;
			//printf("query: %s\n",sql);
            rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
            if(rc != SQLITE_OK){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            printf("SQL prepared OK\n");
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_ROW){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
			char ip[30];
			char status[10];
			sprintf(status,"%s",sqlite3_column_text(stmt,1));
			if(strcmp(status,"ONLINE")==0){
                sprintf(ip,"%s",sqlite3_column_text(stmt,2));
                printf("%s's ip is %s.\n",token,ip);
        		fflush (stdout);
        		s_sleep (atoi (string));    //  Do the work
        		//free (string);
                char *token2=strtok(ip,"*");
                token2 = strtok(NULL,"*");	// token2 is friend's ip
                //printf("%s\n",token2);
                //------------------------------------------------------
                char targetip[30]="tcp://localhost";
                strcat(targetip,token2);
                printf("target ip: %s\n",targetip);
                s_sleep(2000);
                s_send(receiver,targetip);
                
			}
			else s_send(receiver,"EXIT");
		}
        
        free (string);
        num[0]='\0';
        //}
	}
    //zmq_close (sender);
    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}
