#include "zhelpers.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
	// send msg 
    void *context = zmq_ctx_new ();
    void *sender = zmq_socket (context, ZMQ_PUSH);
    zmq_bind (sender, "tcp://*:5556");

    void *sender2 = zmq_socket (context, ZMQ_PUSH);
    zmq_connect (sender2, "tcp://localhost:5502");

    	// receive msg 
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_bind (receiver, "tcp://*:5557");
	
	printf("[----Chat Room----]\n");
        printf(">>>>Client1\n");
        char st[20];
	printf("Your status(online/offline): ");
	scanf("%s",&st[0]);
	if(strcmp(st,"online")==0){
		strcat(st, " Client1");
		printf("%s",st);
		s_send(sender2, st); 
	}	
	else{
		st[0]='\0';
		strcat(st,"offline Client1");
		printf("%s\n",st);
		s_send(sender2, st);
		sleep(3);
		exit(0);
	}
	//strcat(st," Client1");
	//s_send(sender2, st);

	while(1){
                zmq_pollitem_t items [] = {
                        { sender, 0, ZMQ_POLLOUT, 0 },
                        { receiver, 0, ZMQ_POLLIN, 0 }
                };
                zmq_poll (items, 2, -1);

        if (items [0].revents & ZMQ_POLLOUT) {
        	char string[256];
        	printf("Client1: ");
        	fgets(string,sizeof string, stdin);
		//if(strstr(string,"EXIT")!=NULL){
		char temp[256];
		sprintf(temp,"%s",string);
		temp[strlen(string)-1] = '\0';
		if(strcmp(temp,"EXIT")==0){
			st[0]='\0';
                strcat(st,"offline Client1");
                printf("%s\n",st);
                s_send(sender2, st);
                sleep(3);
			printf("Exit now ...\n");
			exit(0);
		}
		//printf("%d\n",strlen(string));		
		//scanf("%s",&string);
        	printf("your msg: %s",string);
        	s_send (sender, string);
	}
        if (items [1].revents & ZMQ_POLLIN) {
                char *string = s_recv(receiver);
                printf("Message from Client2: ");
                printf ("%s", string);     //  Show progress
                fflush (stdout);
                s_sleep (atoi (string));    //  Do the work
                free (string);
	}
	}

/*
    //  Process messages from both sockets
    while (1) {
        char msg [256];
        zmq_pollitem_t items [] = {
            { sender, 0, ZMQ_POLLIN, 0 },
            { receiver, 0, ZMQ_POLLIN, 0 }
        };
        zmq_poll (items, 2, -1);
	printf("start\n");
	if (items [0].revents & ZMQ_POLLIN) {
            int size = zmq_recv (sender, msg, 255, 0);
            if (size != -1) {
                // task
        char *string;
	printf("Client1: ");
	scanf("%s",string);
        s_send (sender, string);
 		}
        }
        if (items [1].revents & ZMQ_POLLIN) {
            int size = zmq_recv (receiver, msg, 255, 0);
            if (size != -1) {
                // task
        char *string = s_recv(receiver);
	printf("Client2: ");
        printf ("%s\n", string);     //  Show progress
        fflush (stdout);
        s_sleep (atoi (string));    //  Do the work
        free (string);
 		}
        }
    }
*/
    zmq_close (sender);
    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}
