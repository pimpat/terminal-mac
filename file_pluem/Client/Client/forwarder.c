//
//  forwarder.c
//  Client
//
//  Created by Kasidej Bungbrakearti on 7/31/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

#include <stdlib.h>
#include <pthread.h>
#include <zmq.h>
#include "forwarder.h"

#include "../../mystruct.h"

#define FWD_PROXY_PORT_MIN 7700
#define FWD_PROXY_PORT_MAX 7710

typedef struct FwderArg_desc FwderArg;
struct FwderArg_desc {
    void *context;
    char frontend_endpoint[32];
    char backend_endpoint[32];
} FwderArg_desc;

uint16_t myLenFwderPort = FWD_PROXY_PORT_MIN;
pthread_t fwder_thread;

static void *forwarder_thread(void *argv) {
    void *frontend, *backend;
    FwderArg *arg = (FwderArg *)argv;
    
    frontend = zmq_socket(arg->context, ZMQ_ROUTER);
    backend = zmq_socket(arg->context, ZMQ_DEALER);
    zmq_bind(frontend, arg->frontend_endpoint);
    zmq_bind(backend, arg->backend_endpoint);
    
    zmq_proxy(frontend, backend, NULL);
    
    zmq_close(backend);
    zmq_close(frontend);
    free(argv);
    return NULL;
}

int startForwarderThread(uint16_t *frontend_port, void *context) {
    if (myLenFwderPort+1 > FWD_PROXY_PORT_MAX) {
        fprintf(stderr, "Error: fwd port is full\n");
        return RET_ERR;
    }
    
    FwderArg *fwder_arg = malloc(sizeof(FwderArg_desc));
    
    *frontend_port = myLenFwderPort;
    
    fwder_arg->context = context;
    sprintf(fwder_arg->frontend_endpoint, "tcp://*:%hu", myLenFwderPort);
    myLenFwderPort++;
    sprintf(fwder_arg->backend_endpoint, "tcp://*:%hu", myLenFwderPort);
    myLenFwderPort++;
    pthread_create(&fwder_thread, NULL, forwarder_thread, fwder_arg);
    return RET_OK;
}

int stopFwdProxy(void *context) {
    /*
     zmq_proxy() runs in the current thread and returns only if/when the current context is closed.
     */
    
    if(pthread_join(fwder_thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return RET_ERR;
    }
    
    return RET_OK;
}