//
//  recv_data.c
//  Client
//
//  Created by Kasidej Bungbrakearti on 7/31/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

#include <stdlib.h>
#include <pthread.h>
#include <zmq.h>
#include <string.h>

#include "../../define.h"
#include "../../mystruct.h"
#include "../../User.h"
#include "../../Group.h"
#include "recv_data.h"
#include "forwarder.h"

typedef struct RecvDataArg_desc RecvDataArg;
struct RecvDataArg_desc {
    void *context;
    char recv_endpoint[32];
    char username[USERNAME_LENGTH];
    
} RecvDataArg_desc;

pthread_t recv_thread;

static void *receive_data_thread(void *argv) {
    int ret, i;
    void *recv_socket, *signal_socket;
    PacketData packet;
	int packet_size;
    char sender_id[ID_LENGTH];
    int sender_id_length;
    User *newDeviceInMyUser;
    Group *listNewDeviceInMyGroup[MAX_LISTGROUP];
    int lenMyGroup=0;
    Device *tmp_device;
//    User *tmp_user;
    Group *tmp_group;
    char *tmp;
    
    RecvDataArg *arg = (RecvDataArg *)argv;
    
    newDeviceInMyUser = newUser(arg->username);
    
    signal_socket = zmq_socket(arg->context, ZMQ_PAIR);
    zmq_bind(signal_socket, "inproc://recv_signal");
    
	recv_socket = zmq_socket(arg->context, ZMQ_REP);
	zmq_bind(recv_socket, arg->recv_endpoint);
    
    zmq_pollitem_t items[] = {
        { recv_socket, 0, ZMQ_POLLIN, 0},
        { signal_socket, 0, ZMQ_POLLIN, 0},
    };
    
	while(1) {
        zmq_poll(items, 2, -1);
        if (items[0].revents & ZMQ_POLLIN) {
            sender_id_length = zmq_recv(recv_socket, sender_id, ID_LENGTH, 0);
            packet_size = zmq_recv(recv_socket, &packet, PACKETDATA_FULLSIZE, 0);
            if (packet_size==-1)
                break;
            switch (packet.tag) {
                case TAG_OK:
                    packet.tag = TAG_OK;
                    zmq_send(recv_socket, &packet, 1, 0);
                    break;
                case TAG_DATA:
                    sender_id[sender_id_length] = '\0';
                    packet.data[packet.data_length] = '\0';
                    printf("%s: %s\n", sender_id, packet.data);
                    packet.tag = TAG_OK;
                    zmq_send(recv_socket, &packet, 1, 0);
                    break;
                case TAG_FWD:
                    ret = startForwarderThread((uint16_t *)packet.data, arg->context);
                    if (ret == RET_OK) {
                        packet.tag = TAG_OK;
                        packet.data_length = 2;
                    } else {
                        fprintf(stderr, "Error: in openFwdProxy\n");
                        packet.tag = TAG_ERR;
                        packet.data_length = 0;
                    }
                    zmq_send(recv_socket, &packet, PACKETDATA_SIZE(packet), 0);
                    break;
                case TAG_USER:
                    tmp_device = castDevice(packet.data);
                    newDeviceInMyUser->func->AddCopyDevice(newDeviceInMyUser, tmp_device);
                    packet.tag = TAG_OK;
                    packet.data_length = 0;
                    zmq_send(recv_socket, &packet, 1, 0);
                    break;
                case TAG_JOINGROUP:
                    tmp = (char *)packet.data;
                    tmp_group = getGroupByNameFromList(listNewDeviceInMyGroup, lenMyGroup, tmp);
                    if (tmp_group != NULL) {
                        tmp_device = castDevice(&packet.data[GROUPNAME_LENGTH]);
                        tmp_group->func->AddCopyDevice(tmp_group, tmp_device);
                    }
                    packet.tag = TAG_OK;
                    packet.data_length = 0;
                    zmq_send(recv_socket, &packet, 1, 0);
                    break;
                case TAG_LEAVEGROUP:
                    tmp = (char *)packet.data;
                    tmp_group = getGroupByNameFromList(listNewDeviceInMyGroup, lenMyGroup, tmp);
                    if (tmp_group != NULL) {
                        tmp_device = castDevice(&packet.data[GROUPNAME_LENGTH]);
                        tmp_device->stat = TAG_STAT_OFF;
                        tmp_group->func->AddCopyDevice(tmp_group, tmp_device);
                    }
                    packet.tag = TAG_OK;
                    packet.data_length = 0;
                    zmq_send(recv_socket, &packet, 1, 0);
                    break;
                default:
                    continue;
            }
        }
        if (items[1].revents & ZMQ_POLLIN) {
//            zmq_recv(signal_socket, NULL, 0, 0); // envelope
            packet_size = zmq_recv(signal_socket, &packet, PACKETDATA_FULLSIZE, 0);
            if (packet_size==-1) {
                fprintf(stderr, "Error: recv_signal = -1\n");
                break;
            }
            if (packet.tag == TAG_FWD) {
                ret = zmq_connect(recv_socket, (char *)packet.data);
                if (ret != 0) {
                    fprintf(stderr, "Error: connect to fwder: ret=%d\n", ret);
                }
            } else if (packet.tag == TAG_USER) {
                packet.tag = TAG_OK;
                packet.data_length = sizeof(Device_desc);
                for (i=0; i<newDeviceInMyUser->len_device; i++) {
                    memcpy(packet.data, newDeviceInMyUser->list_device[i],  packet.data_length);
                    zmq_send(signal_socket, &packet, PACKETDATA_SIZE(packet), ZMQ_SNDMORE);
                }
                zmq_send(signal_socket, &packet, 1, 0);
                newDeviceInMyUser->func->freeDevice(newDeviceInMyUser);
                newDeviceInMyUser->func->free(newDeviceInMyUser);
                newDeviceInMyUser = newUser("");
            } else if (packet.tag == TAG_GROUP) {
                tmp_group = getGroupByNameFromList(listNewDeviceInMyGroup, lenMyGroup, (char *)packet.data);
                packet.tag = TAG_OK;
                packet.data_length = sizeof(Device_desc);
                if (tmp_group != NULL) {
                    for (i=0; i<tmp_group->len_device; i++) {
                        memcpy(packet.data, tmp_group->list_device[i], packet.data_length);
                        zmq_send(signal_socket, &packet, PACKETDATA_SIZE(packet), ZMQ_SNDMORE);
                        tmp_group->list_device[i]->func->free(tmp_group->list_device[i]);
                    }
                    tmp_group->len_device = 0;
                }
                zmq_send(signal_socket, &packet, 1, 0);
            } else if (packet.tag == TAG_JOINGROUP) {
                listNewDeviceInMyGroup[lenMyGroup++] = newGroup((char *)packet.data);
                zmq_send(signal_socket, NULL, 0, 0);
            } else if (packet.tag == TAG_LEAVEGROUP) {
                removeGroupFromList(listNewDeviceInMyGroup, &lenMyGroup, (char *)packet.data);
                zmq_send(signal_socket, NULL, 0, 0);
            } else if (packet.tag == TAG_END) {
                printf("END Signal Receive Data!\n");
                break;
            }
        }
	}
    
	zmq_close(recv_socket);
    zmq_close(signal_socket);
    newDeviceInMyUser->func->freeDevice(newDeviceInMyUser);
    newDeviceInMyUser->func->free(newDeviceInMyUser);
    free(argv);
	return NULL;
}

int startRecvData(void *context, uint16_t myport, char *username) {
    RecvDataArg *recvdata_arg = malloc(sizeof(RecvDataArg_desc));
    recvdata_arg->context = context;
    sprintf(recvdata_arg->recv_endpoint, "tcp://*:%hu", myport);
    strcpy(recvdata_arg->username, username);
    pthread_create(&recv_thread, NULL, receive_data_thread, recvdata_arg);
    
    return RET_OK;
}

int stopReceiveData(void *context) {
    void *socket;
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    packet.tag = TAG_END;
    zmq_send(socket, &packet, 1, 0);
    zmq_close(socket);
    
    if(pthread_join(recv_thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return RET_ERR;
    }
    
    return RET_OK;
}