#ifndef SERVER_HPP
#define SERVER_HPP

#include "request.hpp"
#include "response.hpp"
#include "threadsafequeue.hpp"

#include <string>
#include <filesystem>
#include <fstream>

#include <openssl/ssl.h>
#include <netinet/in.h>

class Server {
    const int port;
    const int server_fd;
    
    const int threads;
    ThreadsafeQueue<int> connections_queue;
    
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    SSL_CTX *ctx;
    
    const std::filesystem::path root;

    std::filesystem::path find_file(std::filesystem::path requested_path);
    static std::string extension_to_mimetype(std::string extension);

    void worker_thread_loop();
    void serve_requesting_socket(const int fd);
    std::unique_ptr<Response> process_request(const Request& req);
    
    static void loop_sigint_handler(int signum);
    
    void create_openssl_context(const std::string& cert_filename, const std::string& key_filename);
    void cleanup_openssl();
    void init_openssl();
    
public:
    Server(std::string _root, std::string cert_filename, std::string key_filename, int _port = 8080, int _threads = 4);
    ~Server();
    void accept_once();
    void loop();
};

#endif // SERVER_HPP
