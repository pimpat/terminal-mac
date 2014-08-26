#include <stdio.h>
#include <stdlib.h>
#include "router.h"
#include <sqlite3.h>
#include "transport.h"

int main(int argc, const char * argv[])
{
    startRouter(5500);
    while (1);
    //stopRouter();
    
    //Client* x = malloc(sizeof(Client));
    //sprintf(x->id,"555");
    //sprintf(x->ip,"192.168.179.555");
    //x->status=0;
    //sprintf(x->name,"miang");
    //x->port=5555;
/*
    Packet* p = malloc(sizeof(Packet));
    p->tag=TAG_CLIENT;
    sprintf(p->data,"%s","555");
    //p->tag=TAG_USER;
    //sprintf(p->data,"%s","miang");
    //memcpy(p->data,x,sizeof(Client));
    testPacket(p);
    
    Client* a = (Client *)p->data;

    printf("id: %s\n",a->id);
    printf("name: %s\n",a->name);
    printf("status: %d\n",a->status);
    printf("ip: %s\n",a->ip);
    printf("port: %d\n",a->port);
*/
    
    /*
    // test
    registerUser("0001", "pim", "ONLINE", "192.168.179.94", 5504);
    registerUser("0002", "pluem", "ONLINE", "192.168.179.111", 5504);
    registerUser("0003", "boom", "ONLINE", "192.168.179.112", 5504);
    registerUser("0004", "pim", "ONLINE", "192.168.179.113", 5504);
    
    int i;
    i=setStatusUser("0001", "pim", "ONLINE", "192.168.179.94", 5504);
    i=setStatusUser("0002", "pluem", "ONLINE", "192.168.179.111", 5504);
    i=setStatusUser("0003", "boom", "OFFLINE", "192.168.179.112", 5504);
    
    
    int size;
    size = getSize("online");
    printf("online: %d\n",size);
    size = getSize("offline");
    printf("offline: %d\n",size);
    size = getSize("pim");
    printf("name: %d\n",size);
    size = getSize("all");
    printf("all: %d\n",size);
    
    Client** list;
    list = malloc(sizeof(list)*size);
    for(i=0;i<size;i++)
        list[i] = malloc(sizeof(Client));
    
    getList(list);
    
    for(i=0;i<size;i++){
        printf("-------------------------\n");
        printf("id: %s\n",list[i]->id);
        printf("name: %s\n",list[i]->name);
        printf("status: %d\n",list[i]->status);
        printf("ip: %s\n",list[i]->ip);
        printf("port: %d\n",list[i]->port);
    }
    
    Client* result;
    result = getInfoByID("0001");
    printf("-------------------------\n");
    printf("id: %s\n",result->id);
    printf("name: %s\n",result->name);
    printf("status: %d\n",result->status);
    printf("ip: %s\n",result->ip);
    printf("port: %d\n",result->port);
    
    size = getSize("pim");
    Client** n_result;
    n_result = malloc(sizeof(n_result)*size);
    for(i=0;i<size;i++)
        n_result[i] = malloc(sizeof(Client));
    
    getInfoByName("pim", n_result);
    
    for(i=0;i<size;i++){
        printf("========================\n");
        printf("id: %s\n",n_result[i]->id);
        printf("name: %s\n",n_result[i]->name);
        printf("status: %d\n",n_result[i]->status);
        printf("ip: %s\n",n_result[i]->ip);
        printf("port: %d\n",n_result[i]->port);
    }
    */
    return 0;
}