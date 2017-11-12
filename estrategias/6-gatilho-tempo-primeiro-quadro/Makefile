CC=gcc
CFLAGS=-I.
DEPS = bridge.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bridge: bridge.c
	gcc -o bridge-a.out bridge-a.c bridge.c -I. -lpthread
