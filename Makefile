CC = gcc
CFLAGS = -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

banker: main.c banker.c cli.c
	$(CC) $(CFLAGS) main.c banker.c cli.c -o banker_gui $(LIBS)

clean:
	rm -f banker