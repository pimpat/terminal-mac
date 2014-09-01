//
//  main.c
//  Router_Client
//
//  Created by Pimpat on 9/1/2557 BE.
//  Copyright (c) 2557 Pimpat. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "router_client.h"

int main(int argc, const char * argv[])
{
    char x;
    printf("Enter r/c: ");
    scanf("%c",&x);
    if (x=='r'){
        printf("[----Router----]\n");
        startRouter(5500);
        while (1);
        //stopRouter();
    }
    
    else if(x=='c'){
        printf("[----Client----]\n");
        char id[32], name[32], ip[32];
        int port;
        printf("Enter data: ");
        scanf("%s %s %s %d", id, name, ip, &port);
        startClient(id, name, ip, port);
        
        char buffer[32];
        char target[16];
        while (1) {
            printf("target:");
            scanf("%s", target);
            if (target[0] == 'r') {
                //printf("enter r\n");
                Message *message = readBufferRecv();
                if (message != NULL) {
                    printf("%s: %s\n", message->from, message->msg);
                }
            }
            else{
                printf("msg:");
                scanf("%s", buffer);
                //sendToClient(target, buffer, strlen(buffer));
                sendToUser(target,buffer,(unsigned int)strlen(buffer));
            }
        }
        stopClient();
    }
    
    else printf("exit\n");
    return 0;
}



