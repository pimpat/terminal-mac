#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <string.h>

#include <transport.h>
#include "recvthread.h"
#include "client.h"

#define ENDPOINT_ROUTER "tcp://localhost:5500"

void *Context;
Client myClient;

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