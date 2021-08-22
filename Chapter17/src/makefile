target = default
filter =  Fig17.02_Creating_a_full-duplex_pipe.c
src = $(filter-out $(filter), $(wildcard *.c))
obj = $(patsubst %.c, %.o, $(src))
CC = cc
LDFLAGS = -lapue

$(target):$(obj)

%.o:%.c
	$(CC) -o $@ $< $(LDFLAGS)

.PYTHON:clean

clean:
	-rm -f $(obj)
