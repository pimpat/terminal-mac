CC=gcc
FLAG=-lzmq -lsqlite3

RC_FILE=main.c config.c router.c client.c funcdb.c recvthread.c
RC_HEADER_FILE=config.h router.h client.h funcdb.h recvthread.h
RC_PATH=../Router_Client/
FRC_PATH=$(addprefix $(RC_PATH), $(RC_FILE))
FRC_HEADER_PATH=$(addprefix $(RC_PATH), $(RC_HEADER_FILE))

all: buildall

buildall: router_client.o

router_client.o: $(FRC_PATH) $(FRC_HEADER_PATH)
	$(CC) -o router_client.o $(FRC_PATH) $(FLAG)
	make test
clean:
	rm -rf *.o *.db

test:
	expect script.exp
