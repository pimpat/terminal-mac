#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *receive_data (void *receiver) {
	while(1) {
		char *string = s_recv(receiver);
		printf ("Message: %s\n", string);     //  Show progress
                fflush (stdout);
                s_sleep (atoi (string));    //  Do the work
                free (string);
	}
	zmq_close(receiver);
	return NULL;
}

int main(void){

	// send msg 
	char ip[30]="tcp://*:5555";
    	void *context = zmq_ctx_new ();
    	void *sender = zmq_socket (context, ZMQ_PUSH);
    	zmq_bind (sender, ip);

	// send ip & status
    	void *sender2 = zmq_socket (context, ZMQ_REQ);
    	zmq_connect (sender2, "tcp://localhost:5502");

	// receive friend's ip
    	void *receiver2 = zmq_socket (context, ZMQ_REP);
    	zmq_connect (receiver2, "tcp://localhost:5501");
	
	printf("[----Chat Room----]\n");
	printf("Enter your name: ");
        char *name;
        scanf("%s",name);
        printf("Hello %s ...\n",name);
        char st[50];
	printf("Your status(ONLINE/OFFLINE): ");
	scanf("%s",&st[0]);

	if(strcmp(st,"ONLINE")==0){
                strcat(st, " ");
                strcat(st, name);
                strcat(st, " ");
		strcat(st, ip);
		//printf("%s\n",st);
		s_send(sender2, st); 
	}	
	else{
		st[0]='\0';
                strcat(st, "OFFLINE ");
                strcat(st, name);
                strcat(st, " ");
                strcat(st, ip);
		//printf("%s\n",st);
		s_send(sender2, st);
		sleep(3);
		exit(0);
	}

    char buffer[10];
    s_recv(buffer);      //###########
    printf("%s\n",buffer);

	char friend[20];
	printf("Enter a friend's name: "); 	
	scanf("%s",&friend[0]);
        st[0]='\0';
        strcat(st, "1 OPEN ");
        strcat(st, friend);			
	printf("Request friend: %s\n",st);
	s_send(sender2, st);
		
        char *frd_ip = s_recv(receiver2);
	if(strcmp(frd_ip,"EXIT")==0) {
		printf("%s is OFFLINE.\n",friend);
		exit(0);
	}
        printf ("friend's ip: %s\n", frd_ip);
        fflush (stdout);
        s_sleep (atoi (frd_ip));
        free (frd_ip);
	zmq_close (receiver2);
    
    	// receive msg
    	void *receiver = zmq_socket (context, ZMQ_PULL);
    	int rc = zmq_connect (receiver, frd_ip);

	pthread_t rec_data;
	pthread_create(&rec_data, NULL,receive_data,receiver);

        printf("Loading ...\n");
        sleep(5);

	while(rc==0){
        	char string[256];
        	printf("%s: ",name);
                //scanf("%s",string);
        	fgets(string,sizeof string, stdin);
		fclose(string);
		//if(strstr(string,"EXIT")!=NULL){
		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';
                //if(strcmp(string,"EXIT")==0){
		if(strcmp(temp,"EXIT")==0){
            		st[0]='\0';
            		strcat(st, "OFFLINE ");
            		strcat(st, name);
            		strcat(st, " ");
            		strcat(st, ip);
            		//printf("%s\n",st);
            		s_send(sender2, st);
            		printf("Exit now ...\n");
            		sleep(3);
            		exit(0);
		}
		//printf("%d\n",strlen(string));		
		//scanf("%s",&string);
        	//printf("your msg: %s",string);
        	s_send (sender, string);
    	}
    	zmq_close (sender);
    	zmq_close (sender2);
    	//zmq_close (receiver);
    	//zmq_close (receiver2);
    	zmq_ctx_destroy (context);
    	return 0;
}
