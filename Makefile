CC      = gcc
CFLAGS  = -g -Wall

all:       receiver processor

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

processor: processor.c
	$(CC) $(CFLAGS) -o processor processor.c

clean:
	rm -f *.o *~ core processor receiver
