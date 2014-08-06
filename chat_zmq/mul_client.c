#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *receive_data (void *receiver) {
	while(1) {
		char *string = s_recv(receiver);
        	if(strlen(string)==1) continue;
		printf ("Message: %s", string);
        	//printf("%ld\n",strlen(string));
        	//fflush (stdout);
        	//s_sleep (atoi (string));    //  Do the work
        	free (string);
	}
	zmq_close(receiver);
	return NULL;
}

int main(int argc, char *argv[]){
    
	// send msg
	//char ip[30] = "tcp://*:5556";
    	void *context = zmq_ctx_new ();
    	void *sender = zmq_socket (context, ZMQ_PUB);
    	//zmq_bind (sender, ip);
    	zmq_bind (sender, argv[1]);
    
	// send ip & status
    	void *requester = zmq_socket (context, ZMQ_REQ);
    	zmq_connect (requester, "tcp://localhost:5502");
    
	printf("[----Chat Room----]\n");
	printf("Enter your name: ");
    	char name[20];
    	//printf("Addr name: %d\n", name);
    	scanf("%s",name);
    	printf("Hello %s ...\n",name);
    	char st[50];
	printf("Your status(ONLINE/OFFLINE): ");
	scanf("%s",st);
    
	if(strcmp(st,"ONLINE")==0){
        	st[0]='\0';
        	sprintf(st,"0 ONLINE ");    //1
        	//strcat(st, " ");
        	strcat(st, name);
        	strcat(st, " ");
		strcat(st, argv[1]);
		//printf("%s\n",st);
		s_send(requester, st);
        	char *buffer = s_recv(requester);     //###########
        	printf("%s\n",buffer);
        	free(buffer);
	}
	else{
		st[0]='\0';
        	sprintf(st,"0 OFFLINE ");   //2
        	//strcat(st, "OFFLINE ");
        	strcat(st, name);
        	strcat(st, " ");
        	strcat(st, argv[1]);
		//printf("%s\n",st);
		s_send(requester, st);
        	char *buffer = s_recv(requester);     //###########
        	printf("%s\n",buffer);
        	free(buffer);
        	printf("Exit now ...\n");
		sleep(3);
		exit(0);
	}
    
    	st[0]='\0';
    	printf("Get list of friends(Y/N): ");
    	scanf("%s",st);
    	if(strcmp(st,"Y")==0){
        s_send(requester, "2");
        char *buffer = s_recv(requester);     //###########
        printf("%s",buffer);
        free(buffer);
    }
   	char friend[20]; 
	int numfrd;
	printf("Enter number of friends: ");
	scanf("%i",&numfrd);
	char friendip[numfrd][30];
	for(int i=0;i<numfrd;i++){
		printf("Enter a friend's name: ");
		scanf("%s",friend);
    		st[0]='\0';
    		strcat(st, "1 ");
    		strcat(st, friend);
		//printf("Request friend: %s\n",st);
		s_send(requester, st);
    		char *frd_ip = s_recv(requester);     //###########
    		printf("%s\n",frd_ip);
    
		if(strcmp(frd_ip,"EXIT")==0) {
			printf("%s is OFFLINE.\n",friend);
			exit(0);
		}

		strcpy(friendip[i], frd_ip);
    		//printf ("Friend's ip: %s\n", frd_ip);
		printf ("Friend's ip: %s\n",friendip[i]);
	}
    
    	// receive msg
	int rc;
	void *receiver[numfrd];
	pthread_t rec_data;

	for(int i=0;i<numfrd;i++){
    		//void *receiver = zmq_socket (context, ZMQ_SUB);
    		receiver[i] = zmq_socket (context, ZMQ_SUB);
    		rc = zmq_connect (receiver[i], (char *)friendip[i]);
        	zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
		//pthread_t rec_data[i];
		pthread_create(&rec_data, NULL,receive_data,receiver[i]);
	}
    
	//pthread_t rec_data;
	//pthread_create(&rec_data, NULL,receive_data,receiver);
    
	printf("Loading ...\n");
	sleep(5);
    
	while(rc==0){
        	char string[256];
        	printf("%s: ",name);
		//scanf("%s",string);
        	fgets(string,sizeof string, stdin);
        	//if(strlen(string)==1) continue;
        	//printf("%ld\n",strlen(string));
		//fclose(string);
		//if(strstr(string,"EXIT")!=NULL){
		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';      // fix useless \n
		//if(strcmp(string,"EXIT")==0){     // use for scanf
		if(strcmp(temp,"EXIT")==0){
            		st[0]='\0';
            		sprintf(st,"0 OFFLINE ");   //3
            		//strcat(st, "OFFLINE ");
            		strcat(st, name);
            		strcat(st, " ");
            		strcat(st, argv[1]);
            		//printf("%s\n",st);
            		//sprintf(string,"%s is OFFLINE.\n",name);    //4
            		//s_send (sender, string);
            		s_send(requester, st);
            		char *buffer = s_recv(requester);     //###########
            		printf("%s\n",buffer);
            		free(buffer);
            		printf("Exit now ...\n");
            		sleep(3);
            		exit(0);
		}
        	if(strcmp(temp,"LIST")==0){
            		st[0]='\0';
            		s_send(requester, "2");
            		printf("\t[----List of friends----]\n");
            		char *buffer = s_recv(requester);     //###########
            		printf("%s",buffer);
            		free(buffer);
        	}
		//printf("%d\n",strlen(string));
        	//printf("your msg: %s",string);
        	if(strcmp(temp,"LIST")!=0) s_send (sender, string);
    	}
    	zmq_close (sender);
    	zmq_close (requester);
    	//zmq_close (receiver);
    	zmq_ctx_destroy (context);
    	return 0;
}
