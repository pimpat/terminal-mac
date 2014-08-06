#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *receive_data (void *receiver) {
	while(1) {
		char *string = s_recv(receiver);
        	if(strlen(string)==1) continue;

                if(strstr(string,"OPEN")!=NULL){
                        char *token=strtok(string," ");
                        token = strtok(NULL," ");
                        token = strtok(NULL," ");
                        printf("New connection request: %s\n", token);
                }

		printf ("Message: %s", string);
        	free (string);
	}
	zmq_close(receiver);
	return NULL;
}

static void *receive_newcon (void *receiver) {
	void **x = (void **)receiver;
        while(1) {
                char *string = s_recv(x[0]);
                if(strstr(string,"OPEN")!=NULL){
			char *token=strtok(string," ");
                        token = strtok(NULL," ");
                        token = strtok(NULL," ");
                        printf("New connection request: %s\n", token);
                }
                free (string);
        }
        zmq_close(x[0]);
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

	// get new connection
	void *responder = zmq_socket (context, ZMQ_PULL);
    	zmq_connect (responder, "tcp://localhost:5503");
        //zmq_setsockopt (responder, ZMQ_SUBSCRIBE, NULL, 0);
    
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
        printf("%s\n",buffer);
        free(buffer);
    }
//---------------------------------------------
	st[0]='\0';	
	printf("Join an available chatting room(Y/N): ");
	scanf("%s",st);	
   	char friend[20]; 
	int oldchat = 0;
	int numfrd;
  	if(strcmp(st, "Y")==0){
		numfrd=1; 
		oldchat=1;
	}
	else{
		printf("Enter number of friends: ");
		scanf("%i",&numfrd);
	}
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
/*
	struct newFriend{
		void *res;
		void *send;
	};
	struct newFriend nf;
	nf.res=responder;
	nf.send=sender;
*/
	void *x[2];
	x[0]=responder;
	x[1]=sender;
    
	//pthread_t rec_data;
	//pthread_create(&rec_data, NULL,receive_data,receiver);
	pthread_t rec_newcon;
	pthread_create(&rec_newcon, NULL,receive_newcon,(void *)x);
    
	printf("Loading ...\n");
	sleep(5);
    
	while(rc==0){
		//printf("oldchat:%i\n",oldchat);
		if(oldchat==1){
            		st[0]='\0';
            		sprintf(st,"3 ");
			strcat(st, friendip[0]);
			strcat(st, " ");
			strcat(st, argv[1]);
            		s_send(requester, st);
                        char *buffer = s_recv(requester);     //###########
                        printf("%s\n",buffer);
                        free(buffer);
			oldchat=0;
		}

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
            		printf("%s\n",buffer);
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
