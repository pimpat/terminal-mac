#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "insert.h"

void insertData(char* sql,int id,char* name,int age,char* addr,double salary){
	//char s[20] = "Yuri";
	sprintf(sql, "INSERT INTO COMPANY VALUES ('%d', '%s', '%d', '%s', '%f');", id, name, age, addr, salary);
	printf("%s\n",sql);
}
