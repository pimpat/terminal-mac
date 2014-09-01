//
//  funcdb.h
//  TransportLayer
//
//  Created by Pimpat on 8/25/2557 BE.
//
//

#ifndef TransportLayer_funcdb_h
#define TransportLayer_funcdb_h

int registerUser(char* id, char* name, char* status, char* ip, int port);
int setStatusUser(char* id, char* name, char* status, char* ip, int port);
void createDatabase(char* path);
void onlineStatus(char* id);
void offlineStatus(char* id);
Client* getInfoByID(char* id);
int getInfoByName(char* name, Client** result);
int getSize(char* type);
int getList(Client** list);

#endif
