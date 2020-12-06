CC = clang++
CFLAGS = -std=c++20 -O3 -g -Wall -Wextra
LINKFLAGS = -lssl -lcrypto -lpthread

OBJS = main.o server.o request.o response.o securesocket.o

.PHONY: all clean test certs cleancerts

all: server certs

# CODE *******************************************

.cpp.o:
	$(CC) $(CFLAGS) -c $<

server: $(OBJS)
	$(CC) $(CFLAGS) $(LINKFLAGS) $(OBJS) -o $@

clean:
	rm $(OBJS) server

# CERTIFICATES *******************************************

certs: crt/csr crt/crt

crt/csr:
	mkdir -p crt
	openssl req -new -newkey rsa:4096 -nodes \
	-keyout crt/key -out crt/csr \
	-subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com"

crt/crt: crt/csr
	openssl req -new -newkey rsa:4096 -days 365 -nodes -x509 \
	-subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com" \
	-keyout crt/key  -out crt/crt

cleancerts:
	rm crt/*

# MISC *******************************************

test: server certs
	./server
