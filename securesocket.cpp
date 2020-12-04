#include "securesocket.hpp"

#include <string> // std::string
#include <sstream> // std::ostringstream
#include <vector> // std::vector
#include <stdexcept> // std::runtime_error
#include <openssl/ssl.h> // SSL_*
#include <openssl/err.h> // ERR_print_errors_fp()
#include <unistd.h> // close()

SecureSocket::SecureSocket(int _fd, SSL_CTX *ctx) {
    init(_fd, ctx);
}

SecureSocket::~SecureSocket() {
    if (!initialized) return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
}

void SecureSocket::init(int _fd, SSL_CTX *ctx) {
    fd = _fd;
    
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    
    if (SSL_accept(ssl) <= 0) {
	// if SSL doesn't work, we just close and throw
	// ERR_print_errors_fp(stdout);

	std::string message = "HTTPS only.";
	::write(fd, message.c_str(), message.length());
	
	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(fd);

	throw std::runtime_error("SSL_accept() failed.");
    }
    initialized = true;
}

int SecureSocket::write(const std::string& message) {
    if (!initialized) return 0;
    // FIXME: avoid broken pipe error when the page is refreshed too often
    // (maybe just use separate threads to handle each request)
    return SSL_write(ssl, message.c_str(), message.length());
}

int SecureSocket::read(std::string& message) {
    if (!initialized) return 0;
    
    std::vector<char> buffer;
    buffer.reserve(64);
    
    // the request is read in portions into a buffer of chars and
    // transferred into a stringstream, which can be used to get a string
    // later
    std::ostringstream request_stream;
    
    while((size_t)SSL_read(ssl, buffer.data(), buffer.capacity()) >= buffer.capacity()) {
        request_stream << buffer.data();
	buffer.reserve(buffer.capacity()*2);
    }
    request_stream << buffer.data();

    message = request_stream.str();

    return message.length();
}
