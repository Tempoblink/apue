TARGET = default
FILTER = Fig15.07_Routines_to_let_a_parent_and_child_synchronize.c Fig15.12_The_popen_and_pclose_functions.c Fig15.19_Filter_to_add_two_numbers,_using_standard_IO.c Fig15.35_Mutual_exclusion_using_a_POSIX_semaphore.c
SRC = $(filter-out $(FILTER), $(wildcard *.c))
OBJ = $(patsubst %.c, %.o, $(SRC))
CC = cc
LDFLAGS = -lapue

$(TARGET):myuclc add2 $(OBJ)

myuclc:Fig15.14_Filter_to_convert_uppercase_characters_to_lowercase.c
	$(CC) -o $@ $< $(LDFLAGS)

add2:Fig15.19_Filter_to_add_two_numbers,_using_standard_IO.c
	$(CC) -o $@ $< $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ $< $(LDFLAGS)

.PYTHON:clean

clean:
	-rm -f $(OBJ) myuclc add2
