#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <sqlite3.h>
#include <pthread.h>
#include <string.h>
#include "transport.h"
#include "funcdb.h"

#define ENDPOINT_INTER_THREAD "inproc://routerSignal"

#define MAX_PACKET_SIZE 264
#define MAX_ID_LENGTH 16
#define MAX_NAME_LENGTH MAX_ID_LENGTH
/*
enum Tag {
    TAG_OK,
    TAG_CLOSE,
    TAG_STATUS,
    TAG_LISTUSER,
};

enum Status { STATUS_ONLINE, STATUS_OFFLINE };

typedef struct Packet Packet;
struct Packet {
    char tag;
    unsigned int dataSize;
    char data[256];
};
*/
typedef struct ThreadArg ThreadArg;
struct ThreadArg {
    char endpointRecv[32];
    char endpointInterThread[32];
};
/*
typedef struct Client Client;
struct Client {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char status[10];
    char ip[16];
    int port;
};
*/
void *context;
pthread_t Pthread;
sqlite3 *DB;
/*
int setStatusUser(char* id, char* name, char* status, char* ip, int port);
int registerUser(char* id, char* name, char* status, char* ip, int port);
void createDatabase(char* path);
void onlineStatus(char* id);
void offlineStatus(char* id);
Client* getInfoByID(char* id);
int getInfoByName(char* name, Client** result);
int getSize(char* type);
int getList(Client** list);
*/
void testPacket(Packet* packet);

void testPacket(Packet* packet){
    if (packet->tag == TAG_STATUS) {
        Client *client = (Client *)packet->data;
        if (client->status == STATUS_ONLINE)
            setStatusUser(client->id, client->name, "ONLINE", client->ip, client->port);
        else
            setStatusUser(client->id, client->name, "OFFLINE", client->ip, client->port);
        packet->tag = TAG_OK;
        packet->dataSize=0;
    } else if (packet->tag == TAG_CLIENT) {
        Client *client = getInfoByID(packet->data);
        packet->tag=TAG_OK;
        if(strcmp(client->id,"-1")==0)
            packet->dataSize=0;
        else
            packet->dataSize=sizeof(Client);
        memcpy(packet->data, client,sizeof(Client));
    } else if (packet->tag == TAG_LISTUSER) {
        int size = getSize("all");          
        packet->tag = TAG_OK;
        packet->dataSize=sizeof(int);
        packet->data[0]=size;
        int i;
        Client** n_result;
        n_result = malloc(sizeof(n_result)*size);
        for(i=0;i<size;i++)
            n_result[i] = malloc(sizeof(Client));
        getList(n_result);
        packet->tag = TAG_OK;
        packet->dataSize=sizeof(Client);
        for(i=0;i<size;i++){
            memcpy(packet->data, n_result[i], sizeof(Client));
        }
    } else if (packet->tag == TAG_USER) {
        char name[16];
        sprintf(name,"%s",packet->data);
        int size = getSize(packet->data);
        packet->tag = TAG_OK;
        packet->dataSize=sizeof(int);
        packet->data[0]=size;
        int i;
        Client** n_result;
        n_result = malloc(sizeof(n_result)*size);
        for(i=0;i<size;i++)
            n_result[i] = malloc(sizeof(Client));
        getInfoByName(name,n_result);
        packet->tag = TAG_OK;
        packet->dataSize=sizeof(Client);
        for(i=0;i<size;i++){
            memcpy(packet->data, n_result[i], sizeof(Client));
        }
    } else printf("tag is not found\n");
}

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
