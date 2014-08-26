//
//  forwarder.h
//  Client
//
//  Created by Kasidej Bungbrakearti on 7/31/2557 BE.
//  Copyright (c) 2557 Kasidej Bungbrakearti. All rights reserved.
//

int startForwarderThread(uint16_t *frontend_port, void *context);
int stopFwdProxy(void *context);