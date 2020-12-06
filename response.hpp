#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <map>
#include <string>

class Response {
public:
    enum Status {
	// success
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	// redirection
	MOVED_PERMANENTLY = 301,
	MOVED_TEMPORARILY = 302,
	// client error
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	PAYLOAD_TOO_LARGE = 413,
	HEADERS_TOO_LARGE = 431,
	UNAVAILABLE_LEGAL_REASONS = 451,
	// server error
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
	LOOP_DETECTED = 508,
    };

    Response(std::string _body = "");
    void set_header(std::string key, std::string value);
    void set_status(Status _status);
    void set_body(std::string _body);
    std::string generate() const;
    static Response generate_error_message(const Status _status, std::string&& message);
    
protected:
    Status status = OK;
    std::map<std::string, std::string> headers;
    std::string body;

    std::string get_status_message() const;
};

#endif // RESPONSE_HPP
