#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){

	// send msg 
        char ip[30]="tcp://*:5557";
    	void *context = zmq_ctx_new ();
    	void *sender = zmq_socket (context, ZMQ_PUSH);
    	zmq_bind (sender, ip);

	// send ip & status
    	void *sender2 = zmq_socket (context, ZMQ_PUSH);
    	zmq_connect (sender2, "tcp://localhost:5502");

    	// receive msg 
    	void *receiver = zmq_socket (context, ZMQ_PULL);
    	zmq_connect (receiver, "tcp://localhost:5556");

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

        while(1){
		zmq_pollitem_t items [] = {
            		{ sender, 0, ZMQ_POLLOUT, 0 },
           		{ receiver, 0, ZMQ_POLLIN, 0 }
        	};
        	zmq_poll (items, 2, -1);

        if (items [0].revents & ZMQ_POLLOUT) {
                char string[256];
                printf("%s: ",name);
                fgets(string,sizeof string, stdin);
                //if(strstr(string,"EXIT")!=NULL){
                char temp[256];
                sprintf(temp,"%s",string);
                temp[strlen(string)-1] = '\0';
                if(strcmp(temp,"EXIT")==0){
                        st[0]='\0';
                        strcat(st, "OFFLINE ");
                        strcat(st, name);
                	strcat(st, " ");
                	strcat(st, ip);
                        //printf("%s\n",st);
                        s_send(sender2, st);
                        sleep(3);
                        printf("Exit now ...\n");
                        exit(0);
                }
                //printf("%d\n",strlen(string));
                //scanf("%s",&string);
                //printf("your msg: %s",string);
                s_send (sender, string);
        }
        if (items [1].revents & ZMQ_POLLIN) {
		char *string = s_recv(receiver);
        	printf("Message: ");
        	printf ("%s", string);     //  Show progress
        	fflush (stdout);
        	s_sleep (atoi (string));    //  Do the work
        	free (string);
        }
	}

    	zmq_close (sender);
    	zmq_close (sender2);
    	zmq_close (receiver);
    	zmq_ctx_destroy (context);
    	return 0;
}
