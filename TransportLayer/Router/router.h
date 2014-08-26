#ifndef TransportLayer_router_h
#define TransportLayer_router_h
#include "transport.h"

int startRouter(int port);
int stopRouter(void);
/*
int registerUser(char* id, char* name, char* status, char* ip, int port);
int setStatusUser(char* id, char* name, char* status, char* ip, int port);
void createDatabase(char* path);
void onlineStatus(char* id);
void offlineStatus(char* id);
Client* getInfoByID(char* id);
int getInfoByName(char* name, Client** result);
int getSize(char* type);
int getList(Client** list);
*/
void testPacket(Packet* packet);

#endif
