CC=gcc
FLAG=-lzmq -lsqlite3 -I../

RC_FILE=main.c router_client.c funcdb.c recvthread.c
RC_PATH=../Router_Client/
FRC_PATH=$(addprefix $(RC_PATH), $(RC_FILE))

all: router_client.o

router_client.o:
	$(CC) -o router_client.o $(FRC_PATH) $(FLAG)
clean:
	rm -rf *.o
