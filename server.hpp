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
#include <atomic>
#include <openssl/ssl.h>

class Server {
    int port;
    int server_fd;
    
    std::atomic_int numthreads = 0;
    int maxthreads;
    
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    SSL_CTX *ctx;
    
    std::filesystem::path root;

    std::filesystem::path find_file(std::filesystem::path requested_path);
    static std::string extension_to_mimetype(std::string extension);
    
    void serve_requesting_socket(const int& fd);
    std::unique_ptr<Response> process_request(const Request& req);
    
    static void loop_sigint_handler(int signum);
    
    void create_context();
    void cleanup_openssl();
    void init_openssl();
    
public:
    Server(std::string _root, int _port = 8080, int _maxthreads = 12);
    ~Server();
    void accept_once();
    void loop();
};

#endif // SERVER_HPP
