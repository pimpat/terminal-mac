#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
        void *req;
        void *res;
        void **recv;
	void *context;
        int num;
	int i;
        pthread_t rec_d;
        char myip[30];
        char friendip[30][30];
        char friendid[30][10];
        char myname[20];
	char myid[10];
	void *noti;
}new_friend;

typedef struct{
	void *res;
	void **recv;
	void *context;
        int num;
	int i;
        pthread_t rec_d;
	char friendip[30][30];
	char friendid[30][10];
	void *noti;
}new_req;

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

static void *receive_req (void *receiver) {
        new_req *y = receiver;
        while(1) {
                char *string = s_recv(y->res);
                if (string == NULL)
                        printf("string == NULL\n");
                if(strstr(string,"4")!=NULL){
			printf("\n<receive_req>\n");
                        char *token=strtok(string," ");
                        token = strtok(NULL," ");

			y->recv[y->i] = zmq_socket (y->context, ZMQ_SUB);
                        zmq_connect (y->recv[y->i], token);
                        zmq_setsockopt (y->recv[y->i], ZMQ_SUBSCRIBE, NULL, 0);
                        pthread_create(&y->rec_d, NULL,receive_data,y->recv[y->i]);
			y->i++;
			printf("value of i: %d\n",y->i);

                        sprintf(y->friendip[y->num],"%s",token);
                        printf("friend ip: %s\n",token);

                        token = strtok(NULL," ");
                        sprintf(y->friendid[y->num],"%s",token);
                        printf("friend id: %s\n",token);
                        y->num++;
                        s_send(y->res,"successful!!");
                        free (string);
                        printf("----------------------------\n");

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
                if (string == NULL)
                        printf("2 string == NULL\n");
                if((strstr(string,"OPEN")!=NULL)){
			printf("\n<receive_newcon>\n");
                        char *token=strtok(string," ");
                        token = strtok(NULL," ");

                        token = strtok(NULL," ");
                        char friendip[30];
                        sprintf(friendip,"%s",token);
                        printf("Friend's IP: %s\n",friendip);

                        token = strtok(NULL," ");
                        char friendname[20];
                        sprintf(friendname,"%s",token);
                        printf("Friend's name: %s\n",friendname);

                        token = strtok(NULL," ");
                        char new_ip[30];        // 192.168.179.xxx
                        char newip[30];         // tcp://192.168.179.xxx:5501
                        sprintf(new_ip,"%s",token);
                        sprintf(newip,"tcp://%s:5501",token);
                        printf("New IP: %s\n", newip);

                        token = strtok(NULL," ");
                        char newid[10];
                        sprintf(newid,"%s",token);
                        printf("New ID: %s\n",newid);

                        int checked=0;
                        int same=0;
                        int i;
                        for(i=0;i<x->num;i++){
                                if(strcmp(x->friendip[i],friendip)==0) checked=1;       // check friend in this group
                                if(strcmp(x->friendip[i],newip)==0) same=1;     // you already have new friend
                        }
                        if(strcmp(x->myip,friendip)==0) checked=1;
                        //printf("checked: %d\n",checked);

                        if(strcmp(newip,x->myip)!=0 && checked==1 && same==0){
                                sprintf(x->friendip[x->num],"%s",newip);
                                sprintf(x->friendid[x->num],"%s",newid);

				x->recv[x->i] = zmq_socket (x->context, ZMQ_SUB);
                                zmq_connect (x->recv[x->i], newip);
                                zmq_setsockopt (x->recv[x->i], ZMQ_SUBSCRIBE, NULL, 0);
                                pthread_create(&x->rec_d, NULL,receive_data,x->recv[x->i]);
                                x->i++;
                                printf("value of i: %d\n",x->i);

                                char str[50];
                                sprintf(str,"NEWIP %s",newip);
                                s_send(x->noti,str);

                                x->num++;
                                //printf("num: %d\n",x->num);
                                if(strcmp(friendname,x->myname)!=0){
                                        //printf("Friend's name is not my name.\n");
                                        char req_friend[70];
                                        sprintf(req_friend,"4 %s %s %s",x->myip,x->myid,new_ip);
                                        s_send(x->req,req_friend);
                                        char *buffer = s_recv(x->req);
                                        printf("%s\n",buffer);
                                        free(buffer);
                                }
                        }
                }
                free (string);
        }
        //zmq_close(x[0]);
        zmq_close(x->res);
        zmq_close(x->req);
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
	char st[70];
	st[0]='\0';	
	//scanf("%s",st);	

	char name[20];
	char id[10];
	char* string = s_recv(fwder);
	sprintf(name,"%s",string);
	s_send(fwder,"received name");
	printf("name: %s",name);
	string = s_recv(fwder);
	sprintf(id,"%s",string);
	s_send(fwder,"received id");
	printf("id: %s",id);

   	char friend[20]; 
	int oldchat = 0;	// 1 = want to join
	int numfrd;
	string = s_recv(fwder);
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

	char friendip[30][30];
	char friendid[30][10];
	int frd;
	int i;
	string = s_recv(fwder);
	number = atoi(token);
	s_send(fwder,"received number");
	printf("number: %d\n",number);
	
	for(i=0;i<number;i++){
		char* string = s_recv(fwder); 
		//printf("%s\n",string);
		char *token=strtok(string," ");
		strcpy(friendip[i], token);
		printf ("Friend's ip: %s\n",friendip[i]);
		token = strtok(NULL," ");
		strcpy(friendid[i], token);
		printf ("Friend's id: %s\n",friendid[i]);
		s_send(fwder,"received friendip/id ");
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
