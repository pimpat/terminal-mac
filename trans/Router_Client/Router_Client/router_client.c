//
//  router_client.c
//  TransportLayer
//
//  Created by Pimpat on 8/29/2557 BE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zmq.h"
#include <sqlite3.h>
#include <pthread.h>
#include <string.h>
#include "transport.h"
#include "funcdb.h"
#include "recvthread.h"
#include "router_client.h"

#define ENDPOINT_ROUTER "tcp://localhost:5500"
#define MAX_PACKET_SIZE 264
#define MAX_ID_LENGTH 16
#define MAX_NAME_LENGTH MAX_ID_LENGTH

typedef struct ThreadArg ThreadArg;
struct ThreadArg {
    char endpointRecv[32];
    char endpointInterThread[32];
};

void *context;
pthread_t Pthread;
sqlite3 *DB;
void *Context;
Client myClient;

static void *routerThread(void *argv) {
    ThreadArg *arg;
    void *recvSocket, *signalSocket;
    Packet packet;
    int packetSize;
    
    arg = (ThreadArg *)argv;
    
    context = zmq_ctx_new();
    recvSocket = zmq_socket(context, ZMQ_REP);
    signalSocket = zmq_socket(context, ZMQ_PAIR);
    zmq_bind(recvSocket, arg->endpointRecv);
    zmq_bind(signalSocket, arg->endpointInterThread);
    
    zmq_pollitem_t pollitems[] = {
        { recvSocket, 0, ZMQ_POLLIN, 0},
        { signalSocket, 0, ZMQ_POLLIN, 0},
    };
    
    while (1) {
        zmq_poll(pollitems, 2, -1);
        if (pollitems[0].revents & ZMQ_POLLIN) {
            packetSize = zmq_recv(recvSocket, &packet, MAX_PACKET_SIZE, 0);
            if (packet.tag == TAG_OK) {
                zmq_send(recvSocket, &packet, 1, 0);
            } else if (packet.tag == TAG_STATUS) {
                Client *client = (Client *)packet.data;
                if (client->status == STATUS_ONLINE) {
                    setStatusUser(client->id, client->name, "ONLINE", client->ip, client->port);
                } else {
                    setStatusUser(client->id, client->name, "OFFLINE", client->ip, client->port);
                }
                packet.tag = TAG_OK;
                packet.dataSize = 0;
                zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                
            } else if (packet.tag == TAG_CLIENT) {
                Client *client = getInfoByID(packet.data);
                packet.tag=TAG_OK;
                if(strcmp(client->id,"-1")==0)
                    packet.dataSize=0;
                else
                    packet.dataSize=sizeof(Client);
                memcpy(packet.data, client,sizeof(Client));
                //printf("pk size: %d\n",PACKET_SIZE(packet));
                zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                
            } else if (packet.tag == TAG_LISTUSER) {
                int size = getSize("all");
                packet.tag = TAG_OK;
                packet.dataSize=sizeof(int);
                packet.data[0]=size;
                zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                //Client* x;
                //memcpy(packet.data, x, sizeof(Client));
                int i;
                Client** n_result = (Client **)packet.data;
                n_result = malloc(sizeof(n_result)*size);
                for(i=0;i<size;i++)
                    n_result[i] = malloc(sizeof(Client));
                getList(n_result);
                packet.tag = TAG_OK;
                packet.dataSize=sizeof(Client);
                for(i=0;i<size;i++){
                    memcpy(packet.data, n_result[i], sizeof(Client));
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                }
                
            } else if (packet.tag == TAG_USER) {
                char name[16];
                sprintf(name,"%s",packet.data);
                int size = getSize(packet.data);
                packet.tag = TAG_OK;
                packet.dataSize=sizeof(int);
                packet.data[0]=size;
                if(size==0){
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                    //continue;
                }
                else
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), ZMQ_SNDMORE);
                int i;
                Client** n_result;
                n_result = malloc(sizeof(n_result)*size);
                for(i=0;i<size;i++)
                    n_result[i] = malloc(sizeof(Client));
                getInfoByName(name,n_result);
                packet.tag = TAG_OK;
                packet.dataSize=sizeof(Client);
                for(i=0;i<size-1;i++){
                    memcpy(packet.data, n_result[i], sizeof(Client));
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), ZMQ_SNDMORE);
                }
                if(i==size-1){
                    memcpy(packet.data, n_result[i], sizeof(Client));
                    zmq_send(recvSocket, &packet, PACKET_SIZE(packet), 0);
                }
                
            } else {
                zmq_send(recvSocket, NULL, 0, 0);
            }
        }
        if (pollitems[1].revents & ZMQ_POLLIN) {
            packetSize = zmq_recv(signalSocket, &packet, MAX_PACKET_SIZE, 0);
            if (packet.tag == TAG_CLOSE) {
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

int startRouter(int port) {
    ///
    // start db.
    ///
    //createDatabase("./new.db");
    createDatabase("/Users/pimpat/Desktop/new.db");
    
    ///
    // start thead
    ///
    ThreadArg *arg = malloc(sizeof(ThreadArg));
    sprintf(arg->endpointRecv, "tcp://*:%d", port);
    strcpy(arg->endpointInterThread, ENDPOINT_INTER_THREAD);
    pthread_create(&Pthread, NULL, routerThread, arg);
    
    return 0;
}

int stopRouter(void) {
    ///
    // stop thread
    ///
    void *signalSocket;
    Packet packet;
    signalSocket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(signalSocket, ENDPOINT_INTER_THREAD);
    packet.tag = TAG_CLOSE;
    zmq_send(signalSocket, &packet, 1, 0);
    zmq_recv(signalSocket, NULL, 0, 0);
    zmq_close(signalSocket);
    zmq_ctx_destroy(context);
    
    if (pthread_join(Pthread, NULL)) {
        fprintf(stderr, "Error: join thread error\n");
        return 1;
    }
    
    ///
    // stop db.
    ///
    sqlite3_close(DB);
    int rc;
    if(rc != SQLITE_OK)
        fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(DB));
    else
        printf("Closed database successfully\n");
    return 0;
}

int statusToRouter(Client *client, char *endpointRouter) {
    int ret;
    void *socket;
    Packet packet;
    
	packet.tag = TAG_STATUS;
    packet.dataSize = sizeof(Client);
    memcpy(packet.data, client, packet.dataSize);
    
    socket = zmq_socket(Context, ZMQ_REQ);
    zmq_connect(socket, endpointRouter);
    zmq_send(socket, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(socket, &packet, PACKET_SIZE_MAX, 0);
    if (packet.tag == TAG_OK) {
        ret = 0;
    } else {
    	fprintf(stderr, "Error: regis return %d\n", packet.tag);
        ret = 1;
	}
    zmq_close(socket);
    return 0;
}

int startClient(char* id, char* name, char *ip, int port) {
    ///
    // start recv thread
    ///
    Context = zmq_ctx_new();
    startRecvData(Context, port);
    
    ///
    // online to router
    ///
    strcpy(myClient.id, id);
    strcpy(myClient.name, name);
    strcpy(myClient.ip, ip);
    myClient.port = port;
    myClient.status = STATUS_ONLINE;
    statusToRouter(&myClient, ENDPOINT_ROUTER);
    
    return 0;
}

int stopClient(void) {
    ///
    // offline to router
    ///
    myClient.status = STATUS_OFFLINE;
    statusToRouter(&myClient, ENDPOINT_ROUTER);
    
    ///
    // stop recv thread
    ///
    stopReceiveData(Context);
    zmq_ctx_destroy(Context);
    
    return 0;
}

int sendToClient(char *id, char* data, unsigned int dataSize) {
    void *socketToRouter, *socketToClient;
    Packet packet;
    char endpoint[32];
    
    ///
    // get client info from mydb
    ///
    
    ///
    // get client info from router
    ///
    socketToRouter = zmq_socket(Context, ZMQ_REQ);
    zmq_connect(socketToRouter, ENDPOINT_ROUTER);
    packet.tag = TAG_CLIENT;
    packet.dataSize = 16;
    memcpy(packet.data, id, 16);
    zmq_send(socketToRouter, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(socketToRouter, &packet, PACKET_SIZE_MAX, 0);
    if (packet.tag == TAG_OK) {
        Client *client = (Client *)packet.data;
        socketToClient = zmq_socket(Context, ZMQ_REQ);
        sprintf(endpoint, "tcp://%s:%u", client->ip, client->port);
        zmq_connect(socketToClient, endpoint);
        zmq_close(socketToRouter);
    } else {
        fprintf(stderr, "Error: Router return tag %d\n", packet.tag);
        zmq_close(socketToRouter);
        return 1;
    }
    
    ///
    // send
    ///
    packet.tag = TAG_MSG_CLIENT;
    memcpy(packet.data, myClient.id, 16);
    memcpy(packet.data+16, data, dataSize);
    packet.dataSize = 16+dataSize;
    zmq_send(socketToClient, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(socketToClient, NULL, 0, 0);
    zmq_close(socketToClient);
    
    return 0;
}

int sendToUser(char* user, char* data, unsigned int dataSize) {
    int i, numUser;
    void *socketToRouter, *socketToClient;
    Packet packet;
    char endpoint[32];
    
    ///
    // get user info from mydb
    ///
    
    ///
    // get user info from router
    ///
    socketToRouter = zmq_socket(Context, ZMQ_REQ);
    zmq_connect(socketToRouter, ENDPOINT_ROUTER);
    packet.tag = TAG_USER;
    packet.dataSize = 16;
    memcpy(packet.data, user, 16);
    zmq_send(socketToRouter, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(socketToRouter, &packet, PACKET_SIZE_MAX, 0);
    if (packet.tag == TAG_OK) {
        Client *client;
        numUser = packet.data[0];
        socketToClient = zmq_socket(Context, ZMQ_REQ);
        for (i=0; i<numUser; i++) {
            zmq_recv(socketToRouter, &packet, PACKET_SIZE_MAX, 0);
            client = (Client *)packet.data;
            sprintf(endpoint, "tcp://%s:%u", client->ip, client->port);
            zmq_connect(socketToClient, endpoint);
        }
        zmq_close(socketToRouter);
    } else {
        fprintf(stderr, "Error: Router return tag %d\n", packet.tag);
        zmq_close(socketToRouter);
        return 1;
    }
    
    ///
    // send
    ///
    memcpy(packet.data, myClient.id, 16);
    memcpy(packet.data+16, data, dataSize);
    packet.dataSize = 16+dataSize;
    for (i=0; i<numUser; i++) {
        packet.tag = TAG_MSG_USER;
        zmq_send(socketToClient, &packet, PACKET_SIZE(packet), 0);
        zmq_recv(socketToClient, &packet, 1, 0);
    }
    
    zmq_close(socketToClient);
    
    return 0;
}

Message* readBufferRecv() {
    ///
    // get data in buffer
    ///
    void *socket;
    Packet packet;
    Message *message;
    socket = zmq_socket(Context, ZMQ_PAIR);
    zmq_connect(socket, ENDPOINT_INTER_THREAD);
    packet.tag = TAG_MSG_CLIENT;
    packet.dataSize = 0;
    zmq_send(socket, &packet, PACKET_SIZE(packet), 0);
    zmq_recv(socket, &packet, PACKET_SIZE_MAX, 0);
    if (packet.tag == TAG_OK) {
        message = malloc(sizeof(Message));
        memcpy(message->from, packet.data, 16);
        message->size = packet.dataSize-16;
        message->msg = malloc(sizeof(char)*message->size);
        memcpy(message->msg, packet.data+16, message->size);
    } else {
        message = NULL;
    }
    zmq_close(socket);
    
    return message;
}

void freeMessage(Message* message) {
    free(message->msg);
    free(message);
}