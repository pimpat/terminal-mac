#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

void select_user(char* sql,char* ip){
        sprintf(sql,"SELECT * FROM data where name='%s';",ip);
        printf("%s\n",sql);
}

void insert_data(char* sql,char* name,char* status,char* ip){
	sprintf(sql, "INSERT OR REPLACE INTO data VALUES ('%s', '%s', '%s');", name, status, ip);
	printf("%s\n",sql);
} 
