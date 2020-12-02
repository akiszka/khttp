#ifndef SERVER_HPP
#define SERVER_HPP

#include "request.hpp"
#include "response.hpp"

#include <netinet/in.h>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
class Server {
    int port;
    int server_fd;
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    std::vector<char> buffer;
    std::filesystem::path root;

    std::ifstream find_file(std::filesystem::path requested_path);
    std::unique_ptr<Response> process_request(const Request& req);
    
public:
    Server(std::string _root, int _port = 8080);
    ~Server();
    void accept_once();
    void loop();
};

#endif // SERVER_HPP
