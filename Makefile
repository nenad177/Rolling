PROGRAM = rolling
CC      = gcc
CFLAGS  = -g -Wall -lm
LDLIBS  = -lglut -lGLU -lGL -lSOIL

$(PROGRAM): main.c
	$(CC) -o $(PROGRAM) main.c $(CFLAGS) $(LDLIBS)

.PHONY: clean 

clean:
	-rm *.o $(PROGRAM) *core