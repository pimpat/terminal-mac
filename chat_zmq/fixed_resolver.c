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
    sprintf(sql,"%s","CREATE TABLE IF NOT EXISTS data (name string NOT NULL PRIMARY KEY, status string, ip string)");
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

    void *requester = zmq_socket (context, ZMQ_PUB);
    zmq_bind (requester, "tcp://*:5503");
    
    printf("[----Resolver----]\n");
    
    //zmq_pollitem_t items [] = {{ receiver, 0, ZMQ_POLLIN, 0 }};
    while(1){
        
        //zmq_poll (items, 1, -1);
        //if (items [0].revents & ZMQ_POLLIN) {
        
		char *string = s_recv(responder);
		char num[2];
        strncpy(num,string,1);
		//printf("%s\n",num);
        
		if(strcmp(num,"0")==0){     // send status to db
			char *token=strtok(string," ");
            token = strtok(NULL," ");
			char tstatus[10];
			sprintf(tstatus,"%s",token);
            token = strtok(NULL," ");
			char tname[20];
			sprintf(tname,"%s",token);
			token = strtok(NULL," ");
			char ip[30];
			sprintf(ip,"%s",token);
            
			//database
            sql[0]='\0';
   			insert_data(sql,tname,tstatus,ip);
            printf("%s is %s.\n",tname,tstatus);
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
            s_send(responder,"received");
		}
        
		if(strcmp(num,"1")==0){     // get friend's ip from db
			//printf("%s\n",string);
			char *token=strtok(string," ");
			token = strtok(NULL," ");       // at this line, token is name.
            sql[0]='\0';
			select_user(sql,token);
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
                printf("%s's IP: %s\n",token,ip);
                char targetip[30]="tcp://";
                strcat(targetip,ip);
                strcat(targetip,":5501");
                //printf("Target IP: %s\n",targetip);
                s_send(responder,targetip);
			}
			else s_send(responder,"EXIT");
		}
        
        if(strcmp(num,"2")==0){     // get list of friends from db
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
            //printf("length: %ld\n",strlen(list));
            s_send(responder,list);
        }

        if(strcmp(num,"3")==0){     // notify everyone about new friends
            printf("%s\n",string);
            s_send(responder,"You are joining ...");
            char string2[50];
            sprintf(string2,"OPEN %s", string);      // "OPEN 3 IP's friend IP's join"
            s_send(requester, string2);
        }
        
        if(strcmp(num,"4")==0){     // send other friends' ip in the group to newip
            printf("%s\n",string);      // "4  sender's ip  new's ip"
            s_send(responder,"received");
            void *requester2 = zmq_socket (context, ZMQ_REQ);
            char oldstring[70];
            sprintf(oldstring,"%s",string);
            char *token=strtok(string," ");
			token = strtok(NULL," ");
            token = strtok(NULL," ");
            char newip[30];
            sprintf(newip,"tcp://%s:5504",token);
            //printf("token: %s\n",newip);
            zmq_connect (requester2, newip);
            s_send(requester2,oldstring);
            char *buffer = s_recv(requester2);
            printf("%s\n",buffer);
            free(buffer);
        }

        free (string);
        num[0]='\0';
    
	}
    zmq_close (responder);
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
