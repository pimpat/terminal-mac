//
//  router.c
//  Router
//
//  Created by Kasidej Bungbrakearti on 7/4/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include "header.h"

#define PORT_RECIEVE "7671"
#define ENDPOINT_RECIEVE(port) "tcp://*:"port

#define MAX_LISTSTATUS 16

Status *listStatus[MAX_LISTSTATUS];
int lenStatus=0;

int getIndexFromId(char* id) {
	int i;
	for (i=0;i<lenStatus;i++) {
		if (!strcmp(listStatus[i]->id, id))
			return i;
	}
	return -1;
}
int setStatus(PacketR *packet) {
	int index = getIndexFromId(packet->id);
	if (index<0) {
        if (lenStatus<MAX_LISTSTATUS) {
            lenStatus++;
            listStatus[lenStatus-1] = malloc(sizeof(Status_desc));
            if (listStatus[lenStatus-1] == NULL) {
                fprintf(stderr, "Error: listStatus[lenStatus-1]==NULL\n");
                return -1;
            }
            listStatus[lenStatus-1]->stat = packet->tag;
            strcpy(listStatus[lenStatus-1]->id, packet->id);
            listStatus[lenStatus-1]->ip = packet->ip;
            listStatus[lenStatus-1]->port = packet->port;
        } else {
            fprintf(stderr, "Error: MAX_LISTSTATUS\n");
            return -1;
        }
	} else {
		listStatus[index]->stat = packet->tag;
        listStatus[index]->ip = packet->ip;
        listStatus[index]->port = packet->port;
	}
	return 0;
}

int main(int argc, const char * argv[])
{
    void *context = zmq_ctx_new();
    void *status_sock = zmq_socket(context, ZMQ_REP);
    zmq_bind(status_sock, ENDPOINT_RECIEVE(PORT_RECIEVE));

    zmq_pollitem_t items[] = {
        { status_sock, 0, ZMQ_POLLIN, 0 },
    };

    PacketR packet;
    size_t packet_size;
    int index;
    while (1) {
        zmq_poll(items, 1, -1);
        if (items[0].revents & ZMQ_POLLIN) {
            packet_size = zmq_recv(status_sock, &packet, PACKETR_SIZE, 0);
            switch (packet.tag) {
                case TAG_STAT_OFF:
                case TAG_STAT_ON:
                    if (packet_size == PACKETR_SIZE) {
                        setStatus(&packet);
                        zmq_send(status_sock, "\0", 1, 0); //send OK
                    }
                    break;
                case TAG_GETIP:
                    index = getIndexFromId(packet.id);
                    if (index>=0) {
                        packet.ip = listStatus[index]->ip;
                        packet.port = listStatus[index]->port;
                        zmq_send(status_sock, &packet, PACKETR_SIZE, 0);
                    } else {
                        zmq_send(status_sock, "\255", 1, 0);
                    }
                default:
                    break;
            }
        }
    }
    
    zmq_close(status_sock);
    zmq_ctx_destroy(context);
    
    return 0;
}

