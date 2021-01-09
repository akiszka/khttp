#include "response.hpp"

#include <string>
#include <sstream>

Response::Response(std::string _body) {
    set_body(_body);
}

void Response::set_header(std::string key, std::string value) {
    headers[key] = value;
}

void Response::set_status(Response::Status _status) {
    status = _status;
}

void Response::set_body(std::string _body) {
    body = _body;
    headers["Content-Length"] = std::to_string(body.length());
}

std::string Response::generate() const {
    std::ostringstream response;

    response << "HTTP/1.1 " << get_status_message() << "\r\n";

    for (const auto& header : headers) {
	response << header.first << ": " << header.second << "\r\n";
    }

    response << "\r\n" << body;

    return response.str();
}

Response Response::generate_error_message(const Status _status, std::string&& message) {
    Response response;
    response.set_header("Content-Type", "text/html");
    response.set_status(_status);
    response.set_body("<h1>" + response.get_status_message() + "</h1><br>" + message + "<hr><small>KHTTP/" KHTTP_VERSION "</small>");
    return response;
}

std::string Response::get_status_message() const {
    std::string message;
    
    switch (status) {
    case OK: message = "200 OK"; break;
    case CREATED: message = "201 Created"; break;
    case NO_CONTENT: message = "204 No Content"; break;
	
    case MOVED_PERMANENTLY: message = "301 Moved Permanently"; break;
    case MOVED_TEMPORARILY: message = "302 Found"; break;

    case BAD_REQUEST: message = "400 Bad Request"; break;
    case UNAUTHORIZED: message = "401 Unauthorized"; break;
    case FORBIDDEN: message = "403 Forbidden"; break;
    case NOT_FOUND: message = "404 Not Found"; break;
    case PAYLOAD_TOO_LARGE: message = "413 Payload Too Large"; break;
    case HEADERS_TOO_LARGE: message = "431 Request Header Fields Too Large"; break;
    case UNAVAILABLE_LEGAL_REASONS: message = "451 Unavailable For Legal Reasons"; break;

    case INTERNAL_SERVER_ERROR: message = "500 Internal Server Error"; break;
    case NOT_IMPLEMENTED: message = "501 Not Implemented"; break;
    case BAD_GATEWAY: message = "502 Bad Gateway"; break;
    case SERVICE_UNAVAILABLE: message = "503 Service Unavailable"; break;
    case LOOP_DETECTED: message = "508 Loop Detected"; break;
    }

    return message;
}
