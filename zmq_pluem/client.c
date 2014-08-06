//
//  client.c
//  Client
//
//  Created by Kasidej Bungbrakearti on 7/7/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include <zmq.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>

#include "header.h"

#define PORT_RECIEVE 7672
#define ENDPOINT_ROUTER "tcp://localhost:7671"

uint16_t port_recieve;

uint32_t getMyIP() {
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) {
		fprintf(stderr, "Error: getMyIP: getifaddrs\n");
        return RET_ERR;
	}
	for (ifa = ifaddr; ifa!=NULL; ifa=ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) continue;
		if (ifa->ifa_addr->sa_family != AF_INET) continue;
		if (!strcmp(ifa->ifa_name,"lo0")) continue;
        return *(uint32_t*)(ifa->ifa_addr->sa_data+2);
	}
    fprintf(stderr, "Error: getMyIP: no IP\n");
	return RET_ERR;
}

int getId(char *id) {
    printf("ID:");
    scanf("%s", id);
    return RET_OK;
}
int getTarget(char *target) {
    printf("Target:");
    scanf("%s", target);
    if (!strcmp(target, ":q!"))
        return RET_EXIT;
    return RET_OK;
}
int send_data(char *buffer) {
    printf("say:");
    scanf("%s", buffer);
	buffer[strlen(buffer)-1] = "\n";
    if (!strcmp(buffer, ":q!"))
        return RET_EXIT;
    return RET_OK;
}

int onlineToRouter(void *context) {
    PacketR online_packet;
    int ret=RET_OK, packet_size;
	char buffer[256];
    
	online_packet.tag = TAG_STAT_ON;
    getId(online_packet.id);
    online_packet.ip = getMyIP();
    if (online_packet.ip == RET_ERR)
        return RET_ERR;
    online_packet.port = port_recieve;
    
    void *wRouter = zmq_socket(context, ZMQ_REQ);
    zmq_connect(wRouter, ENDPOINT_ROUTER);
    zmq_send(wRouter, &online_packet, sizeof(online_packet), 0);
    packet_size = zmq_recv(wRouter, buffer, 1, 0);
    if (packet_size!=1 && buffer[0]!=0) {
		fprintf(stderr, "Error: regis return %d\n", buffer[0]);
        ret = RET_ERR;
	}

    zmq_close(wRouter);
    return ret;
}
int getTargetbyIdFromRouter(char *endpoint_target, char *target, void *context) {
	PacketR getIp_packet;
	int ret=RET_OK, packet_size;
	
	getIp_packet.tag = TAG_GETIP;
    ret = getTarget(getIp_packet.id);
    if (ret == RET_EXIT)
        return RET_EXIT;

	void *wRouter = zmq_socket(context, ZMQ_REQ);
	zmq_connect(wRouter, ENDPOINT_ROUTER);
	zmq_send(wRouter, &getIp_packet, ID_LENGTH+1, 0);
	packet_size = zmq_recv(wRouter, &getIp_packet, PACKETR_SIZE, 0);
	if (getIp_packet.tag!=TAG_GETIP || packet_size!=PACKETR_SIZE) {
        fprintf(stderr, "Error: getTargetbyIdFromRouter\n");
        ret = RET_ERR;
    }
    uint8_t *ip = (uint8_t *)&getIp_packet.ip;
    sprintf(endpoint_target, "tcp://%u.%u.%u.%u:%hu", ip[0], ip[1], ip[2], ip[3], getIp_packet.port);

	zmq_close(wRouter);
	return ret;
}

static void *recieve_data(void *context) {
	void *receiver = zmq_socket(context, ZMQ_PAIR);
    char endpoint_recieve_data[32];
    sprintf(endpoint_recieve_data, "tcp://*:%hu", port_recieve);
	zmq_bind(receiver, endpoint_recieve_data);

	char buffer[256];
	int size;
	while(1) {
		size = zmq_recv(receiver, buffer, 256, 0);
        if (size==-1)
            break;
		buffer[size] = '\0';
		printf("rec: %s\n", buffer);
		
        // Do Something
	}
	zmq_close(receiver);
	return NULL;
}

int main(int argc, const char * argv[])
{
    int ret;
    char target[ID_LENGTH];
	pthread_t rec_data;
    void *context = zmq_ctx_new();
    
    if (argc>1)
        port_recieve = (uint16_t)argv[1];
    else
        port_recieve = PORT_RECIEVE;

	pthread_create(&rec_data, NULL, recieve_data, context);
    
    ret = onlineToRouter(context);
    
	char endpoint_target[32];
	char buffer[256];
	while(1) {
		ret = getTargetbyIdFromRouter(endpoint_target, target, context);
        if (ret==RET_EXIT)
            break;
		else if (ret!=RET_OK)
			continue;
		void *sender = zmq_socket(context, ZMQ_PAIR);
		ret = zmq_connect(sender, endpoint_target);
        if (ret == -1) {
            continue;
        }
		while (1) {
            ret = send_data(buffer);
            if (ret == RET_EXIT)
                break;
			zmq_send(sender, buffer, strlen(buffer), 0);
		}
		zmq_close(sender);
	}
    
    zmq_ctx_destroy(context);
    
//    gets(0);
    
    return 0;
}

