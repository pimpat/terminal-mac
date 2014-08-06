#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
	void *recv;
	void **rec_f;
	int num;
	pthread_t rec_d;
}new_ip;

static void *receive_data (void *receiver) {
        while(1) {
                char *string = s_recv(receiver);
                if(strlen(string)==1) continue;
                printf ("Message: %s", string);
                free (string);
        }
        zmq_close(receiver);
        return NULL;
}

static void *receive_ip (void *receiver){
        new_ip *x = receiver;
        while(1){
                char *string = s_recv(x->recv);
		printf("%s\n",string);
                char *token=strtok(string," ");
                if(strcmp(token,"NEWIP")==0){
                        token = strtok(NULL," ");
                        //  "tcp://192.168.179.111:5501"
                        printf("NEW IP: %s\n",token);
                        zmq_connect (x->rec_f[x->num], token);
                        zmq_setsockopt (x->rec_f[x->num], ZMQ_SUBSCRIBE, NULL, 0);
                        pthread_create(&x->rec_d, NULL,receive_data,x->rec_f[x->num]);
                        x->num++;
                }
        }

}

int main(int argc, char *argv[]){

	// send ip & status
    	void *context = zmq_ctx_new ();
        void *requester = zmq_socket (context, ZMQ_REQ);
        zmq_connect (requester, "tcp://192.168.179.94:5502");
	
	// send msg to fwder
	char fwd_ip[30];
	sprintf(fwd_ip,"tcp://");
	strcat(fwd_ip,argv[1]);
	strcat(fwd_ip,":5505");
    	void *sender = zmq_socket (context, ZMQ_REQ);
    	zmq_connect(sender, fwd_ip);

	printf("[----Chat Room----]\n");
        printf("Enter your name: ");
        char name[20];
        scanf("%s",name);
        printf("Hello %s ...\n",name);

	char st[50];
        printf("Your status(ONLINE/OFFLINE): ");
        scanf("%s",st);

        if(strcmp(st,"ONLINE")==0){
                st[0]='\0';
                sprintf(st,"0 ONLINE ");
                strcat(st, name);
                strcat(st, " ");
                strcat(st, argv[1]);
                s_send(requester, st);
                char *buffer = s_recv(requester);
                free(buffer);
        }
        else{
                st[0]='\0';
                sprintf(st,"0 OFFLINE ");
                strcat(st, name);
                strcat(st, " ");
                strcat(st, argv[1]);
                s_send(requester, st);
                char *buffer = s_recv(requester);
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
                char *buffer = s_recv(requester);
                printf("%s\n",buffer);
                free(buffer);
        }

	st[0]='\0';
        printf("Join an available chatting room(Y/N): ");
        scanf("%s",st);
        int numfrd;
        if(strcmp(st, "Y")==0){
		numfrd = 1;
		s_send(sender,"Y");
  		char *buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	free(buffer);
		
        }
	else{
		s_send(sender,"N");
  		char *buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	printf("Enter number of friends: ");
        	scanf("%i",&numfrd);
		char cnum[2];
		sprintf(cnum,"%d",numfrd);
		s_send(sender,cnum);
  		buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	free(buffer);
	}
	
	char friend[20];
	char friendip[numfrd][30];
	int i;
        for(i=0;i<numfrd;i++){
                printf("Enter a friend's name: ");
                scanf("%s",friend);
                st[0]='\0';
                strcat(st, "1 ");
                strcat(st, friend);
                s_send(requester, st);
                char *frd_ip = s_recv(requester);
                if(strcmp(frd_ip,"EXIT")==0) {
                        printf("%s is OFFLINE.\n",friend);
                        exit(0);
                }
                strcpy(friendip[i], frd_ip);
                printf ("Friend's ip: %s\n",friendip[i]);
		s_send(sender,friendip[i]);
		char *buffer = s_recv(sender);
		free(buffer);
        }

	// receive msg
        int rc;
        void *receiver[10];     // fix 10 friends for one group
        pthread_t rec_data;

        for(i=0;i<10;i++){
                receiver[i] = zmq_socket (context, ZMQ_SUB);
                if(i<numfrd){
                        rc = zmq_connect (receiver[i], (char *)friendip[i]);
                        zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
                        pthread_create(&rec_data, NULL,receive_data,receiver[i]);
                }
        }

	char temp[30];
	void *recv_noti = zmq_socket(context,ZMQ_SUB);
	sprintf(temp,"tcp://%s:5506",argv[1]);
	zmq_connect(recv_noti,temp);
        zmq_setsockopt (recv_noti, ZMQ_SUBSCRIBE, NULL, 0);

	new_ip newip;
	newip.recv=recv_noti;
	newip.rec_f=receiver;
	newip.num=numfrd;
	newip.rec_d=rec_data;
	pthread_t rec_ip;
        pthread_create(&rec_ip, NULL,receive_ip,&newip);

	printf("Loading ...\n");
	sleep(5);

	while(rc==0){

        	char string[256];
        	printf("%s: ",name);
        	fgets(string,sizeof string, stdin);

		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';      // fix useless \n
		if(strcmp(temp,"EXIT")==0){
			st[0]='\0';
            		sprintf(st,"0 OFFLINE ");   //3
            		strcat(st, name);
            		strcat(st, " ");
     		       	strcat(st, argv[1]);
            		s_send(requester, st);
            		char *buffer = s_recv(requester);
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
		if(strcmp(temp,"LIST")!=0){ 

			s_send (sender, string);
			char *buffer = s_recv(sender);
                        //printf("%s\n",buffer);
                        free(buffer);
		}			
    	}
    	zmq_close (sender);
    	zmq_close (requester);
    	zmq_ctx_destroy (context);
    	return 0;
}
