#include <stdio.h>
#include "client.h"

int main(int argc, const char * argv[])
{
    char id[32], name[32], ip[32];
    int port;
    scanf("%s %s %s %d", id, name, ip, &port);
    
    startClient(id, name, ip, port);
    
    char buffer[32];
    char target[16];
    while (1) {
        printf("target:");
        scanf("%s", target);
        if (target[0] == 'r') {
            Message *message = readBufferRecv();
            if (message != NULL) {
                printf("%s: %s\n", message->from, message->msg);
            }
        }
        else{
            printf("msg:");
            scanf("%s", buffer);
            //sendToClient(target, buffer, strlen(buffer));
            sendToUser(target,buffer,strlen(buffer));
        }
    }
    
    stopClient();
    return 0;
}