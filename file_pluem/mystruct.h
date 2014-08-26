//
//  mystruct.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 7/9/2557 BE.
//
//

#include "define.h"

typedef struct PacketR_desc PacketR;
struct PacketR_desc {
    uint8_t tag;
    uint32_t ip;
    uint16_t port;
    char id[ID_LENGTH];
    char username[USERNAME_LENGTH];
};

typedef struct PacketData_desc PacketData;
struct PacketData_desc {
    uint8_t tag;
    uint16_t data_length;
    uint8_t data[1024];
};
