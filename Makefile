CC := clang++
CFLAGS := -std=c++17 -O0 -g -Wall -Wextra

OBJS = main.o server.o request.o response.o

.PHONY: all clean test

all: server

.cpp.o:
	$(CC) $(CFLAGS) -c $<

server: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

test: server
	./server

clean:
	rm $(OBJS) server
