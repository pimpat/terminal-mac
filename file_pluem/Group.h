//
//  Group.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/8/2557 BE.
//
//

#ifndef newTran_Group_h
#define newTran_Group_h

#include "define.h"
#include "Device.h"

typedef struct Group_desc Group;
typedef struct GroupFunc_desc GroupFunc;

struct Group_desc {
    Device *list_device[MAX_DEVICE_PER_GROUP];
    uint8_t len_device;
    char groupname[GROUPNAME_LENGTH];
    GroupFunc *func;
    
} Group_desc;

struct GroupFunc_desc {
    int (*getIndexById)(Group *self, char *id);
    void (*AddDevice)(Group *self, Device *device);
    void (*AddCopyDevice)(Group *self, Device *device);
    void (*RemoveAtDevice)(Group *self, int index);
    void (*RemoveDevice)(Group *self, Device *device);
    void (*setGroupname)(Group *self, char *groupname);
    void (*freeDevice)(Group *self);
    void (*free)(Group *self);
} GroupFunc_desc;

Group* newGroup(char *groupname);

Group* getGroupByNameFromList(Group **listgroup, int len, char *name);
void removeGroupFromList(Group **listgroup, int *len, char *name);

#endif
