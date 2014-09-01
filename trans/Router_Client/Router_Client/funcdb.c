//
//  funcdb.c
//  TransportLayer
//
//  Created by Pimpat on 8/25/2557 BE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "transport.h"

sqlite3 *DB;

void createDatabase(char* path){
    int rc;
    char sql[200];
    rc = sqlite3_open(path, &DB);
	if(rc != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(DB));
        exit(0);
  	}
    else printf("Opened database successfully\n");
    sprintf(sql,"%s","CREATE TABLE IF NOT EXISTS data (id string NOT NULL PRIMARY KEY, name string, status string, ip string, port int)");
    rc = sqlite3_exec(DB,sql,0,0,0);
    if(rc != SQLITE_OK)
        printf("SQL error\n");
    else
        printf("SQL sent to database successfully\n");
}

int registerUser(char* id, char* name, char* status, char* ip, int port){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
	sprintf(sql, "INSERT OR REPLACE INTO data VALUES ('%s', '%s', '%s', '%s', '%d');", id, name, status, ip, port);
	printf("SQL: %s\n",sql);
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK){
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
        return 1;
    }
    else{
        printf("SQL prepared OK\n");
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE){
            printf("SQL error\n");
            return 1;
        }
        else{
            printf("SQL sent to database successfully\n");
            return 0;
        }
    }
}

int setStatusUser(char* id, char* name, char* status, char* ip, int port){
    int i;
    if (strcmp(status,"ONLINE")==0 || strcmp(status,"OFFLINE")==0){
        i = registerUser(id,name,status,ip,port);
        return i;
    }
    else return 1;
}

void offlineStatus(char* id){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    sprintf(sql, "UPDATE data SET status = '%s' WHERE id = '%s';", "OFFLINE",id);
    printf("SQL: %s\n",sql);
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK)
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
    else
        printf("SQL prepared OK\n");
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
        printf("SQL error\n");
    else
        printf("SQL sent to database successfully\n");
}

void onlineStatus(char* id){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    sprintf(sql, "UPDATE data SET status = '%s' WHERE id = '%s';", "ONLINE",id);
    printf("SQL: %s\n",sql);
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK)
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
    else
        printf("SQL prepared OK\n");
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
        printf("SQL error\n");
    else
        printf("SQL sent to database successfully\n");
}

int getList(Client** list){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    int i;
    sprintf(sql, "SELECT * FROM data;");
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK){
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
        return 1;
    }
    i=0;
    do{
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW){
            sprintf(list[i]->id,"%s",sqlite3_column_text(stmt,0));
            sprintf(list[i]->name,"%s",sqlite3_column_text(stmt,1));
            char st[10];
            sprintf(st,"%s",sqlite3_column_text(stmt,2));
            if(strcmp(st,"ONLINE")==0)
                list[i]->status=0;
            else
                list[i]->status=1;
            sprintf(list[i]->ip,"%s",sqlite3_column_text(stmt,3));
            list[i]->port=sqlite3_column_int(stmt,4);
            i++;
        }
    }while(rc == SQLITE_ROW);
    return 0;
}

int getSize(char* type){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    int size;
    if(strcmp(type,"all")==0)
        sprintf(sql, "SELECT Count(*) FROM data;");
    else if(strcmp(type,"online")==0)
        sprintf(sql, "SELECT Count(*) FROM data WHERE status = 'ONLINE';");
    else if(strcmp(type,"offline")==0)
        sprintf(sql, "SELECT Count(*) FROM data WHERE status = 'OFFLINE';");
    else
        sprintf(sql, "SELECT Count(*) FROM data WHERE name = '%s';",type);
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK){
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
        return -1;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        size = sqlite3_column_int(stmt,0);
        return size;
    }
    else return -1;
}

int getInfoByName(char* name, Client** result){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    int i;
    sprintf(sql, "SELECT * FROM data WHERE name = '%s';",name);
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK){
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
        return 1;
    }
    i=0;
    do{
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW){
            sprintf(result[i]->id,"%s",sqlite3_column_text(stmt,0));
            sprintf(result[i]->name,"%s",sqlite3_column_text(stmt,1));
            char st[10];
            sprintf(st,"%s",sqlite3_column_text(stmt,2));
            if(strcmp(st,"ONLINE")==0)
                result[i]->status=0;
            else
                result[i]->status=1;
            sprintf(result[i]->ip,"%s",sqlite3_column_text(stmt,3));
            result[i]->port=sqlite3_column_int(stmt,4);
            i++;
        }
    }while(rc == SQLITE_ROW);
    return 0;
}

Client* getInfoByID(char* id){
    sqlite3_stmt *stmt;
    int rc;
    char sql[200];
    sprintf(sql, "SELECT * FROM data where id='%s';",id);
    Client* result = (Client*)malloc(sizeof(Client));
    rc = sqlite3_prepare_v2(DB,sql,-1,&stmt,0);
    if(rc != SQLITE_OK){
        printf("SQL error: %s\n",sqlite3_errmsg(DB));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        sprintf(result->id,"%s",sqlite3_column_text(stmt,0));
        sprintf(result->name,"%s",sqlite3_column_text(stmt,1));
        char st[10];
        sprintf(st,"%s",sqlite3_column_text(stmt,2));
        if(strcmp(st,"ONLINE")==0)
            result->status=0;
        else
            result->status=1;
        sprintf(result->ip,"%s",sqlite3_column_text(stmt,3));
        result->port=sqlite3_column_int(stmt,4);
        return result;
    }
    else {
        sprintf(result->id,"%s","-1");
        return result;
    }
}
