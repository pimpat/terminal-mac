//
//  Device.c
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/4/2557 BE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Device.h"

void setStat(Device *device, uint8_t stat) {
    device->stat = stat;
}
void setIP(Device *device, uint32_t ip) {
    device->ip = ip;
}
void setPort(Device *device, uint16_t port) {
    device->port = port;
}
void setId(Device *device, char *id) {
    strcpy(device->id, id);
}

void freeDevice(Device *device) {
    free(device->func);
    free(device);
}

Device* newDevice2(char *id, uint8_t stat, uint32_t ip, uint16_t port) {
    Device *device = malloc(sizeof(Device_desc));
    if (device == NULL) {
        fprintf(stderr, "Error: can't malloc device\n");
        return NULL;
    }
    device->func = malloc(sizeof(DeviceFunc_desc));
    if (device->func == NULL) {
        fprintf(stderr, "Error: can't malloc device->func\n");
        free(device);
        return NULL;
    }
    
    device->stat = stat;
    device->ip = ip;
    device->port = port;
    if (id != NULL) {
        setId(device, id);
    }
    
    device->func->setStat = setStat;
    device->func->setIP = setIP;
    device->func->setPort = setPort;
    device->func->setId = setId;
    device->func->free = freeDevice;
    
    return device;
}
Device* newDevice(char *id) {
    return newDevice2(id, 0, 0, 0);
}

Device* castDevice(void *device) {
    ((Device *)device)->func = malloc(sizeof(DeviceFunc_desc));
    if (((Device *)device)->func == NULL) {
        fprintf(stderr, "Error: can't malloc device->func\n");
        free(device);
        return NULL;
    }
    ((Device *)device)->func->setStat = setStat;
    ((Device *)device)->func->setIP = setIP;
    ((Device *)device)->func->setPort = setPort;
    ((Device *)device)->func->setId = setId;
    ((Device *)device)->func->free = freeDevice;
    return ((Device *)device);
}


// List Device
Device *listDevice[MAX_LISTDEVICE];
int lenDevice;

int initListDevice() {
    int i;
    for (i=0; i<MAX_LISTDEVICE; i++) {
        listDevice[i] = NULL;
    }
    lenDevice = 0;
    return RET_OK;
}
int addDeviceToList(Device *device) {
    if (lenDevice >= MAX_LISTDEVICE) {
        fprintf(stderr, "Error: lenDevice >= MAX_LISTDEVICE\n");
        return RET_ERR;
    }
    listDevice[lenDevice++] = device;
    return RET_OK;
}
int RemoveDeviceAtFromList(int index) {
    int i;
    if (index >= lenDevice) {
        return RET_ERR;
    }
    for (i=index+1; i<lenDevice; i++) {
        listDevice[i-1] = listDevice[i];
    }
    listDevice[i-1] = NULL;
    lenDevice--;
    return RET_OK;
}
int RemoveDeviceFromList(Device *device) {
    int i;
    for (i=0; i<lenDevice; i++) {
        if (listDevice[i] == device || (strcmp(listDevice[i]->id, device->id)==0)) {
            for (i=i+1; i<lenDevice; i++) {
                listDevice[i-1] = listDevice[i];
            }
            listDevice[i-1] = NULL;
            lenDevice--;
            return RET_OK;
        }
    }
    return RET_ERR;
}
Device* getDeviceByIdFromList(char *id) {
    int i;
    for (i=0; i<lenDevice; i++) {
        if (strcmp(listDevice[i]->id, id) == 0 ) {
            return listDevice[i];
        }
    }
    return NULL;
}