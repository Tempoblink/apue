TARGET = default
SRC = $(filter-out Fig5.11_Print_buffering_for_various_standard_IO_streams.c, $(wildcard *.c))
OBJ = $(patsubst %.c, %.o, $(SRC))
CC = cc
LDFLAGS = -lapue

$(TARGET):$(OBJ)

%.o:%.c
	$(CC) -o $@ $< $(LDFLAGS)

.PYTHON:clean

clean:
	-rm -f $(OBJ)
