#ifndef TransportLayer_recvthread_h
#define TransportLayer_recvthread_h

#define ENDPOINT_INTER_THREAD "inproc://clientSignal"

int startRecvData(void *context, int port);
int stopReceiveData(void *context);

#endif
