#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <pthread.h>

#include <transport.h>
#include "recvthread.h"

/// PacketQueue
typedef struct Item Item;
struct {
    struct Item {
        Packet packet;
        Item *next;
    } *head;
    Item *tail;
    int inQueue;
} PacketQ;

void AddItemToQueue(Item *item) {
    if (PacketQ.inQueue == 0) {
        PacketQ.head = item;
        PacketQ.tail = item;
    } else {
        PacketQ.tail->next = item;
        PacketQ.tail = item;
    }
    PacketQ.inQueue++;
}
Item* getItemFromQueue() {
    if (PacketQ.inQueue == 0) {
        return NULL;
    } else {
        Item *item = PacketQ.head;
        PacketQ.head = item->next;
        PacketQ.inQueue--;
        return item;
    }
}
///

typedef struct ThreadArg ThreadArg;
struct ThreadArg {
    char endpointRecv[32];
    char endpointInterThread[32];
    void *context;
};

pthread_t Pthread;

static void *recvThread(void *argv) {
    ThreadArg *arg;
    void *recvSocket, *signalSocket;
    Packet packet;
    int packetSize;
    Item *item;
    
    
    arg = (ThreadArg *)argv;
    
    recvSocket = zmq_socket(arg->context, ZMQ_REP);
    signalSocket = zmq_socket(arg->context, ZMQ_PAIR);
    zmq_bind(recvSocket, arg->endpointRecv);
    zmq_bind(signalSocket, arg->endpointInterThread);
    
    zmq_pollitem_t pollitems[] = {
        { recvSocket, 0, ZMQ_POLLIN, 0},
        { signalSocket, 0, ZMQ_POLLIN, 0},
    };
    
    while (1) {
        zmq_poll(pollitems, 2, -1);
        if (pollitems[0].revents & ZMQ_POLLIN) {
            packetSize = zmq_recv(recvSocket, &packet, PACKET_SIZE_MAX, 0);
            switch (packet.tag) {
                case TAG_OK:
                    packet.tag = TAG_OK;
                    packet.dataSize = 0;
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                    break;
                case TAG_MSG_CLIENT:
                case TAG_MSG_USER:
                    item = malloc(sizeof(Item));
                    memcpy(&item->packet, &packet, packetSize);
                    item->next = NULL;
                    AddItemToQueue(item);
                    packet.tag = TAG_OK;
                    packet.dataSize = 0;
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                    break;
                default:
                    zmq_send(recvSocket, NULL, 0, 0);
                    break;
            }
        }
        if (pollitems[1].revents & ZMQ_POLLIN) {
            packetSize = zmq_recv(signalSocket, &packet, PACKET_SIZE_MAX, 0);
            if (packet.tag == TAG_MSG_CLIENT) {
                item = getItemFromQueue();
                if (item != NULL) {
                    item->packet.tag = TAG_OK;
                    zmq_send(signalSocket, &item->packet, PACKET_SIZE(item->packet), 0);
                    free(item);
                } else {
                    zmq_send(signalSocket, NULL, 0, 0);
                }
            } else if (packet.tag == TAG_CLOSE) {
                zmq_send(signalSocket, NULL, 0, 0);
                break;
            }
        }
    }
    
    zmq_close(signalSocket);
    zmq_close(recvSocket);
    free(argv);
    return NULL;
}

int startRecvData(void *context, int port) {
    ///
    // initial queue
    ///
    PacketQ.head = NULL;
    PacketQ.tail = NULL;
    PacketQ.inQueue = 0;
    
    ///
    // start thread
    ///
    ThreadArg *arg = malloc(sizeof(ThreadArg));
    sprintf(arg->endpointRecv, "tcp://*:%d", port);
    strcpy(arg->endpointInterThread, ENDPOINT_INTER_THREAD);
    arg->context = context;
    pthread_create(&Pthread, NULL, recvThread, arg);
    
    return 0;
}
int stopReceiveData(void *context) {
    void *signalSocket;
    Packet packet;
    signalSocket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(signalSocket, ENDPOINT_INTER_THREAD);
    packet.tag = TAG_CLOSE;
    packet.dataSize = 0;
    zmq_send(signalSocket, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(signalSocket, NULL, 0, 0);
    zmq_close(signalSocket);
    zmq_ctx_destroy(context);
    
    if (pthread_join(Pthread, NULL)) {
        fprintf(stderr, "Error: join thread error\n");
        return 1;
    }
    return 0;
}