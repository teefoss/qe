EXEC    = $(shell basename $$(pwd))
CC      = clang
CFLAGS  = -Wall -g
LFLAGS	= -lncurses
SRC     = $(wildcard *.c)
OBJ		= $(SRC:.c=.o)

all: $(EXEC)

${EXEC}: $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	@rm -rf *.o
