#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "lib_db.h"

void select_all(char* sql){
	sprintf(sql,"SELECT * FROM data");
	printf("this sql: %s\n",sql);
}

void insert_data(char* sql,int uuid,int dataid,char* dataname,char* data,int owner,long msec){
        //sprintf(sql, "INSERT INTO data VALUES ('%d', '%d', '%s', '%s', '%d', (strftime('%%s','now')));", uuid, dataid, dataname, data, owner);
        sprintf(sql, "INSERT INTO data VALUES ('%d', '%d', '%s', '%s', '%d', '%ld');", uuid, dataid, dataname, data, owner, msec);
        printf("%s\n",sql);
}

void insert_user(char* sql,int userid,char* username,char* pass){
        sprintf(sql, "INSERT INTO user VALUES ('%d', '%s', '%s');", userid, username, pass);
        printf("%s\n",sql);
}

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

long current_timestamp(long msec){
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    msec = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    printf("milliseconds: %ld\n", msec);
    return msec;
}

void random_user(char* x){
        char ch;
        for (int j = 0; j < 5; j++){
                ch = rand() % 26 + 97;
                append(x,ch);
        }
        printf("%s\n",x);
}

void random_password(char* x){
        char ch;
        for (int j = 0; j < 8; j++){
                ch = rand() % 26 + 97;
                append(x,ch);
        }
        printf("%s\n",x);
}

void random_dataname(char* x){
	char ch;
	append(x,'F');
        for (int j = 0; j < 3; j++){
                ch = rand() % 26 + 97;
                append(x,ch);
        }
        printf("%s\n",x);
}


void random_data(char* x){
        char ch;
        for (int j = 0; j < 50; j++){
                ch = rand() % 26 + 65;
                append(x,ch);
        }
        printf("%s\n",x);
}
