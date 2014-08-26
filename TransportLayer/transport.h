#ifndef TransportLayer_transport_h
#define TransportLayer_transport_h

#define PACKET_SIZE(packet) packet.dataSize+8
#define PACKET_SIZE_MAX 264

#define MAX_ID_LENGTH 16
#define MAX_NAME_LENGTH MAX_ID_LENGTH

enum Tag {
    TAG_OK = 0,
    TAG_MSG_CLIENT,
    TAG_MSG_USER,
    TAG_STATUS,
    TAG_CLIENT,
    TAG_USER,
    TAG_LISTUSER,
    TAG_CLOSE = 0x00ff,
};

enum Status { STATUS_ONLINE = 0, STATUS_OFFLINE };

typedef struct Packet Packet;
struct Packet {
    unsigned int tag;
    unsigned int dataSize;
    char data[256];
};

typedef struct Client Client;
struct Client {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    unsigned int  status;
    char ip[16];
    int port;
};


#endif
