#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <memory>
#include <vector>

class Server {
    int port;
    int server_fd;
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    std::vector<char> buffer;
    
public:
    Server(int _port);
    ~Server();
    void accept_once();
    void loop();
};

#endif // SERVER_HPP
