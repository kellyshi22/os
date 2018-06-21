EXEC = sds
OBJ = sds.o
CFLAGS = -Wall -g -std=c99 -pthread -D_XOPEN_SOURCE=500 -lrt -Wno-deprecated -Wunused-variable
CC = gcc

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(CFLAGS)

sds.o : sds.c sds.h
	$(CC) -c sds.c $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ)
