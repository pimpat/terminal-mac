/*
 * reader.c
 * Simple program to read file into a config struct
 * gcc -Wall reader.c -o reader
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define FILENAME "config.conf"
#define MAXBUF 1024 
#define DELIM "="
 
struct config
{
   char imgserver[MAXBUF];
   char ccserver[MAXBUF];
   char port[MAXBUF];
   char imagename[MAXBUF];
   char getcmd[MAXBUF];
};
 
struct config get_config(char *filename) 
{
	struct config configstruct;
	FILE *file = fopen (filename, "r");
 
	if (file != NULL)
	{ 
		char line[MAXBUF];
		int i = 0;
 
		while(fgets(line, sizeof(line), file) != NULL)
		{
			char *cfline;
			cfline = strstr((char *)line,DELIM);
			cfline = cfline + strlen(DELIM);
 
			if (i == 0){
				memcpy(configstruct.imgserver,cfline,strlen(cfline));
				//printf("%s",configstruct.imgserver);
			} else if (i == 1){
				memcpy(configstruct.ccserver,cfline,strlen(cfline));
				//printf("%s",configstruct.ccserver);
			} else if (i == 2){
				memcpy(configstruct.port,cfline,strlen(cfline));
				//printf("%s",configstruct.port);
			} else if (i == 3){
				memcpy(configstruct.imagename,cfline,strlen(cfline));
				printf("%s",configstruct.imagename);
			} else if (i == 4){
				memcpy(configstruct.getcmd,cfline,strlen(cfline));
				//printf("%s",configstruct.getcmd);
			}
 
			i++;
		} // End while
	} // End if file
 
	fclose(file);	
 
	return configstruct;
 
}
 
int main(int argc, char **argv)
{
	struct config configstruct;
 
	configstruct = get_config(FILENAME);
 
	/* Struct members */
	printf("%s",configstruct.imgserver);
	printf("%s",configstruct.ccserver);
	printf("%s",configstruct.port);
	printf("%s",configstruct.imagename);
	printf("%s",configstruct.getcmd);
 
	/* Cast port as int */
	int x;
	x = atoi(configstruct.port);
	printf("%d\n",x);
 
	return 0;
}
