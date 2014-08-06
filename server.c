#include "zhelpers.h"
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

int main (void)
{
    void *context = zmq_ctx_new ();
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_bind (receiver, "tcp://*:5557");

    printf ("Press Enter to get online list: ");
    getchar ();
    printf ("Loading…\n");

    char *text;
    for (int i=0;i<10;i++) {
	text = "Hello Server."; 
        s_send (sender, text);
    }
    printf ("Text: %s\n", text);
    zmq_close (sender);

    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_bind (receiver, "tcp://*:5558");

    while (1) {
        char *string = s_recv (receiver);
        printf ("%s\n", string);     //  Show progress
        fflush (stdout);
        s_sleep (atoi (string));    //  Do the work
        free (string);
    }   

    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}
