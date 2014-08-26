//
//  Device.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/4/2557 BE.
//
//

#ifndef newTran_Device_h
#define newTran_Device_h

#include "define.h"

typedef struct Device_desc Device;
typedef struct DeviceFunc_desc DeviceFunc;

struct Device_desc {
	uint8_t stat;
    uint32_t ip;
    uint16_t port;
   	char id[ID_LENGTH];
    DeviceFunc *func;
} Device_desc;

struct DeviceFunc_desc {
    void (*setStat)(Device *device, uint8_t stat);
    void (*setIP)(Device *device, uint32_t ip);
    void (*setPort)(Device *device, uint16_t port);
    void (*setId)(Device *device, char *id);
    void (*free)(Device *device);
} DeviceFunc_desc;

Device* newDevice(char *id);
Device* newDevice2(char *id, uint8_t stat, uint32_t ip, uint16_t port);
Device* castDevice(void *device);


// ListDevice

int initListDevice();
int addDeviceToList(Device *device);
int RemoveDeviceAtFromList(int index);
int RemoveDeviceFromList(Device *device);
Device* getDeviceByIdFromList(char *id);

#endif