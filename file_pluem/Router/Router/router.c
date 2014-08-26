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

#include "layerdb.h"
#include "../../define.h"
#include "../../mystruct.h"
#include "../../Device.h"
#include "../../User.h"
#include "../../Group.h"

#define PORT_RECIEVE "7671"
#define ENDPOINT_RECIEVE(port) "tcp://*:"port

#define CHK_FIREWALL_TIMEOUT 1000 // ms

//User *listUser[MAX_LISTUSER];
//int lenUser=0;
Group *listGroup[MAX_LISTGROUP];
int lenGroup=0;

void *context;

int chkFirewall(uint32_t ip, uint16_t port) {
    int ret;
    void *socket;
    int timeout;
    char endpoint_target[32];
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_REQ);
    timeout = CHK_FIREWALL_TIMEOUT;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof timeout);
    zmq_setsockopt(socket, ZMQ_IDENTITY, "CHKFIREWALL", 11);
    ENDPOINT_IPPORT(endpoint_target, ip, port);
    zmq_connect(socket, endpoint_target);
    
    zmq_send(socket, "CHKFILEWALL", 11, ZMQ_SNDMORE);
    packet.tag = TAG_OK;
    zmq_send(socket, &packet, 1, 0);
    ret = zmq_recv(socket, &packet, PACKETDATA_SIZE(packet), 0);
    
    if (ret == 1 && packet.tag == TAG_OK)
        ret = RET_OK;
    else
        ret = RET_FIREWALL;
    
    zmq_close(socket);
    
    return ret;
}

Device* findFwder(void* argv) {
    return getDeviceByIdFromList("FWD");
}
int createFwder(PacketR *ret_packet) {
    int ret;
    void *socket;
    Device *fwder;
    char endpoint_target[32];
    PacketData packet;
    
    fwder = findFwder(NULL);
    if (fwder == NULL) {
        fprintf(stderr, "Error: can't find fwder\n");
        return RET_ERR;
    }
    
    socket = zmq_socket(context, ZMQ_REQ);
    zmq_setsockopt(socket, ZMQ_IDENTITY, "FWD", 3);
    ENDPOINT_IPPORT(endpoint_target, fwder->ip, fwder->port);
    zmq_connect(socket, endpoint_target);
    
    zmq_send(socket, "FWD", 3, ZMQ_SNDMORE);
    packet.tag = TAG_FWD;
    zmq_send(socket, &packet, 1, 0);
    ret = zmq_recv(socket, &packet, PACKETDATA_FULLSIZE, 0);
    if (packet.tag == TAG_OK || packet.data_length == 2) {
        ret_packet->ip = fwder->ip;
        ret_packet->port = *(uint16_t *)packet.data;
        ret = RET_OK;
    } else {
        fprintf(stderr, "Error: fwder ack %d\n", packet.tag);
        ret = RET_ERR;
    }
    
    zmq_close(socket);
    
    return ret;
}

int setStatusUserDevice(PacketR *packet) {
    int ret, i;
    User *user;
    Device *device;
    
    user = getUserByNameFromList(packet->username);
    if (user == NULL) {
        user = newUser(packet->username);
        if (user == NULL) {
            fprintf(stderr, "Error: in newUser\n");
            return RET_ERR;
        }
        ret = addUserToList(user);
        if (ret != RET_OK) {
            fprintf(stderr, "Error: in addUserToList\n");
            return RET_ERR;
        }
    }
    
    device = getDeviceByIdFromList(packet->id);
    if (device == NULL) {
        device = newDevice(packet->id);
        if (device == NULL) {
            fprintf(stderr, "Error: in newDevice\n");
            return RET_ERR;
        }
        ret = addDeviceToList(device);
        if (ret != RET_OK) {
            fprintf(stderr, "Error: in addDeviceToList\n");
            return RET_ERR;
        }
    }
    ret = RET_OK;
    if (packet->tag == TAG_STAT_ON) {
        ret = chkFirewall(packet->ip, packet->port);
        ret = !strcmp(packet->id, "03")? RET_FIREWALL:ret;
        if (ret==RET_FIREWALL) {
            createFwder(packet);
        }
    }
    device->func->setStat(device, packet->tag);
    device->func->setIP(device, packet->ip);
    device->func->setPort(device, packet->port);
    
    i = user->func->getIndexById(user, packet->id);
    if (i == -1) {
        user->func->AddDevice(user, device);
    } else if (device!=user->list_device[i]) {
        user->func->RemoveAtDevice(user, i);
        user->func->AddDevice(user, device);
    }
    
    return ret;
}

int setStatusGroup(PacketR *packet) {
    int ret, i;
    Group *group;
    Device *device;
    
    group = getGroupByNameFromList(listGroup, lenGroup, packet->username);
    if (group == NULL) {
        if (lenGroup<MAX_LISTGROUP) {
            group = newGroup(packet->username);
            if (group == NULL) {
                fprintf(stderr, "Error: in newGroup\n");
                return RET_ERR;
            }
            listGroup[lenGroup++] = group;
        } else {
            fprintf(stderr, "Error: MAX_LISTGROUP\n");
            return RET_ERR;
        }
    }
    
    device = getDeviceByIdFromList(packet->id);
    if (device == NULL) {
        device = newDevice(packet->id);
        if (device == NULL) {
            fprintf(stderr, "Error: in newDevice\n");
            return RET_ERR;
        }
        ret = addDeviceToList(device);
        if (ret != RET_OK) {
            fprintf(stderr, "Error: in addDeviceToList\n");
            return RET_ERR;
        }
    }
    device->func->setStat(device, packet->tag);
    device->func->setIP(device, packet->ip);
    device->func->setPort(device, packet->port);
    
    i = group->func->getIndexById(group, packet->id);
    if (i == -1) {
        group->func->AddDevice(group, device);
    } else if (device != group->list_device[i]) {
        group->func->RemoveAtDevice(group, i);
        group->func->AddDevice(group, device);
    }
    
    return RET_OK;
}

void init() {
    initListDevice();
    initListUser();
}
int main(int argc, const char * argv[])
{
    int ret, i;
    void *socket;
    PacketR packet;
    int packet_size;
    Device *device;
    User *user;
    Group *group;
    
//    myInitDB("~/Documents/newTran/Router/Router/statTable.db");
    ret = myInitDB("/Users/KasidejB/Documents/newTran/Router/Router/statTable.db");
    if (ret == 0) {
        printf("Connected database\n");
    } else {
        return 1;
    }
    
    init();
    
    context = zmq_ctx_new();
    socket = zmq_socket(context, ZMQ_REP);
    ret = zmq_bind(socket, ENDPOINT_RECIEVE(PORT_RECIEVE));
    if (ret == 0) {
        printf("Bind socket: port %s\n", PORT_RECIEVE);
    } else {
        fprintf(stderr, "Error: can't bind socket: port %s\n", PORT_RECIEVE);
        zmq_close(socket);
        zmq_ctx_destroy(context);
        myCloseDB();
        return 1;
    }

    zmq_pollitem_t items[] = {
        { socket, 0, ZMQ_POLLIN, 0 },
    };

    while (1) {
        zmq_poll(items, 1, -1);
        if (items[0].revents & ZMQ_POLLIN) {
            packet_size = zmq_recv(socket, &packet, PACKETR_SIZE, 0);
            switch (packet.tag) {
                case TAG_STAT_OFF:
                case TAG_STAT_ON:
                    if (packet_size != PACKETR_SIZE)
                        break;
                    ret = setStatusUserDevice(&packet);
                    if (ret == RET_OK) {
                        printf("Packet in: Status %s: from %u:%hu\n", packet.tag==TAG_STAT_ON? "On": "Off", packet.ip, packet.port);
                        packet.tag = TAG_OK;
                        zmq_send(socket, &packet, 1, 0);
                    } else if (ret == RET_FIREWALL) {
                        printf("Packet in: Status %s: firewall %u:%hu\n", packet.tag==TAG_STAT_ON? "On": "Off", packet.ip, packet.port);
                        packet.tag = TAG_FIREWALL;
//                        packet.ip = packet.ip;
                        packet.port = packet.port+1; // backend = frontend + 1
                        zmq_send(socket, &packet, PACKETR_SIZE, 0);
                        ;
                    } else {
                        fprintf(stderr, "Error: in setDeviceList\n");
                        packet.tag = TAG_ERR;
                        zmq_send(socket, &packet, 1, 0);
                    }
                    break;
                case TAG_DEVICE:
                    printf("Request id: %s\n", packet.id);
                    device = getDeviceByIdFromList(packet.id);
                    if (device!=NULL && device->stat==TAG_STAT_ON) {
                        printf("  Reply %u:%hu\n", device->ip, device->port);
                        packet.tag = TAG_OK;
                        packet.ip = device->ip;
                        packet.port = device->port;
                        zmq_send(socket, &packet, PACKETR_SIZE, 0);
                    } else {
                        fprintf(stderr, "  Error: in getDeviceFromId\n");
                        packet.tag = TAG_ERR;
                        zmq_send(socket, &packet, 1, 0);
                    }
                    break;
                case TAG_USER:
                    printf("Request username: %s\n", packet.username);
                    user = getUserByNameFromList(packet.username);
                    if (user!=NULL) {
                        for (i=0; i<user->len_device; i++) {
                            device = user->list_device[i];
                            if (device->stat != TAG_STAT_ON) {
                                continue;
                            }
                            printf("  Reply id:%s, %u:%hu\n", device->id, device->ip, device->port);
                            packet.tag = TAG_OK;
                            packet.ip = device->ip;
                            packet.port = device->port;
                            strcpy(packet.id, device->id);
                            zmq_send(socket, &packet, PACKETR_SIZE, ZMQ_SNDMORE);
                        }
                        zmq_send(socket, NULL, 0, 0);
                    } else {
                        fprintf(stderr, "  Error: username is incorrect\n");
                        packet.tag = TAG_ERR;
                        zmq_send(socket, &packet, 1, 0);
                    }
                    break;
                case TAG_JOINGROUP:
                case TAG_LEAVEGROUP:
                    if (packet_size != PACKETR_SIZE)
                        break;
                    ret = setStatusGroup(&packet);
                    if (ret == RET_OK) {
                        printf("Packet in: %s group %s: from %u:%hu\n", packet.tag==TAG_JOINGROUP? "Join":"Leave", packet.username, packet.ip, packet.port);
                        packet.tag = TAG_OK;
                        zmq_send(socket, &packet, 1, 0);
                    } else {
                        fprintf(stderr, "Error: in setDeviceList\n");
                    }
                    break;
                case TAG_GROUP:
                    printf("Request groupname: %s\n", packet.username);
                    group = getGroupByNameFromList(listGroup, lenGroup, packet.username);
                    if (group != NULL) {
                        for (i=0; i<group->len_device; i++) {
                            device = group->list_device[i];
                            if (device->stat != TAG_JOINGROUP) {
                                continue;
                            }
                            printf("  Reply id:%s, %u:%hu\n", device->id, device->ip, device->port);
                            packet.tag = TAG_OK;
                            packet.ip    = device->ip;
                            packet.port = device->port;
                            strcpy(packet.id, device->id);
                            zmq_send(socket, &packet, PACKETR_SIZE, ZMQ_SNDMORE);
                        }
                        zmq_send(socket, NULL, 0, 0);
                    } else {
                        fprintf(stderr, "  Error: groupname is incorrect");
                        packet.tag = TAG_ERR;
                        zmq_send(socket, &packet, 1, 0);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    zmq_close(socket);
    zmq_ctx_destroy(context);
    
    myCloseDB();
    
    return 0;
}

