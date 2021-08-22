TARGET = default
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
CC = cc
LDFLAGS = -lapue

$(TARGET):$(OBJ)

%.o:%.c
	$(CC) -o $@ $< $(LDFLAGS)

.PYTHON:clean

clean:
	-rm -f $(OBJ)
