#include "server.hpp"
#include "request.hpp"

#include <unistd.h> // read()
#include <sys/socket.h> // socket(), setsockopt(), bind()...

#include <string> // std::string, string.c_str()...
#include <iostream> // std::cout
#include <stdexcept> // std::runtime_error
#include <vector> // std::vector
#include <sstream> // std::ostringstream

Server::Server(int _port) {
    port = _port;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	throw std::runtime_error("Socket init failed.");
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
		   &opt, sizeof(opt))) { 
        throw std::runtime_error("Setting socket options failed");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
        throw std::runtime_error("Socket binding failed.");
    }

    if (listen(server_fd, 3) < 0) {
	throw std::runtime_error("Listening on a socker failed.");
    }

    buffer.reserve(256);
}

Server::~Server() {
    close(server_fd);
}

void Server::accept_once() {
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
			     (socklen_t*)&addrlen))<0) {
	throw std::runtime_error("Accept() failed.");
    }

    // the request is read in portions into a buffer of chars and
    // transferred into a stringstream, which can be used to get a string
    // later
    std::ostringstream request;

    while((size_t)read(new_socket, buffer.data(), buffer.capacity()) >= buffer.capacity()) {
        request << buffer.data();
	buffer.reserve(buffer.capacity()*2);
    }
    request << buffer.data();

    Request request_processor(request.str());
    
    std::string hello = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n<b>Hello</b>"; // string builder or something...
    send(new_socket, hello.c_str(), hello.length(), 0);
    close(new_socket);
}

void Server::loop() {
    while(true) {
	accept_once();
    }
}
