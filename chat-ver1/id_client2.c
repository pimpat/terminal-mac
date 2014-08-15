#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
	void *req;
	void *res;
	//void **rec_f;
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
	
}new_friend;

typedef struct{
	void *res;
	//void **rec_f;
	void **recv;
	void *context;
        int num;
	int i;
        pthread_t rec_d;
	char friendip[30][30];
	char friendid[30][10];
}new_req;

static void *receive_data (void *receiver) {
	while(1) {
		char *string = s_recv(receiver);
        	if(strlen(string)==1) continue;
		printf ("%s",string);
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
			printf("1 string == NULL\n");
		if(strstr(string,"4")!=NULL){
			printf("\n<receive_req>\n");
			char *token=strtok(string," ");
			token = strtok(NULL," ");
/*
    			y->rec_f[y->num] = zmq_socket (context, ZMQ_SUB);
			zmq_connect (y->rec_f[y->num], token);
			zmq_setsockopt (y->rec_f[y->num], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&y->rec_d, NULL,receive_data,y->rec_f[y->num]);
*/

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
			char new_ip[30];	// 192.168.179.xxx
			char newip[30];		// tcp://192.168.179.xxx:5501
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
				if(strcmp(x->friendip[i],friendip)==0) checked=1;	// check friend in this group 
				if(strcmp(x->friendip[i],newip)==0) same=1;	// you already have new friend
			}
			if(strcmp(x->myip,friendip)==0) checked=1; 
			printf("checked: %d\n",checked);	
			printf("same: %d\n",same);	
			printf("----------------------------\n");

			if(strcmp(newip,x->myip)!=0 && checked==1 && same==0){
				sprintf(x->friendip[x->num],"%s",newip);
				sprintf(x->friendid[x->num],"%s",newid);
/*
    				x->rec_f[y->num] = zmq_socket (context, ZMQ_SUB);
				zmq_connect (x->rec_f[x->num], newip);
				zmq_setsockopt (x->rec_f[x->num], ZMQ_SUBSCRIBE, NULL, 0);
				pthread_create(&x->rec_d, NULL,receive_data,x->rec_f[x->num]);
*/

                                x->recv[x->i] = zmq_socket (x->context, ZMQ_SUB);
                                zmq_connect (x->recv[x->i], newip);
                                zmq_setsockopt (x->recv[x->i], ZMQ_SUBSCRIBE, NULL, 0);
                                pthread_create(&x->rec_d, NULL,receive_data,x->recv[x->i]);
				x->i++;
				printf("value of i: %d\n",x->i);

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
        	//printf("%s\n",buffer);
        	free(buffer);
	}
	else{
		st[0]='\0';
        	sprintf(st,"0 OFFLINE %s %s %s", id, name, argv[1]);
		s_send(requester, st);
        	char *buffer = s_recv(requester);
        	//printf("%s\n",buffer);
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
        	printf("%s",buffer);
        	free(buffer);
    	}

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
	char friendip[30][30];
	char friendid[30][10];
	int frd=0;
	int i;
	for(i=0;i<numfrd;i++){
		printf("Enter a friend's name: ");
		scanf("%s",friend);
    		st[0]='\0';
		sprintf(st,"1 %s",friend);
		s_send(requester, st);
    		char *string= s_recv(requester);
    		//printf("%s\n",string);

		if(strcmp(string,"-1")==0) {
			printf("%s is OFFLINE.\n",friend);
			exit(0);
		}
		else{
			char *token=strtok(string,"\n");
			int number;
			number = atoi(token);
			for(i=0;i<number;i++){
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
    
    	// receive msg
	void *receiver[30];	// fix 30 id for one group
	pthread_t rec_data;
/*
	for(i=0;i<30;i++){
    		receiver[i] = zmq_socket (context, ZMQ_SUB);
		if(i<frd){
    			rc = zmq_connect (receiver[i], (char *)friendip[i]);
			//printf("rc:%d\n", rc);
			//printf("friendip[i]: %s\n", (char *)friendip[i]);
        		zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
			pthread_create(&rec_data, NULL,receive_data,receiver[i]);
		}
	}
*/

	for(i=0;i<frd;i++){
    		receiver[i] = zmq_socket (context, ZMQ_SUB);
    		zmq_connect (receiver[i], (char *)friendip[i]);
        	zmq_setsockopt (receiver[i], ZMQ_SUBSCRIBE, NULL, 0);
		pthread_create(&rec_data, NULL,receive_data,receiver[i]);
	}

	new_friend nf;
	nf.req=requester;
	nf.res=responder;
	//nf.rec_f=receiver;
	nf.recv=&receiver[frd];
	nf.context=context;
	nf.num=frd;
	nf.i=0;
	nf.rec_d=rec_data;
	sprintf(nf.myip,"tcp://%s:5501",argv[1]);
	for(i=0;i<frd;i++){
		memcpy(&nf.friendip[i],&friendip[i],sizeof(friendip[0]));
		memcpy(&nf.friendid[i],&friendid[i],sizeof(friendid[0]));
	}
	sprintf(nf.myname,"%s",name);
	sprintf(nf.myid,"%s",id);

	new_req nq;
	nq.res=responder2;
	//nq.rec_f=receiver;
	nq.recv=&receiver[frd];
	nq.context=context;
        nq.num=frd;
        nq.i=0;
        nq.rec_d=rec_data;
	for(i=0;i<frd;i++){
                memcpy(&nq.friendip[i],&friendip[i],sizeof(friendip[0]));
                memcpy(&nq.friendid[i],&friendid[i],sizeof(friendid[0]));
        }
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
			int i;
			for(i=0;i<frd;i++){
            			st[0]='\0';
            			//sprintf(st,"3 %s %s",friendip[0],argv[1]);
				sprintf(st,"3 %s %s %s %s",friendip[i],friend,argv[1],id);
            			s_send(requester, st);
                        	char *buffer = s_recv(requester);
				if (buffer==NULL) printf("NULLL");
				//char *buffer = NULL;	
                        	printf("%s\n",buffer);
                        	free(buffer);
			}
			oldchat=0;
		}

        	char string[256];
        	printf("%s: ",name);
		//scanf("%s",string);
        	fgets(string,sizeof string, stdin);
        	if(strlen(string)==1) {
			printf("\r                    \n"); 
			continue;
		}
		//if(strstr(string,"EXIT")!=NULL){
		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';      // fix useless \n
		//if(strcmp(string,"EXIT")==0){     // use for scanf

		if(strcmp(temp,"EXIT")==0){
            		st[0]='\0';
        		sprintf(st,"0 OFFLINE %s %s %s", id, name, argv[1]);
            		//printf("%s\n",st);
            		//sprintf(string,"%s is OFFLINE.\n",name);
            		//s_send (sender, string);
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
		//printf("%d\n",strlen(string));
        	if(strcmp(temp,"LIST")!=0) {
			char tempstring[256];
			sprintf(tempstring,"%s: %s",name,string);
			s_send (sender, tempstring);
			//s_send (sender, string);
		}
    	}
	pthread_join(rec_data,NULL);
	pthread_join(rec_newcon,NULL);
	pthread_join(rec_req,NULL);
    	zmq_close (sender);
    	zmq_close (requester);
    	zmq_ctx_destroy (context);
    	return 0;
}
