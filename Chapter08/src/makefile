TARGET = default
SRC = $(filter-out Fig8.05_Print_a_description_of_the_exit_status.c, $(wildcard *.c))
OBJ = $(patsubst %.c, %.o, $(SRC))
CC = cc
LDFLAGS = -lapue

$(TARGET):$(OBJ)

%.o:%.c
	$(CC) -o $@ $< $(LDFLAGS)

.PYTHON:clean

clean:
	-rm -f $(OBJ)
