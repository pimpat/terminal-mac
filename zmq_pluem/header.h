//
//  header.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 7/9/2557 BE.
//
//

#define RET_OK 0
#define RET_EXIT 1
#define RET_ERR -1

#define ID_LENGTH 16

#define PACKETR_SIZE sizeof(PacketR_desc) // when stable, change to const

#define TAG_OK 0
#define TAG_STAT_OFF 1
#define TAG_STAT_ON 2
#define TAG_GETIP 3
#define TAG_ERR 255

typedef struct PacketR_desc PacketR;
struct PacketR_desc {
    uint8_t tag;
    char id[ID_LENGTH];
    uint32_t ip;
    uint16_t port;
} PacketR_desc;

typedef struct Status_desc Status;
struct Status_desc {
	uint8_t stat;
   	char id[ID_LENGTH];
	uint32_t ip;
    uint16_t port;
} Status_desc;