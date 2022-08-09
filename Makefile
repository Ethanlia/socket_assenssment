TAG=./bin/main
SRC= $(wildcard src/*.c)
OBJ=$(SRC:%.c=%.o)
CC=gcc
 
override CONFIG += -I./include  

$(TAG):$(OBJ)
	$(CC)  $(^) -o $(@) $(CONFIG)
	


%.o:%.c
	$(CC) $< -o $(@) $(CONFIG) -c 


clean:
	$(RM) ./bin/*  ./src/*.o

.PHONY:clean