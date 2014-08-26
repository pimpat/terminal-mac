CC=gcc
FLAG=-lzmq -lsqlite3 -I/Users/pimpat/TransportLayer

R_FILE=main.c router.c funcdb.c
R_PATH=/Users/pimpat/TransportLayer/Router/
FR_PATH=$(addprefix $(R_PATH), $(R_FILE))

C_FILE=main.c client.c recvthread.c
C_PATH=/Users/pimpat/TransportLayer/Client/
FC_PATH=$(addprefix $(C_PATH), $(C_FILE))

all: chat

chat: router.o client.o

router.o: 
	$(CC) $(FR_PATH) $(FLAG) -o router.o
client.o:
	$(CC) $(FC_PATH) $(FLAG) -o client.o
clean:
	rm -rf *.o
