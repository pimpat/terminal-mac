//
//  client.c
//  Client
//
//  Created by Kasidej Bungbrakearti on 7/7/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>

#include "recv_data.h"
#include "forwarder.h"
#include "client.h"

User *myUser;
Device *myDevice;
Group *listMyGroup[MAX_LISTGROUP];
int lenGroup=0;
void *context;

void init() {
    initListDevice();
    initListUser();
}
int main(int argc, const char * argv[])
{
    int ret;
    char command[32];
    
    init();
    
    myUser = newUser("");
    if (myUser == NULL) {
        fprintf(stderr, "Error: in newUser\n");
        return RET_ERR;
    }
    myDevice = newDevice("");
    if (myDevice == NULL) {
        fprintf(stderr, "Error: in newDevice\n");
        return RET_ERR;
    }
    myDevice->func->setStat(myDevice, TAG_STAT_ON);
    
    ret = getMyIP();
    if (ret == RET_ERR) {
        fprintf(stderr, "Error: in getMyIP\n");
        return RET_ERR;
    }
    if (argc>1)
        myDevice->func->setPort(myDevice, atoi(argv[1]));
    else
        myDevice->func->setPort(myDevice, DEFAULT_PORT_RECIEVE);
    char str[32];
    ENDPOINT_IPPORT(str, myDevice->ip, myDevice->port);
    printf("myEndpoint = %s\n", str);
    getMyId(myDevice->id);
    
    getMyUsername(myUser->username);
    myUser->func->AddDevice(myUser, myDevice);
    
    addUserToList(myUser);
    addDeviceToList(myDevice);
    
    context = zmq_ctx_new();
    startRecvData(context, myDevice->port, myUser->username);
    
    ret = statusToRouter(TAG_STAT_ON);
    if (ret == RET_ERR) {
        fprintf(stderr, "Error: in onlineToRouter\n");
        return RET_ERR;
    }

    while (1) {
        getCommand(command);
        switch (command[0]) {
            case 's': // [send] send to user
                commandSend();
                break;
            case 'd': // [device] send to device
                commandSendDevice();
                break;
            case 'j': // [join] join group
                commandJoinGroup();
                break;
            case 'l': // [leave] leave group
                commandLeaveGroup();
                break;
            case 'g': // [group] send to group
                commandSendGroup();
                break;
            default:
                break;
        }
        if (strcmp(command, "q!") == 0) {
            break;
        }
    }
    
    ret = stopReceiveData(context);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in stopReceiveData\n");
    }
    ret = statusToRouter(TAG_STAT_OFF);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in statusToRouter\n");
    }
    
    zmq_ctx_destroy(context);
    
    ret = stopFwdProxy(context);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in stopFwdProxy\n");
    }
    
    myUser->func->freeDevice(myUser);
    myUser->func->free(myUser);
    
    getchar();
    
    return 0;
}

int getMyIP() {
    /*
    uint8_t ip32[4] = {192,168,179,93};
    myIp = *(uint32_t *)ip32;
    return RET_OK;
    */
    
    // For OSX
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) {
		fprintf(stderr, "Error: getifaddrs return -1\n");
        return RET_ERR;
	}
	for (ifa = ifaddr; ifa!=NULL; ifa=ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) continue;
		if (ifa->ifa_addr->sa_family != AF_INET) continue;
		if (strcmp(ifa->ifa_name,"lo0") == 0) continue;
        myDevice->func->setIP(myDevice, *(uint32_t*)(ifa->ifa_addr->sa_data+2));
        return RET_OK;
	}
    fprintf(stderr, "Error: no IP return\n");
	return RET_ERR;
}

void getMyUsername(char *myUsername) {
    printf("Username:");
    scanf("%s", myUsername);
}
void getMyId(char *myId) {
    printf("ID:");
    scanf("%s", myId);
}
void getCommand(char *command) {
    printf("Command:");
    scanf("%s", command);
}
void getTargetId(char *target_id) {
    scanf("%s", target_id);
}
void getTargetUsername(char *target_username) {
    scanf("%s", target_username);
}
void getTargetGroupname(char *target_groupname) {
    scanf("%s", target_groupname);
}
void getData(uint8_t *packet) {
    scanf("%s", packet);
}
int BuildDataToUser(PacketData *packet) {
    packet->tag = TAG_DATA;
    printf("%s:", myUser->username);
    getData(packet->data);
    if (strcmp((char *)packet->data, "q!") == 0)
        return RET_EXIT;
    packet->data_length = strlen((char*)packet->data);
    return RET_OK;
}
int BuildDataToDevice(PacketData *packet) {
    packet->tag = TAG_DATA;
    printf("%s:", myDevice->id);
    getData(packet->data);
    if (strcmp((char *)packet->data, "q!") == 0)
        return RET_EXIT;
    packet->data_length = strlen((char*)packet->data);
    return RET_OK;
}

int statusToRouter(uint8_t status) {
    int ret;
    void *router_socket;
    PacketR packet;
    
	packet.tag = status;
    packet.ip = myDevice->ip;
    packet.port = myDevice->port;
    strcpy(packet.id, myDevice->id);
    strcpy(packet.username, myUser->username);
    
    router_socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(router_socket, ENDPOINT_ROUTER);
    
    zmq_send(router_socket, &packet, PACKETR_SIZE, 0);
    zmq_recv(router_socket, &packet, PACKETR_SIZE, 0);
    if (packet.tag == TAG_OK) {
        ret = RET_OK;
    } else if (packet.tag == TAG_FIREWALL) {
        printf("In Firewall/NAT...\n");
        char *endpoint = malloc(sizeof(char)*32);
        ENDPOINT_IPPORT(endpoint, packet.ip, packet.port);
        printf("  Forwarder is %s\n", endpoint);
        signalToRecvDataForConnectFwder(context, endpoint);
        ret = RET_OK;
    } else {
    	fprintf(stderr, "Error: regis return %d\n", packet.tag);
        ret = RET_ERR;
	}
    
    zmq_close(router_socket);
    
    if (status == TAG_STAT_ON) {
        ret = getOtherDeviceInMyUser();
        if (ret != RET_OK) {
            fprintf(stderr, "Error: in getOtherDeviceInMyUser\n");
            return RET_ERR;
        }
        ret = alertOtherDeviceInMyUser();
        if (ret != RET_OK) {
            fprintf(stderr, "Error: in alertOtherDeviceInMyUser\n");
            return RET_ERR;
        }
    }
    
    return ret;
}

int getOtherDeviceInMyUser() {
    int i;
    User *user;
    Device *device;
    
    user = getUserInfoFromRouter(myUser->username);
    if (user == NULL) {
        fprintf(stderr, "Error: in getUserInfoFromRouter\n");
        return RET_ERR;
    }
    if (user->len_device < 2) {
        return RET_OK;
    }
    for (i=0; i<user->len_device; i++) {
        device = user->list_device[i];
        if (strcmp(device->id, myDevice->id) != 0) {
            myUser->func->AddDevice(myUser, device);
            addDeviceToList(device);
        } else {
            device->func->free(device);
        }
    }
//    user->func->freeDevice(user); // every device were used or freed
    user->func->free(user);
    return RET_OK;
}
int getOtherDeviceInMyGroup(Group *my_group) {
    int i;
    Group *group;
    Device *device;
    
    group = getGroupInfoFromRouter(my_group->groupname);
    if (group == NULL) {
        fprintf(stderr, "Error: in getGroupInfoFromRouter\n");
        return RET_ERR;
    }
    if (group->len_device < 2) {
        return RET_OK;
    }
    for (i=0; i<group->len_device; i++) {
        device = group->list_device[i];
        if (strcmp(device->id, myDevice->id) != 0) {
            my_group->func->AddCopyDevice(my_group, device);
        }
    }
    group->func->freeDevice(group);
    group->func->free(group);
    return RET_OK;
}
int alertOtherDeviceInMyUser() {
    int i;
    void *socket;
    Device *device;
    char endpoint[32];
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_REQ);
    for (i=1; i<myUser->len_device; i++) {
        device = myUser->list_device[i];
        ENDPOINT_IPPORT(endpoint, device->ip, device->port)
        zmq_connect(socket, endpoint);
    }
    packet.tag = TAG_USER;
    packet.data_length = sizeof(Device_desc);
    memcpy(packet.data, (void*)myDevice, packet.data_length);
    for (i=1; i<myUser->len_device; i++) {
        zmq_send(socket, NULL, 0, ZMQ_SNDMORE);
        zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
        zmq_recv(socket, NULL, 0, 0);
    }
    zmq_close(socket);
    
    return RET_OK;
}
int alertOtherDeviceInMyGroup(Group *my_group, int tag) {
    int i;
    void *socket;
    Device *device;
    char endpoint[32];
    PacketData packet;
    
    if (my_group->len_device < 2) {
        return RET_OK;
    }
    socket = zmq_socket(context, ZMQ_REQ);
    for (i=1; i<my_group->len_device; i++) {
        device = my_group->list_device[i];
        ENDPOINT_IPPORT(endpoint, device->ip, device->port)
        zmq_connect(socket, endpoint);
    }
    packet.tag = TAG_JOINGROUP;
    memcpy(packet.data, my_group->groupname, GROUPNAME_LENGTH);
    memcpy(&packet.data[GROUPNAME_LENGTH], (void*)myDevice, sizeof(Device_desc));
    packet.data_length = GROUPNAME_LENGTH + sizeof(Device_desc);
    for (i=1; i<my_group->len_device; i++) {
        zmq_send(socket, NULL, 0, ZMQ_SNDMORE);
        zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
        zmq_recv(socket, NULL, 0, 0);
    }
    zmq_close(socket);
    
    return RET_OK;
}

// send to User
int commandSend() {
    int ret, i;
    void *sender;
    char target_username[USERNAME_LENGTH];
    User *target_user;
    PacketData packet;
    
    getTargetUsername(target_username);
    if (strcmp(target_username, myUser->username) != 0) {
        target_user = getUserInfoFromRouter(target_username);
    } else {
        target_user = newUser(target_username);
    }
    if (target_user == NULL) {
        fprintf(stderr, "Error: in getUserInfoFromRouter\n");
        return RET_ERR;
    }
    
    sender = zmq_socket(context, ZMQ_REQ);
    zmq_setsockopt(sender, ZMQ_IDENTITY, myDevice->id, strlen(myDevice->id));
    
    ret = connectToUser(sender, target_user);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in connectToUser\n");
        zmq_close(sender);
        return RET_ERR;
    }
    
    while (1) {
        ret = BuildDataToUser(&packet);
        if (ret == RET_EXIT)
            break;
        for (i=0; i<target_user->len_device; i++) {
            ret = zmq_send(sender, myUser->username, USERNAME_LENGTH, ZMQ_SNDMORE);
            ret = zmq_send(sender, &packet, PACKETDATA_SIZE(packet), 0);
            if (ret == -1) {
                fprintf(stderr, "Error: zmq_send return -1\n");
                break;
            }
            zmq_recv(sender, NULL, 0, 0);
        }
    }
    ret = disconnectUser(sender, target_user);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in disconnectUser\n");
        ret = RET_ERR;
    }
    
    zmq_close(sender);
    target_user->func->freeDevice(target_user);
    target_user->func->free(target_user);
    
    return RET_OK;
}

// send to Device
int commandSendDevice() {
    int ret;
    void *sender;
    char target_id[ID_LENGTH];
    Device *target_device;
    PacketData packet;
    
    getTargetId(target_id);
    target_device = getDeviceInfoFromRouter(target_id);
    if (target_device == NULL) {
        fprintf(stderr, "Error: in getDeviceInfoFromRouter\n");
        return RET_ERR;
    }
    
    sender = zmq_socket(context, ZMQ_REQ);
    zmq_setsockopt(sender, ZMQ_IDENTITY, myDevice->id, strlen(myDevice->id));
    
    ret = connectToDevice(sender, target_device);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in connectToDevice\n");
        zmq_close(sender);
        return RET_ERR;
    }
    while (1) {
        ret = BuildDataToDevice(&packet);
        if (ret == RET_EXIT)
            break;
        ret = zmq_send(sender, myDevice->id, ID_LENGTH, ZMQ_SNDMORE);
        ret = zmq_send(sender, &packet, PACKETDATA_SIZE(packet), 0);
        if (ret == -1) {
            fprintf(stderr, "Error: zmq_send return -1\n");
            break;
        }
        zmq_recv(sender, NULL, 0, 0);
    }
    ret = disconnectDevice(sender, target_device);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in disconnectDevice\n");
        zmq_close(sender);
        return RET_ERR;
    }
    
    zmq_close(sender);
    
    return RET_OK;
}

int commandJoinGroup() {
    int ret;
    Group *group;
    void *socket;
    char target_groupname[GROUPNAME_LENGTH];
    PacketR packet;
    
    getTargetGroupname(target_groupname);
    group = getGroupByNameFromList(listMyGroup, lenGroup, target_groupname);
    if (group != NULL) {
        fprintf(stderr, "Error: joined\n");
        return RET_ERR;
    }
    group = newGroup(target_groupname);
    if (group == NULL) {
        fprintf(stderr, "Error: in newGroup\n");
        return RET_ERR;
    }
    group->func->AddCopyDevice(group, myDevice);
    listMyGroup[lenGroup++] = group;
    
    packet.tag = TAG_JOINGROUP;
    packet.ip = myDevice->ip;
    packet.port = myDevice->port;
    strcpy(packet.id, myDevice->id);
    strcpy(packet.username, target_groupname);
    
    socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, ENDPOINT_ROUTER);
    zmq_send(socket, &packet, PACKETR_SIZE, 0);
    zmq_recv(socket, &packet, 1, 0);
    if (packet.tag == TAG_OK) {
        printf("Join to %s success.\n", packet.username);
        ret = RET_OK;
    } else {
    	fprintf(stderr, "Error: join group return %d\n", packet.tag);
        ret = RET_ERR;
	}
    zmq_close(socket);
    
    ret = getOtherDeviceInMyGroup(group);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in getOtherDeviceInMyGroup\n");
        return RET_ERR;
    }
    ret = alertOtherDeviceInMyGroup(group, TAG_JOINGROUP);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in alertOtherDeviceInMyGroup\n");
        return RET_ERR;
    }
    ret = signalToRecvDataForNewGroup(target_groupname);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in signalRecvDataToNewGroup\n");
        return RET_ERR;
    }
    
    return ret;
}
int commandLeaveGroup() {
    int ret;
    Group *group;
    void *router_socket;
    char target_groupname[GROUPNAME_LENGTH];
    PacketR packet;
    
    getTargetGroupname(target_groupname);
    group = getGroupByNameFromList(listMyGroup, lenGroup, target_groupname);
    if (group == NULL) {
        fprintf(stderr, "Error: leaved\n");
        return RET_ERR;
    }
    
    packet.tag = TAG_LEAVEGROUP;
    packet.ip = myDevice->ip;
    packet.port = myDevice->port;
    strcpy(packet.id, myDevice->id);
    strcpy(packet.username, target_groupname);
    
    router_socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(router_socket, ENDPOINT_ROUTER);
    zmq_send(router_socket, &packet, PACKETR_SIZE, 0);
    zmq_recv(router_socket, &packet, 1, 0);
    if (packet.tag == TAG_OK) {
        printf("Leave %s success.\n", packet.username);
        ret = RET_OK;
    } else {
    	fprintf(stderr, "Error: leave group return %d\n", packet.tag);
        ret = RET_ERR;
	}
    zmq_close(router_socket);
    
    ret = alertOtherDeviceInMyGroup(group, TAG_LEAVEGROUP);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in alertOtherDeviceInMyGroup\n");
        return RET_ERR;
    }
    ret = signalToRecvDataForRemoveGroup(target_groupname);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in signalRecvDataToRemoveGroup\n");
        return RET_ERR;
    }
    
    removeGroupFromList(listMyGroup, &lenGroup, target_groupname);
    
    return ret;
}
int commandSendGroup() {
    int ret, i;
    void *sender;
    char target_groupname[GROUPNAME_LENGTH];
    Group *target_group;
    PacketData packet;
    
    getTargetGroupname(target_groupname);
    target_group = getGroupByNameFromList(listMyGroup, lenGroup, target_groupname);
    if (target_group == NULL) {
        fprintf(stderr, "Error: leaved\n");
        return RET_ERR;
    }
    
    sender = zmq_socket(context, ZMQ_REQ);
    zmq_setsockopt(sender, ZMQ_IDENTITY, myDevice->id, strlen(myDevice->id));
    
    ret = connectToGroup(sender, target_group);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in connectToGroup\n");
        zmq_close(sender);
        return RET_ERR;
    }
    
    while (1) {
        ret = BuildDataToDevice(&packet);
        if (ret == RET_EXIT)
            break;
        for (i=1; i<target_group->len_device; i++) {
            ret = zmq_send(sender, myDevice->id, ID_LENGTH, ZMQ_SNDMORE);
            ret = zmq_send(sender, &packet, PACKETDATA_SIZE(packet), 0);
            if (ret == -1) {
                fprintf(stderr, "Error: zmq_send return -1\n");
                break;
            }
            zmq_recv(sender, NULL, 0, 0);
        }
    }
    ret = disconnectGroup(sender, target_group);
    if (ret != RET_OK) {
        fprintf(stderr, "Error: in disconnectGroup\n");
        ret = RET_ERR;
    }
    zmq_close(sender);
    
    return ret;;
}


Device* getDeviceInfoFromRouter(char *target_id) {
    int ret;
    void *socket;
    Device *device;
    PacketR packet;
    
    packet.tag = TAG_DEVICE;
    strcpy(packet.id, target_id);
    
    socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, ENDPOINT_ROUTER);
    zmq_send(socket, &packet, PACKETR_SIZE, 0);
    ret = zmq_recv(socket, &packet, PACKETR_SIZE, 0);
    if (packet.tag == TAG_OK) {
        device = newDevice2(target_id, TAG_STAT_ON, packet.ip, packet.port);
        if (device == NULL) {
            fprintf(stderr, "Error: in newDevice2\n");
        }
    } else {
        fprintf(stderr, "Error: Router doesn't return device\n");
        device = NULL;
    }
    
    zmq_close(socket);
    
    return device;
}
User* getUserInfoFromRouter(char *target_username) {
    int ret;
    void *socket;
    PacketR packet;
    User *user;
    Device *device;
    size_t size = 4;
    
    packet.tag = TAG_USER;
    strcpy(packet.username, target_username);
    
    socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, ENDPOINT_ROUTER);
    zmq_send(socket, &packet, PACKETR_SIZE, 0);
    
    ret = zmq_recv(socket, &packet, PACKETR_SIZE, 0);
    if (packet.tag == TAG_OK) {
        user = newUser(target_username);
        if (user != NULL) {
            do {
                device = newDevice2(packet.id, TAG_STAT_ON, packet.ip, packet.port);
                if (device != NULL) {
                    user->func->AddDevice(user, device);
                } else {
                    fprintf(stderr, "Error: in newDevice2\n");
                }
                zmq_recv(socket, &packet, PACKETR_SIZE, 0);
                zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
            } while (ret == 1);
        } else {
            fprintf(stderr, "Error: in newUser\n");
            ret = RET_ERR;
        }
    } else {
        fprintf(stderr, "Error: Router doesn't return user\n");
        user = NULL;
    }
    
    zmq_close(socket);
    
    return user;
}
Group* getGroupInfoFromRouter(char *target_groupname) {
    int ret;
    void *socket;
    PacketR packet;
    Group *group;
    Device *device;
    size_t size = 4;
    
    packet. tag = TAG_GROUP;
    strcpy(packet.username, target_groupname);
    
    socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, ENDPOINT_ROUTER);
    zmq_send(socket, &packet, PACKETR_SIZE, 0);
    
    ret = zmq_recv(socket, &packet, PACKETR_SIZE, 0);
    if (packet.tag == TAG_OK) {
        group = newGroup(target_groupname);
        if (group != NULL) {
            do {
                device = newDevice2(packet.id, TAG_STAT_ON, packet.ip, packet.port);
                if (device != NULL) {
                    group->func->AddDevice(group, device);
                } else {
                    fprintf(stderr, "Error: in newDevice2\n");
                }
                zmq_recv(socket, &packet, PACKETR_SIZE, 0);
                zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
            } while (ret == 1);
        } else {
            fprintf(stderr, "Error: in newGroup\n");
            ret = RET_ERR;
        }
    } else {
        fprintf(stderr, "Error: Router doesn't return group\n");
    }
    zmq_close(socket);
    
    return group;
}

int connectToDevice(void *socket, Device *device) {
    int ret;
    char endpoint_target[32];
    
    ENDPOINT_IPPORT(endpoint_target, device->ip, device->port);
    ret = zmq_connect(socket, endpoint_target);
    if (ret == -1) {
        fprintf(stderr, "Error: connect to %s return -1\n", device->id);
        return RET_ERR;
    }

    return RET_OK;
}
int connectToUser(void *socket, User *user) {
    int ret, i;
    PacketData packet;
    
    // add other device in myUser to user
    fetchNewDeviceInMyUserFromRecvDataThread();
    for (i=1; i<myUser->len_device; i++) {
        user->func->AddCopyDevice(user, myUser->list_device[i]);
    }
    
    for (i=0; i<user->len_device; i++) {
        ret = connectToDevice(socket, user->list_device[i]);
        if (ret == RET_ERR) {
            fprintf(stderr, "Error: in connectToDevice\n");
            return RET_ERR;
        }
    }
    
    packet.tag = TAG_OK;
    for (i=0; i<user->len_device; i++) {
        zmq_send(socket, myUser->username, USERNAME_LENGTH, ZMQ_SNDMORE);
        ret = zmq_send(socket, &packet, 1, 0);
        ret = zmq_recv(socket, NULL, 0, 0);
    }
    
    return RET_OK;
}
int connectToGroup(void *socket, Group *group) {
    int ret, i;
    PacketData packet;
    
    fetchNewDeviceInGroupFromRecvDataThread(group);
    for (i=1; i<group->len_device; i++) {
        ret = connectToDevice(socket, group->list_device[i]);
        if (ret == RET_ERR) {
            fprintf(stderr, "Error: in connectToDevice\n");
            return RET_ERR;
        }
    }
    
    packet.tag = TAG_OK;
    for (i=1; i<group->len_device; i++) {
        zmq_send(socket, group->groupname, GROUPNAME_LENGTH, ZMQ_SNDMORE);
        ret = zmq_send(socket, &packet, 1, 0);
        ret = zmq_recv(socket, NULL, 0, 0);
    }
    return RET_OK;
}

int disconnectDevice(void *socket, Device *device) {
    int ret;
    char endpoint_target[32];
    
    ENDPOINT_IPPORT(endpoint_target, device->ip, device->port);
    ret = zmq_disconnect(socket, endpoint_target);
    if (ret == -1) {
        fprintf(stderr, "Error: disconnect from %s return -1\n", device->id);
        return RET_ERR;
    }
    
    return RET_OK;
}
int disconnectUser(void *socket, User *user) {
    int ret, i;
    
    for (i=0; i<user->len_device; i++) {
        ret = disconnectDevice(socket, user->list_device[i]);
        if (ret == RET_ERR) {
            fprintf(stderr, "Error: in disconnectDevice\n");
            return RET_ERR;
        }
    }
    return RET_OK;
}
int disconnectGroup(void *socket, Group *group) {
    int ret, i;
    for (i=1; i<group->len_device; i++) {
        ret = disconnectDevice(socket, group->list_device[i]);
        if (ret == RET_ERR) {
            fprintf(stderr, "Error: in disconnectDevice\n");
            return RET_ERR;
        }
    }
    return RET_OK;
}

int signalToRecvDataForConnectFwder(void *context, char *fwder_endpoint) {
    void *socket;
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    packet.tag = TAG_FWD;
    strcpy((char *)packet.data, fwder_endpoint);
    packet.data_length = 32;
    zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
    zmq_close(socket);
    
    return RET_OK;
}

int signalToRecvDataForNewGroup(char *groupname) {
    void *socket;
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    packet.tag = TAG_JOINGROUP;
    strcpy((char *)packet.data, groupname);
    packet.data_length = GROUPNAME_LENGTH;
    zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
    zmq_recv(socket, NULL, 0, 0);
    zmq_close(socket);
    
    return RET_OK;
}
int signalToRecvDataForRemoveGroup(char *groupname) {
    void *socket;
    PacketData packet;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    packet.tag = TAG_JOINGROUP;
    strcpy((char *)packet.data, groupname);
    packet.data_length = GROUPNAME_LENGTH;
    zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
    zmq_recv(socket, NULL, 0, 0);
    zmq_close(socket);
    
    return RET_OK;
}

int fetchNewDeviceInMyUserFromRecvDataThread() {
    int ret;
    void *socket;
    PacketData packet;
    size_t size = 4;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    
    packet.tag = TAG_USER;
    zmq_send(socket, &packet, 1, 0);
    
    ret = zmq_recv(socket, &packet, PACKETDATA_FULLSIZE, 0);
    if (packet.tag == TAG_OK) {
        zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
        while (ret == 1) {
            myUser->func->AddCopyDevice(myUser, castDevice(packet.data));
            zmq_recv(socket, &packet, PACKETDATA_FULLSIZE, 0);
            zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
        }
        ret = RET_OK;
    } else {
        fprintf(stderr, "Error: recv_data thread doesn't return user\n");
        ret = RET_ERR;
    }
    
    zmq_close(socket);
    return ret;
}
int fetchNewDeviceInGroupFromRecvDataThread(Group *group) {
    int ret;
    void *socket;
    PacketData packet;
    Device *device;
    size_t size = 4;
    
    socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "inproc://recv_signal");
    
    packet.tag = TAG_GROUP;
    memcpy(packet.data, group->groupname, GROUPNAME_LENGTH);
    packet.data_length = GROUPNAME_LENGTH;
    zmq_send(socket, &packet, PACKETDATA_SIZE(packet), 0);
    
    ret = zmq_recv(socket, &packet, PACKETDATA_FULLSIZE, 0);
    if (packet.tag == TAG_OK) {
        zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
        while (ret == 1) {
            device = castDevice(packet.data);
            if (device->stat == TAG_STAT_ON) {
                group->func->AddCopyDevice(group, device);
            } else if (device->stat == TAG_STAT_OFF) {
                group->func->RemoveDevice(group, device);
            } else {
                fprintf(stderr, "Error: recv_data thread msg error\n");
                zmq_close(socket);
                return RET_ERR;
            }
            
            zmq_recv(socket, &packet, PACKETDATA_FULLSIZE, 0);
            zmq_getsockopt(socket, ZMQ_RCVMORE, &ret, &size);
        }
        ret = RET_OK;
    } else {
        fprintf(stderr, "Error: recv_data thread doesn't return group\n");
        ret = RET_ERR;
    }
    
    zmq_close(socket);
    return ret;
}
