#include "server.hpp"
#include "request.hpp"
#include "response.hpp"

#include <unistd.h> // read()
#include <sys/socket.h> // socket(), setsockopt(), bind()...

#include <string> // std::string, string.c_str()...
#include <iostream> // std::cout
#include <stdexcept> // std::runtime_error, std::invalid_argument
#include <vector> // std::vector
#include <sstream> // std::ostringstream
#include <filesystem> // std::filesystem::path, std::filesystem::is_directory...
#include <fstream> // std::fstream
#include <memory> // std::unique_ptr, std::make_unique

Server::Server(std::string _root, int _port) {
    root = _root;
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
    std::ostringstream request_stream;

    while((size_t)read(new_socket, buffer.data(), buffer.capacity()) >= buffer.capacity()) {
        request_stream << buffer.data();
	buffer.reserve(buffer.capacity()*2);
    }
    request_stream << buffer.data();

    // now a request object is generated and processed by a special function
    // TODO: make this function swappable
    Request request(request_stream.str());
    auto response = process_request(request);
    
    std::string response_raw = response->generate();

    send(new_socket, response_raw.c_str(), response_raw.length(), 0);
    close(new_socket);
}

void Server::loop() {
    while(true) {
	accept_once();
    }
}

std::ifstream Server::find_file(std::filesystem::path requested_path) {
    std::filesystem::path path = root / requested_path.relative_path();
	
    if (std::filesystem::is_directory(path)) {
	path /= "index.html";
    }
    
    if (!std::filesystem::exists(path)) {
	throw std::invalid_argument("Doesn't exist");
    }

    return std::ifstream(path);
}

std::unique_ptr<Response> Server::process_request(const Request& req) {
    std::ifstream requested_file;

    // try to open the file or index.html
    // TODO: generic function for errors
    try {
	requested_file = find_file(req.get_path());
    } catch (const std::invalid_argument& ia) {
	// return a 404 if the file doesn't exist
	auto response = std::make_unique<Response>("The file does not exist.");
	response->set_status(Response::NOT_FOUND);
	return response;
    }

    // read the requested file into a string
    std::string contents;
    requested_file.seekg(0, std::ios::end);
    contents.resize(requested_file.tellg());
    requested_file.seekg(0, std::ios::beg);
    requested_file.read(&contents[0], contents.size());
    requested_file.close();


    auto response = std::make_unique<Response>(contents);
    response->set_header("Content-Type", "text/html");
    return response;
}
