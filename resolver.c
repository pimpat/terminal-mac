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
    //!!!int q_cnt=100,q_size=200,ind=0;
    //!!!char **queries = malloc(sizeof(char) * q_cnt * q_size);
    char sql[200];
   	sqlite3_stmt *stmt;
    
	// open data.db & user.db
   	rc = sqlite3_open("/Users/pimpat/zmq.db", &db);
	if(rc != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
  	}
	else fprintf(stderr, "Opened database successfully\n");
    
	// create sql
    //!!!queries[ind++] = "CREATE TABLE IF NOT EXISTS data (name string NOT NULL PRIMARY KEY, status string, ip string)";
    sprintf(sql,"%s","CREATE TABLE IF NOT EXISTS data (name string NOT NULL PRIMARY KEY, status string, ip string)");
    //!!!rc = sqlite3_exec(db,queries[ind-1],0,0,0);
    rc = sqlite3_exec(db,sql,0,0,0);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    
    //----------------------------------------------------------
    
    // receive
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    zmq_bind (responder, "tcp://*:5502");
    
    void *requester = zmq_socket (context, ZMQ_PUSH);
    zmq_bind (requester, "tcp://*:5503");
    
    //void *requester = zmq_socket (context, ZMQ_PUB);
    //zmq_bind (requester, "tcp://*:5553");
    
    printf("[----Resolver----]\n");
    
    //zmq_pollitem_t items [] = {{ receiver, 0, ZMQ_POLLIN, 0 }};
    while(1){
        
        //zmq_poll (items, 1, -1);
        //if (items [0].revents & ZMQ_POLLIN) {
        
		char *string = s_recv(responder);
		char num[2];
        strncpy(num,string,1);
		//memcpy(num,string,1);
		//printf("%s\n",num);
		if(strcmp(num,"0")==0){     // send status to db
			printf("Status: ");
			char *token=strtok(string," ");
            token = strtok(NULL," ");
			char tstatus[10];
			sprintf(tstatus,"%s",token);
			printf("%s\n",tstatus);
            
			token = strtok(NULL," ");
			//printf("%s\n",token);
			char tname[20];
			sprintf(tname,"%s",token);
			//printf("%s\n",tname);
            
			token = strtok(NULL," ");
			char ip[30];
			sprintf(ip,"%s",token);
			//printf("%s\n",ip);
            
			//database
            //!!!char sql[200];
            sql[0]='\0';
   			insert_data(sql,tname,tstatus,ip);
            //!!!queries[ind++] = sql;
            //!!!rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
            rc = sqlite3_prepare_v2(db,sql,-1,&stmt,0);
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
            //fflush (stdout);
            //s_sleep (atoi (string));    //  Do the work
            //free (string);
            s_send(responder,"received");      //###########
            
		}
		if(strcmp(num,"1")==0){     // get friend's ip
			printf("%s\n",string);
			char *token=strtok(string," ");
			token = strtok(NULL," ");	// at this line, token is name.
            //!!!char sql[100];
            sql[0]='\0';
			select_user(sql,token);
            //!!!queries[ind++] = sql;
			//printf("query: %s\n",sql);
            //!!!rc = sqlite3_prepare_v2(db,queries[ind-1],-1,&stmt,0);
            rc = sqlite3_prepare_v2(db,sql,-1,&stmt,0);
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
                char *token2=strtok(ip,"*");
                token2 = strtok(NULL,"*");	// token2 is friend's ip
                //printf("%s\n",token2);
                char targetip[30]="tcp://localhost";
                strcat(targetip,token2);
                printf("target ip: %s\n",targetip);
                //s_sleep(2000);
                s_send(responder,targetip);
			}
			else s_send(responder,"EXIT");
		}
        if(strcmp(num,"2")==0){     // get list of friends
			char *token=strtok(string," ");
			token = strtok(NULL," ");
            //!!!char sql[30] = "SELECT name, status FROM data;";
            sql[0] = '\0';
            sprintf(sql,"%s","SELECT name, status FROM data ORDER BY name;");
            rc = sqlite3_prepare_v2(db,sql,-1,&stmt,0);
            printf("%s\n",sql);
            if(rc != SQLITE_OK){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            printf("SQL prepared OK\n");
            char list[2000];
            char tlist[30];
            list[0]='\0';
            tlist[0]='\0';
            do {
                rc = sqlite3_step(stmt);
                if(rc == SQLITE_ROW){
                    sprintf(tlist,"\t%-10s|%s\n",sqlite3_column_text(stmt,0),sqlite3_column_text(stmt,1));
                    strcat(list,tlist);
                }
            } while(rc == SQLITE_ROW);
            printf("length: %ld\n",strlen(list));
            s_send(responder,list);
        }
        if(strcmp(num,"3")==0){
            printf("%s\n",string);
            s_send(responder,"You are joining ...");         // code here !!!
            //char *token=strtok(string," ");
			//token = strtok(NULL," ");
            //token = strtok(NULL," ");
            char string2[50];
            //sprintf(string2,"OPEN %s",token);      // OPEN 3 IP's friend IP's join
            sprintf(string2,"OPEN %s",string);
            printf("%s\n",string2);
            s_send(requester, string2);
        }
        free (string);
        num[0]='\0';
    
	}
    zmq_close (responder);
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
