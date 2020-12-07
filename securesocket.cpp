#include "securesocket.hpp"
#include "response.hpp"

#include <string> // std::string
#include <vector> // std::vector
#include <stdexcept> // std::runtime_error
#include <openssl/ssl.h> // SSL_*
#include <unistd.h> // close(), write()

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

	
	std::string message = Response::generate_error_message(
	    Response::NOT_IMPLEMENTED,
	    "HTTPS only."
	    ).generate();
	(void) ::write(fd, message.c_str(), message.length());
	
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
    buffer.resize(64);

    size_t bytes_read_total = 0;
    while (true) {
	size_t bytes_read_now = SSL_read(
	    ssl,
	    buffer.data()+bytes_read_total,
	    buffer.size()-bytes_read_total);
	
	if (bytes_read_now < buffer.size()-bytes_read_total) break;
	
	bytes_read_total += bytes_read_now;
	
	if (buffer.size() >= 1024) {
	    throw std::runtime_error("Too long request.");
	} else {
	    buffer.resize(buffer.size()*2);
	}
    }

    message = std::string(buffer.data());
        
    return message.length();
}
