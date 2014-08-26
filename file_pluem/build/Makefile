CC:=gcc
ROUTER_FOLDER:=../Router/Router/
ROUTER_FILE:=router.c layerdb.c
ROUTER_PATH:=$(addprefix $(ROUTER_FOLDER), $(ROUTER_FILE))
ROUTER_HEADER_FILE:=layerdb.h
ROUTER_HEADER:=$(addprefix $(ROUTER_FOLDER), $(ROUTER_HEADER_FILE))
ROUTER_FLAG:=-lzmq -lsqlite3

CLIENT_FOLDER:=../Client/Client/
CLIENT_FILE:=client.c recv_data.c forwarder.c
CLIENT_PATH:=$(addprefix $(CLIENT_FOLDER), $(CLIENT_FILE))
CLIENT_HEADER_FILE:=recv_data.h forwarder.h
CLIENT_HEADER:=$(addprefix $(CLIENT_FOLDER), $(CLIENT_HEADER_FILE))
CLIENT_FLAG:=-lzmq

CO_FOLDER:=../
CO_FILE:=Device.c User.c Group.c
CO_PATH:=$(addprefix $(CO_FOLDER), $(CO_FILE))
CO_HEADER_FILE:=Device.h User.h Group.h define.h mystruct.h
CO_HEADER:=$(addprefix $(CO_FOLDER), $(CO_HEADER_FILE))

all: router.o client.o

router.o: $(ROUTER_PATH) $(ROUTER_HEADER) $(CO_PATH) $(CO_HEADER)
	$(CC) -o router.o $(ROUTER_PATH) $(CO_PATH) $(ROUTER_FLAG);

client.o: $(CLIENT_PATH) $(CLIENT_HEADER) $(CO_PATH) $(CO_HEADER)
	$(CC) -o client.o $(CLIENT_PATH) $(CO_PATH) $(CLIENT_FLAG);

clean:
	rm -rf *.o

test: all
	expect attest.exp
