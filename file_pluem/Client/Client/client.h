//
//  client.h
//  Client
//
//  Created by Kasidej Bungbrakearti on 8/13/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

#ifndef Client_client_h
#define Client_client_h

#include "../../mystruct.h"
#include "../../User.h"
#include "../../Device.h"
#include "../../Group.h"

#define DEFAULT_PORT_RECIEVE 7672
#define ENDPOINT_ROUTER "tcp://localhost:7671"

int getMyIP();
void getMyUsername(char *myUsername);
void getMyId(char *myId);
void getCommand(char *command);
void getTargetId(char *target_id);
void getTargetUsername(char *target);
void getTargetGroupname(char *target_groupname);
void getData(uint8_t *packet);
int BuildDataToUser(PacketData *packet);
int BuildDataToDevice(PacketData *packet);

int statusToRouter(uint8_t status);

int getOtherDeviceInMyUser();
int getOtherDeviceInMyGroup(Group *my_group);
int alertOtherDeviceInMyUser();
int alertOtherDeviceInMyGroup(Group *my_group, int tag);

int commandSend();
int commandSendDevice();
int commandJoinGroup();
int commandLeaveGroup();
int commandSendGroup();

Device* getDeviceInfoFromRouter(char *target_id);
User* getUserInfoFromRouter(char *target_username);
Group* getGroupInfoFromRouter(char *target_groupname);

int connectToDevice(void *socket, Device *device);
int connectToUser(void *socket, User *user);
int connectToGroup(void *socket, Group *group);

int disconnectDevice(void *socket, Device *device);
int disconnectUser(void *socket, User *user);
int disconnectGroup(void *socket, Group *group);

int signalToRecvDataForConnectFwder(void *context, char *fwder_endpoint);
int signalToRecvDataForNewGroup(char *groupname);
int signalToRecvDataForRemoveGroup(char *groupname);
int fetchNewDeviceInMyUserFromRecvDataThread();
int fetchNewDeviceInGroupFromRecvDataThread(Group *groupname);

#endif
