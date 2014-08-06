#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
	void *req;
	void *res;
	void **rec_f;
	int num;
	pthread_t rec_d;
	char myip[30];
	char friendip[10][30];
	
}new_friend;

typedef struct{
	void *res;
	void **rec_f;
        int num;
        pthread_t rec_d;
}new_req;

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

static void *receive_req (void *receiver) {
	new_req *y = receiver;
        while(1) {
                char *string = s_recv(y->res);
		if(strstr(string,"4")!=NULL){
			//  fix here!!!
			char *token=strtok(string," ");
			token = strtok(NULL," ");
			zmq_connect (y->rec_f[y->num], token);
			zmq_setsockopt (y->rec_f[y->num], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&y->rec_d, NULL,receive_data,y->rec_f[y->num]);
			y->num++;
			s_send(y->res,"successful!!");
                	free (string);
		}
        }
        zmq_close(y->res);
        return NULL;
}

static void *receive_newcon (void *responder) {
	new_friend *x = responder;
	//void **x = (void **)responder;
        while(1) {
                //char *string = s_recv(x[0]);
                char *string = s_recv(x->res);
                if((strstr(string,"OPEN")!=NULL)){
			char *token=strtok(string," ");
                        token = strtok(NULL," ");
                        token = strtok(NULL," ");
			char friendip[30];
			sprintf(friendip,"%s",token);
			//printf("Friend's IP: %s\n",token);

                        token = strtok(NULL," ");
			char newip[30];
			sprintf(newip,"tcp://%s:5501",token);
                        printf("New connection request: %s\n", newip);
			//printf("num: %d\n",x->num);

			int checked=0;
			for(int i=0;i<x->num;i++)
				if(strcmp(x->friendip[i],friendip)==0) checked=1; 
			//printf("checked: %d\n",checked);	
			if((strcmp(newip,x->myip)!=0)&&(checked||strcmp(friendip,x->myip)==0)){
				zmq_connect (x->rec_f[x->num], newip);
				zmq_setsockopt (x->rec_f[x->num], ZMQ_SUBSCRIBE, NULL, 0);
				pthread_create(&x->rec_d, NULL,receive_data,x->rec_f[x->num]);
				x->num++;
				//printf("num: %d\n",x->num);

				// do sth.
				if(checked==1){
					char req_friend[70];
					sprintf(req_friend,"4 %s %s",x->myip,token); 
					s_send(x->req,req_friend); 
					char *buffer = s_recv(x->req);
					//printf("%s\n",buffer);
					free(buffer);
				}
			}
        	}
                free (string);
        }
        //zmq_close(x[0]);
        zmq_close(x->res);
        return NULL;
}

int main(int argc, char *argv[]){

	char ip[30] = "tcp://";
	strcat(ip,argv[1]);
	//printf("IP: %s\n", ip);
    
	// send msg
    	void *context = zmq_ctx_new ();
    	void *sender = zmq_socket (context, ZMQ_PUB);
    	zmq_bind (sender, "tcp://*:5501");
    
	// send ip & status
	char reqip[30];		// fix
	//sprintf(reqip,"%s:5502",ip);		// fix
	//printf("%s\n",reqip);
    	void *requester = zmq_socket (context, ZMQ_REQ);
    	//zmq_connect (requester, reqip);	// fix
	zmq_connect (requester, "tcp://192.168.179.94:5502");

	// get new connection
	void *responder = zmq_socket (context, ZMQ_SUB);
    	zmq_connect (responder, "tcp://192.168.179.94:5503");
        zmq_setsockopt (responder, ZMQ_SUBSCRIBE, NULL, 0);

	// get request from friends
	void *responder2 = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder2, "tcp://*:5504");

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
        	sprintf(st,"0 ONLINE ");    //1
        	strcat(st, name);
        	strcat(st, " ");
		strcat(st, argv[1]);
		s_send(requester, st);
        	char *buffer = s_recv(requester);
        	printf("%s\n",buffer);
        	free(buffer);
	}
	else{
		st[0]='\0';
        	sprintf(st,"0 OFFLINE ");   //2
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
    
    	st[0]='\0';
    	printf("Get list of friends(Y/N): ");
    	scanf("%s",st);
    	if(strcmp(st,"Y")==0){
        	s_send(requester, "2");
        	char *buffer = s_recv(requester);
        	printf("%s\n",buffer);
        	free(buffer);
    	}
//---------------------------------------------
	st[0]='\0';	
	printf("Join an available chatting room(Y/N): ");
	scanf("%s",st);	
   	char friend[20]; 
	int oldchat = 0;	// 1 = want to join
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
		s_send(requester, st);
    		char *frd_ip = s_recv(requester);
    		//printf("%s\n",frd_ip);
		if(strcmp(frd_ip,"EXIT")==0) {
			printf("%s is OFFLINE.\n",friend);
			exit(0);
		}
		strcpy(friendip[i], frd_ip);
		printf ("Friend's ip: %s\n",friendip[i]);
	}
    
    	// receive msg
	int rc;
	void *receiver[10];
	pthread_t rec_data;

	for(int i=0;i<10;i++){
    		receiver[i] = zmq_socket (context, ZMQ_SUB);
		if(i<numfrd){
    			rc = zmq_connect (receiver[i], (char *)friendip[i]);
        		zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&rec_data, NULL,receive_data,receiver[i]);
		}
	}

	new_friend nf;
	nf.req=requester;
	nf.res=responder;
	nf.rec_f=receiver;
	nf.num=numfrd;
	nf.rec_d=rec_data;
	sprintf(nf.myip,"tcp://%s:5501",argv[1]);
	for(int i=0;i<numfrd;i++)
		memcpy(&nf.friendip[i],&friendip[i],sizeof(friendip[0]));

	new_req nq;
	nq.res=responder2;
	nq.rec_f=receiver;
        nq.num=numfrd;
        nq.rec_d=rec_data;
/*
	void *x[2];
	x[0]=responder;
	x[1]=sender;
*/    
	//pthread_t rec_data;
	//pthread_create(&rec_data, NULL,receive_data,receiver);
	//pthread_create(&rec_newcon, NULL,receive_newcon,(void *)x);

	pthread_t rec_newcon;
	pthread_create(&rec_newcon, NULL,receive_newcon,&nf);
    
	pthread_t rec_req;
	pthread_create(&rec_req, NULL,receive_req,&nq);

	printf("Loading ...\n");
	sleep(5);
    
	while(rc==0){
		//printf("oldchat: %i\n",oldchat);

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
        	if(strcmp(temp,"LIST")!=0) {
			s_send (sender, string);
		}
    	}
    	zmq_close (sender);
    	zmq_close (requester);
    	zmq_ctx_destroy (context);
    	return 0;
}
