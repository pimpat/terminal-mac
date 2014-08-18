#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
	void *recv_noti;
	void **recv;
	void *context;
	int num;
	int i;
	pthread_t rec_d;
        char friendip[30][30];
        char friendid[30][10];
}new_ip;

static void *receive_data (void *receiver) {
        while(1) {
                char *string = s_recv(receiver);
                if(strlen(string)==1) continue;
                printf ("%s", string);
                free (string);
        }
        zmq_close(receiver);
        return NULL;
}

static void *receive_ip (void *receiver){
        new_ip *x = receiver;
        while(1){
                char *string = s_recv(x->recv_noti);
		printf("%s\n",string);
                char *token=strtok(string," ");
                if(strcmp(token,"NEWIP")==0){
                        token = strtok(NULL," ");
                        //  "tcp://192.168.179.111:5501"
                        //printf("NEW IP: %s\n",token);

			x->recv[x->i] = zmq_socket (x->context, ZMQ_SUB);
                        zmq_connect (x->recv[x->i], token);
                        zmq_setsockopt (x->recv[x->i], ZMQ_SUBSCRIBE, NULL, 0);
                        pthread_create(&x->rec_d, NULL,receive_data,x->recv[x->i]);
			x->i++;
			printf("value of i: %d\n",x->i);

			sprintf(x->friendip[x->num],"%s",token);
			printf("friend ip: %s\n",token);

			token = strtok(NULL," ");
                        sprintf(x->friendid[x->num],"%s",token);
			printf("friend id: %s\n",token);
                        x->num++;
                }
        }
	zmq_close(x->recv_noti);
	return NULL;
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
	printf("Enter your id: ");
        char id[10];
        scanf("%s",id);
	char st[70];
        printf("Your status(ONLINE/OFFLINE): ");
        scanf("%s",st);

        if(strcmp(st,"ONLINE")==0){
                st[0]='\0';
		sprintf(st,"0 ONLINE %s %s %s", id, name, argv[1]);
                s_send(requester, st);
                char *buffer = s_recv(requester);
                free(buffer);
        }
        else{
                st[0]='\0';
		sprintf(st,"0 OFFLINE %s %s %s", id, name, argv[1]);
                s_send(requester, st);
                char *buffer = s_recv(requester);
                free(buffer);
                printf("Exit now ...\n");
                sleep(3);
                exit(0);
        }
	
	s_send(sender,name);
        char *buffer = s_recv(sender);
        printf("%s\n",buffer);
        free(buffer);

        s_send(sender,id);
        buffer = s_recv(sender);
        printf("%s\n",buffer);
        free(buffer);

	st[0]='\0';
        printf("Get list of friends(Y/N): ");
        scanf("%s",st);
        if(strcmp(st,"Y")==0){
                s_send(requester, "2");
                buffer = s_recv(requester);
                printf("%s",buffer);
                free(buffer);
        }

	st[0]='\0';
        printf("Join an available chatting room(Y/N): ");
        scanf("%s",st);
	char friend[20];
        int numfrd;
        if(strcmp(st, "Y")==0){
		numfrd = 1;
		s_send(sender,"Y");
  		buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	free(buffer);
		
        }
	else{
		s_send(sender,"N");
  		buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	printf("Enter number of friends: ");
        	scanf("%i",&numfrd);
		char cnum[2];
		sprintf(cnum,"%d",numfrd);
		free(buffer);
		s_send(sender,cnum);
  		buffer = s_recv(sender);
        	printf("%s\n",buffer);
        	free(buffer);
	}
	
	char friendip[30][30];
	char friendid[30][10];
	int frd=0;
	int i;
        for(i=0;i<numfrd;i++){
                printf("Enter a friend's name: ");
                scanf("%s",friend);
		if(i==0){
                	s_send(sender, friend);
                	char *string= s_recv(sender);
			printf("%s\n",string);
			free(string);
		}
                st[0]='\0';
		sprintf(st,"1 %s",friend);
                s_send(requester, st);
                char *string= s_recv(requester);

                if(strcmp(string,"-1")==0) {
                        printf("%s is OFFLINE.\n",friend);
                        exit(0);
                }
		else{
                        char *token=strtok(string,"\n");
                        int number;
                        number = atoi(token);
			int j;
                        for(j=0;j<number;j++){
                                token = strtok(NULL," ");
                                strcpy(friendip[frd], token);
                                printf ("Friend's IP: %s\t\t",friendip[frd]);
                                token = strtok(NULL,"\n");
                                strcpy(friendid[frd], token);
                                printf ("Friend's ID: %s\n",friendid[frd]);
                                frd++;
                        }
		}
		free(string);
        }

        st[0]='\0';
        sprintf(st,"1 %s",name);
        s_send(requester,st);
        char *string= s_recv(requester);
        if(strcmp(string,"-1")!=0) {
                char *token=strtok(string,"\n");
                int number;
                number = atoi(token);
                char myip[30];
                sprintf(myip,"tcp://%s:5501",argv[1]);
                for(i=0;i<number;i++){
                        token = strtok(NULL," ");
                        if(strcmp(token,myip)!=0){
                                strcpy(friendip[frd], token);
                                printf ("Your IP: %s\t\t",friendip[frd]);
                                token = strtok(NULL,"\n");
                                strcpy(friendid[frd], token);
                                printf ("Your ID: %s\n",friendid[frd]);
                                frd++;
                        }
                        else if(strcmp(token,myip)==0)
                                token = strtok(NULL,"\n");

                }
                free(string);
        }
	
	char tempfrd[2];
	printf("frd: %d\n",frd);
	sprintf(tempfrd,"%d",frd);
	s_send(sender,tempfrd);
	buffer = s_recv(sender);
	printf("%s\n",buffer);
	free(buffer);
	
	for(i=0;i<frd;i++){
		st[0]='\0';
		sprintf(st,"%s %s",friendip[i],friendid[i]);
		s_send(sender,st);
		char *buffer = s_recv(sender);
		printf("%s\n",buffer);
		free(buffer);
	}

	// receive msg
        void *receiver[30];     // fix 30 friends for one group
        pthread_t rec_data;

        for(i=0;i<frd;i++){
                receiver[i] = zmq_socket (context, ZMQ_SUB);
                zmq_connect (receiver[i], (char *)friendip[i]);
                zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
                pthread_create(&rec_data, NULL,receive_data,receiver[i]);
        }

	char temp[30];
	void *recv_noti = zmq_socket(context,ZMQ_SUB);
	sprintf(temp,"tcp://%s:5506",argv[1]);
	zmq_connect(recv_noti,temp);
        zmq_setsockopt (recv_noti, ZMQ_SUBSCRIBE, NULL, 0);

//------------------------------------------------------------
	new_ip newip;
	newip.recv_noti=recv_noti;
	newip.recv=&receiver[frd];
	newip.context=context;
	newip.num=frd;
	newip.i=0;
	newip.rec_d=rec_data;
        for(i=0;i<frd;i++){
                memcpy(&newip.friendip[i],&friendip[i],sizeof(friendip[0]));
                memcpy(&newip.friendid[i],&friendid[i],sizeof(friendid[0]));
        }
	pthread_t rec_ip;
        pthread_create(&rec_ip, NULL,receive_ip,&newip);
//------------------------------------------------------------

	printf("Loading ...\n");
	sleep(5);

	while(1){

        	char string[256];
        	printf("%s: ",name);
        	fgets(string,sizeof string, stdin);
                if(strlen(string)==1) {
                        printf("\r                    \n");
                        continue;
                }
		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';      // fix useless \n
		if(strcmp(temp,"EXIT")==0){
			st[0]='\0';
			sprintf(st,"0 OFFLINE %s %s %s", id, name, argv[1]);
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
            		char *buffer = s_recv(requester);
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
	pthread_join(rec_data,NULL);
	pthread_join(rec_ip,NULL);
    	zmq_close (sender);
    	zmq_close (requester);
    	zmq_ctx_destroy (context);
    	return 0;
}
