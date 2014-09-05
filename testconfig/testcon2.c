#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "config2.conf"
#define MAXSIZE 256
#define DELIM "="

char name[MAXSIZE];
char nickname[MAXSIZE];
char age[MAXSIZE];
char id[MAXSIZE];

int main(){
	FILE *file = fopen(FILENAME, "r");
	if(file!=NULL){
		char line[MAXSIZE];
		int i=0;
		while(fgets(line,sizeof(line),file) != NULL){
			char *cfline;
			cfline = strstr((char *)line,DELIM);
			cfline = cfline + strlen(DELIM);
			if(i==0)	memcpy(name,cfline,strlen(cfline)-1);
			else if(i==1)	memcpy(nickname,cfline,strlen(cfline)-1);
			else if(i==2)	memcpy(age,cfline,strlen(cfline)-1);
			else if(i==3)	memcpy(id,cfline,strlen(cfline)-1);
		i++;
		}
	}
/*
	printf("char: %c",name[6]);
	printf("len: %d\n",strlen(name));
	name[strlen(name)-1] ='\0';
	printf("len(fix): %d\n",strlen(name));
*/
	//printf("len: %d\n",strlen(name));
	printf("name: %s\n",name);
	printf("nickname: %s\n",nickname);
	printf("age: %s\n",age);
	printf("id: %s\n",id);

	/* cast string to int */
	int c_id, c_age;
	c_id=atoi(id);
	c_age=atoi(age);
	printf("age(int): %d\n",c_age);
	printf("id(int): %d\n",c_id);

	return 0;
}
