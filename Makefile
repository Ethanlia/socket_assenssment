QL_TARGET_SERVER_EXE  = server
QL_TARGET_SERVER_FILE = ./src/serve.c ./src/log.c ./src/func.c ./src/data_collection.c 

QL_TARGET_CLIENT_EXE  = client
QL_TARGET_CLIENT_FILE = ./src/client.c ./src/log.c ./src/func.c ./src/data_collection.c

SERVER_TAG=./bin/$(QL_TARGET_SERVER_EXE)
CLIENT_TAG=./bin/$(QL_TARGET_CLIENT_EXE)
SERVER_SRC=$(QL_TARGET_SERVER_FILE)
CLIENT_SRC=$(QL_TARGET_CLIENT_FILE)
SERVER_OBJ=$(SERVER_SRC:%.c=%.o)
CLIENT_OBJ=$(CLIENT_SRC:%.c=%.o)
# VPATH = ./src 
CC=gcc

override CONFIG += -I ./include  


all: $(SERVER_TAG) $(CLIENT_TAG) 

%.o:%.c
	$(CC) -o $@ -c $^ $(CONFIG)

$(SERVER_TAG):$(SERVER_OBJ)
	$(CC)   -o $@ $(SERVER_OBJ) $(CONFIG)

$(CLIENT_TAG):$(CLIENT_OBJ)
	$(CC)   -o $@ $(CLIENT_OBJ) $(CONFIG)


clean:
	$(RM) ./bin/*  ./src/*.o 


.PHONY:clean