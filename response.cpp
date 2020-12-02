#include "response.hpp"

#include <string>
#include <sstream>

Response::Response(std::string _body) {
    body = _body;
    headers["Content-Length"] = std::to_string(body.length());
}

void Response::set_header(std::string key, std::string value) {
    headers[key] = value;
}

void Response::set_status(Response::Status _status) {
    status = _status;
}

std::string Response::generate() const {
    std::ostringstream response;

    response << "HTTP/1.1 ";
    
    switch (status) {
    case OK: response << "200 OK"; break;
    case CREATED: response << "201 Created"; break;
    case NO_CONTENT: response << "204 No Content"; break;
	
    case MOVED_PERMANENTLY: response << "301 Moved Permanently"; break;
    case MOVED_TEMPORARILY: response << "302 Found"; break;

    case BAD_REQUEST: response << "400 Bad Request"; break;
    case UNAUTHORIZED: response << "401 Unauthorized"; break;
    case FORBIDDEN: response << "403 Forbidden"; break;
    case NOT_FOUND: response << "404 Not Found"; break;
    case PAYLOAD_TOO_LARGE: response << "413 Payload Too Large"; break;
    case HEADERS_TOO_LARGE: response << "431 Request Header Fields Too Large"; break;
    case UNAVAILABLE_LEGAL_REASONS: response << "451 Unavailable For Legal Reasons"; break;

    case INTERNAL_SERVER_ERROR: response << "500 Internal Server Error"; break;
    case NOT_IMPLEMENTED: response << "501 Not Implemented"; break;
    case BAD_GATEWAY: response << "502 Bad Gateway"; break;
    case SERVICE_UNAVAILABLE: response << "503 Service Unavailable"; break;
    case LOOP_DETECTED: response << "508 Loop Detected"; break;
    }

    response << "\r\n";

    for (const auto& header : headers) {
	response << header.first << ": " << header.second << "\r\n";
    }

    response << "\r\n";
    response << body;

    return response.str();
}
