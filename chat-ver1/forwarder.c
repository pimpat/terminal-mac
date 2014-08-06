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
	void *noti;
	
}new_friend;

typedef struct{
	void *noti;
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
			char *token=strtok(string," ");
			token = strtok(NULL," ");
			zmq_connect (y->rec_f[y->num], token);
			zmq_setsockopt (y->rec_f[y->num], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&y->rec_d, NULL,receive_data,y->rec_f[y->num]);
			y->num++;
			s_send(y->res,"successful!!");

			char str[50];
                        sprintf(str,"NEWIP %s",token);
                        s_send(y->noti,str);
                	free (string);
		}
        }
        zmq_close(y->res);
	zmq_close(y->noti);
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

			int checked=0;
			int i;
			for(i=0;i<x->num;i++)
				if(strcmp(x->friendip[i],friendip)==0) checked=1; 
			//printf("checked: %d\n",checked);	

			if((strcmp(newip,x->myip)!=0)&&(checked||strcmp(friendip,x->myip)==0)){
				zmq_connect (x->rec_f[x->num], newip);
				zmq_setsockopt (x->rec_f[x->num], ZMQ_SUBSCRIBE, NULL, 0);
				pthread_create(&x->rec_d, NULL,receive_data,x->rec_f[x->num]);
				x->num++;
				//printf("num: %d\n",x->num);
				char str[50];
				sprintf(str,"NEWIP %s",newip); 
				s_send(x->noti,str);

				if(checked==1){
					char req_friend[70];
					sprintf(req_friend,"4 %s %s",x->myip,token); 
					s_send(x->req,req_friend); 
					char *buffer = s_recv(x->req);
					//printf("%s\n",buffer);
					free(buffer);
				}
			// continue here!!!
			}
        	}
                free (string);
        }
        //zmq_close(x[0]);
        zmq_close(x->res);
	zmq_close(x->noti);
        return NULL;
}

int main(int argc, char *argv[]){

	// send msg
    	void *context = zmq_ctx_new ();
    	void *sender = zmq_socket (context, ZMQ_PUB);
    	zmq_bind (sender, "tcp://*:5501");
    
	// send ip & status
    	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://192.168.179.94:5502");

	// get new connection
	void *responder = zmq_socket (context, ZMQ_SUB);
    	zmq_connect (responder, "tcp://192.168.179.94:5503");
        zmq_setsockopt (responder, ZMQ_SUBSCRIBE, NULL, 0);

	// receive request from friends
	void *responder2 = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder2, "tcp://*:5504");

	// fwder msg to clients
	void *fwder = zmq_socket (context, ZMQ_REP);
	zmq_bind (fwder, "tcp://*:5505");

	// noti to fwder
	void *noti = zmq_socket(context, ZMQ_PUB);
    	zmq_bind (noti, "tcp://*:5506");

	printf("[----Forwarder----]\n");
	printf("Forwarder is working ...\n");
	char st[50];
	st[0]='\0';	
	//scanf("%s",st);	
   	char friend[20]; 
	int oldchat = 0;	// 1 = want to join
	int numfrd;
	char* string = s_recv(fwder);
  	if(strcmp(string, "Y")==0){
		numfrd=1; 
		oldchat=1;
		printf("oldchat: 1\n");
		printf("numfrd: 1\n");
		s_send(fwder,"received oldchat=1, numfrd=1");
	}
  	if(strcmp(string, "N")==0){
		printf("oldchat: 0\n");
		s_send(fwder,"received oldchat=0");
		string = s_recv(fwder); 
		numfrd=atoi(string);
		printf("numfrd: %d\n",numfrd);
		s_send(fwder,"received numfrd");
	}

	char friendip[numfrd][30];
	int i;
	for(i=0;i<numfrd;i++){
		char* string = s_recv(fwder); 
		//printf("%s\n",string);
		strcpy(friendip[i], string);
		printf ("Friend's ip: %s\n",friendip[i]);
		s_send(fwder,"received friend's IP");
	}
    
    	// receive msg
	int rc;
	void *receiver[10];	// fix 10 friends for one group
	pthread_t rec_data;

	for(i=0;i<10;i++){
    		receiver[i] = zmq_socket (context, ZMQ_SUB);
		if(i<numfrd){
    			rc = zmq_connect (receiver[i], (char *)friendip[i]);
        		zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&rec_data, NULL,receive_data,receiver[i]);
		}
	}

	new_friend nf;
	nf.noti=noti;
	nf.req=requester;
	nf.res=responder;
	nf.rec_f=receiver;
	nf.num=numfrd;
	nf.rec_d=rec_data;
	sprintf(nf.myip,"tcp://%s:5501",argv[1]);
	for(i=0;i<numfrd;i++)
		memcpy(&nf.friendip[i],&friendip[i],sizeof(friendip[0]));

	new_req nq;
	nq.noti=noti;
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
    
	while(1){
		//printf("oldchat: %i\n",oldchat);

		if(oldchat==1){
            		st[0]='\0';
            		sprintf(st,"3 ");
			strcat(st, friendip[0]);
			strcat(st, " ");
			strcat(st, argv[1]);
            		s_send(requester, st);
                        char *buffer = s_recv(requester);
                        printf("%s\n",buffer);
                        free(buffer);
			oldchat=0;
		}

		char *string = s_recv(fwder);
		printf("Fwd msg: %s",string);
		s_send(fwder,"received");	// send ack. to client
		s_send(sender,string);		// forward msg to this group
    	}
	zmq_close (fwder);
    	zmq_close (sender);
    	zmq_close (requester);
    	zmq_ctx_destroy (context);
    	return 0;
}
