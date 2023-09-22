CC = $(CROSS_COMPILE)gcc
LDFLAGS = -lpthread
CFLAGS = -Wall -Werror -g

DEPS = aesdsocket.h queue.h
OBJS = aesdsocket.o main.o

%o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: aesdsocket

aesdsocket: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ core