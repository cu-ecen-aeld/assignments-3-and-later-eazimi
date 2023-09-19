# DEFAULT_CC = gcc
# CC = $(DEFAULT_CC)

# ifdef CROSS_COMPILE
# 	CC = $(CROSS_COMPILE)$(DEFAULT_CC)
# endif

# CFLAGS = -Wall -Werror
# LDFLAGS = -pthread

# SRCS = \
# 	queue.h	\
# 	aesdsocket.h \
# 	aesdsocket.c \
# 	main.c

# OBJS = $(SRCS:.c=.o)

# TARGET = aesdsocket

# all: $(TARGET)
		
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(TARGET): $(OBJS)
# 	$(CC) $(LDFLAGS) -o $@ $^

# clean:
# 	rm -f *.o $(TARGET)

CC ?= $(CROSS_COMPILE)gcc
LDFLAGS ?= -lpthread
CFLAGS ?= Wall -Werror -O2 -g

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf aesdsocket