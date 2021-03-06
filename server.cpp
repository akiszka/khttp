#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "securesocket.hpp"

#include <unistd.h> // close()
#include <sys/socket.h> // socket(), setsockopt(), bind()...

#include <string> // std::string, string.c_str()...
#include <iostream> // std::cout, std::cerr
#include <stdexcept> // std::runtime_error, std::invalid_argument
#include <filesystem> // std::filesystem::path, std::filesystem::is_directory...
#include <memory> // std::unique_ptr, std::make_unique
#include <thread> // std::thread
#include <signal.h> // sigaction()

#include <openssl/ssl.h> // SSL_*
#include <openssl/err.h> // ERR_print_errors_fp()

volatile sig_atomic_t loop_running = false; // I hate this.

Server::Server(std::string _root,
	       std::string cert_filename,
	       std::string key_filename,
	       int _port,
	       int _threads) : port(_port), server_fd(socket(AF_INET, SOCK_STREAM, 0)), threads(_threads), root(_root) {
    init_openssl();
    create_openssl_context(cert_filename, key_filename);

    {
	// when the client happens to close connection before writing,
	// I don't want the program to crash with SIGPIPE
	struct sigaction sigpipe{};
	sigpipe.sa_handler = SIG_IGN;
	sigpipe.sa_flags = 0;
	sigaction(SIGPIPE, &sigpipe, NULL);
    }

    if (server_fd == 0) {
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
}

Server::~Server() {
    close(server_fd);
    cleanup_openssl();
}

void Server::accept_once() {
    int new_fd;

    if ((new_fd = accept(server_fd, (struct sockaddr *)&address,
			     (socklen_t*)&addrlen))<0) {
	throw std::runtime_error("Accept() failed.");
    }

    connections_queue.enqueue(new_fd);
}

void Server::worker_thread_loop() {
    while (true)
	serve_requesting_socket(connections_queue.dequeue());
}

void Server::serve_requesting_socket(const int fd) {
    SecureSocket new_socket;

    try {
	new_socket.init(fd, ctx);
    } catch (const std::runtime_error& re) {
	return;
    }

    std::unique_ptr<Response> response;
    try {
	// read the data that was sent
	std::string request_raw;
	new_socket.read(request_raw);

	// now a request object is generated and processed
	// TODO: make this function swappable
	Request request(std::move(request_raw));
	response = process_request(request);
    } catch (const std::exception& e) {
	response = std::make_unique<Response>(
	    Response::generate_error_message(Response::INTERNAL_SERVER_ERROR,
					     "Error."));
	std::cerr << e.what() << std::endl;

    }

    std::string response_raw = response->generate();

    new_socket.write(response_raw);
}

extern "C" void Server::loop_sigint_handler(int signum) {
    (void)signum;
    loop_running = false;
}

void Server::loop() {
    for (int i = 0; i < threads; ++i)
	(std::thread(&Server::worker_thread_loop, this)).detach();

    // there is an assumption only one loop can be running at a time
    loop_running = true;

    // add a new handler for ctrl+c
    struct sigaction sigint_new{};
    struct sigaction sigint_old{};
    sigint_new.sa_handler = &Server::loop_sigint_handler;
    sigint_new.sa_flags = 0;
    sigaction(SIGINT, &sigint_new, &sigint_old);

    std::cout << "Entering a server loop... Press CTRL+C to exit." << std::endl;

    while (loop_running) {
	try {
	    accept_once();
	} catch (...) {}
    }

    // after the server stops running, re-establish the old handler
    sigaction(SIGINT, &sigint_old, NULL);

    std::cout << "The process will now terminate, but it's nothing to worry about." << std::endl;
    std::terminate();
}

std::filesystem::path Server::find_file(std::filesystem::path requested_path) {
    std::filesystem::path path = root / requested_path.relative_path();

    if (std::filesystem::is_directory(path)) {
	path /= "index.html";
    }

    if (!std::filesystem::exists(path)) {
	throw std::invalid_argument("Doesn't exist");
    }

    return path;
}

std::string Server::extension_to_mimetype(std::string extension) {
    if (extension == ".html" || extension == ".htm")
	return "text/html";
    else if (extension == ".css")
	return "text/css";
    else if (extension == ".js")
	return "text/javascript";
    else if (extension == ".jpg" || extension == ".jpeg")
	return "image/jpeg";
    else if (extension == ".png")
	return "image/png";
    else if (extension == ".txt")
	return "text/plain";
    else if (extension == ".wasm")
	return "application/wasm";
    else
	return "application/octet-stream";
}

std::unique_ptr<Response> Server::process_request(const Request& req) {
    if (req.get_method() != Request::Method::GET) {
	return std::make_unique<Response>(
	    Response::generate_error_message(Response::BAD_REQUEST,
		"Wrong request method."));
    }


    std::filesystem::path filename;

    // try to open the file or index.html
    try {
	filename = find_file(req.get_path());
    } catch (const std::invalid_argument& ia) {
	// return a 404 if the file doesn't exist
	return std::make_unique<Response>(
	    Response::generate_error_message(Response::NOT_FOUND,
	    "File not found: " + req.get_path()));
    } catch (const std::filesystem::filesystem_error& fse) {
	return std::make_unique<Response>(
	    Response::generate_error_message(Response::BAD_REQUEST,
					     "Filename too long."));
    }

    std::ifstream requested_file(filename);

    // read the requested file into a string
    std::string contents;
    requested_file.seekg(0, std::ios::end);
    contents.resize(requested_file.tellg());
    requested_file.seekg(0, std::ios::beg);
    requested_file.read(&contents[0], contents.size());

    auto response = std::make_unique<Response>(contents);
    response->set_header("Content-Type", extension_to_mimetype(filename.extension()));

    return response;
}

void Server::init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void Server::cleanup_openssl() {
    SSL_CTX_free(ctx);
    EVP_cleanup();
}

void Server::create_openssl_context(const std::string& cert_filename, const std::string& key_filename) {
    const SSL_METHOD *method;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	std::cerr << "Unable to create SSL context." << std::endl;
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, cert_filename.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_filename.c_str(), SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }
}
