//
//  define.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/4/2557 BE.
//
//

#define RET_OK 0
#define RET_EXIT 1
#define RET_FIREWALL 11
#define RET_ERR -1

#define MAX_LISTDEVICE 16
#define MAX_LISTUSER MAX_LISTDEVICE
#define MAX_LISTGROUP MAX_LISTDEVICE
#define MAX_DEVICE_PER_USER 16
#define MAX_DEVICE_PER_GROUP MAX_DEVICE_PER_USER

#define ID_LENGTH 16
#define USERNAME_LENGTH ID_LENGTH
#define GROUPNAME_LENGTH USERNAME_LENGTH

#define PACKETR_SIZE 44
#define PACKETDATA_FULLSIZE 1028
//#define PACKETR_SIZE sizeof(PacketR_desc)
//#define PACKETDATA_FULLSIZE sizeof(PacketData_desc)
#define PACKETDATA_SIZE(packet) 4+packet.data_length

#define TAG_OK          0
#define TAG_STAT_OFF    1
#define TAG_STAT_ON     2
#define TAG_DEVICE      3
#define TAG_USER        4
#define TAG_DATA        5
#define TAG_FIREWALL    6
#define TAG_FWD         7
#define TAG_JOINGROUP   8
#define TAG_LEAVEGROUP  9
#define TAG_GROUP       10
#define TAG_END         99
#define TAG_ERR         255

#define ENDPOINT_IPPORT(endpoint, ip32, port) \
    sprintf(endpoint, \
    "tcp://%hhu.%hhu.%hhu.%hhu:%hu", \
    (uint8_t)(ip32), \
    (uint8_t)(ip32>>8), \
    (uint8_t)(ip32>>16), \
    (uint8_t)(ip32>>24), \
    port);

