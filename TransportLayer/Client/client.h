#ifndef TransportLayer_client_h
#define TransportLayer_client_h

int startClient(char* id, char* name, char *ip, int port);
int stopClient(void);
int sendToClient(char *id, char* data, unsigned int dataSize);
int sendToUser(char* user, char* data, unsigned int dataSize);


typedef struct Message Message;
struct Message {
    char *msg;
    unsigned int size;
    char from[16];
};
Message* readBufferRecv();
void freeMessage(Message* message);

#endif
