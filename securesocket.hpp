#ifndef SECURESOCKET_HPP
#define SECURESOCKET_HPP

#include <string>
#include <openssl/ssl.h>

class SecureSocket {
public:
    SecureSocket() = default;
    SecureSocket(int _fd, SSL_CTX *ctx);
    ~SecureSocket();
    void init(int _fd, SSL_CTX *ctx);
    int write(const std::string& message);
    int read(std::string& message);
    
protected:
    int fd;
    SSL* ssl;
    bool initialized = false;
};

#endif // SECURESOCKET_HPP
