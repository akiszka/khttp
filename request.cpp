#include "request.hpp"

#include <sstream> // std::stringstream
#include <string> // std::string

Request::Request(std::string req) {
    std::istringstream req_stream(req);
    std::string line;
    int state = 0; // 0 - waiting for the first line, 1 - processing headers, 2 - processing response body
    
    while (getline(req_stream, line)) {
        switch(state) {
	case 0: {
	    std::istringstream line_stream(line);
	    std::string word;

	    // find the request type
	    line_stream >> word;
	    if ("GET" == word) method = Method::GET;
	    else if ("POST" == word) method = Method::POST;
	    else method = Method::UNIMPLEMENTED;

	    // find the requested path
	    line_stream >> word;
	    path = word;

	    // find the HTTP version
	    line_stream >> word;
	    version = word;

	    // get to the next state (see above)
	    ++state;
	    break;
	}
	case 1: {
	    // find the colon in the header
	    auto delimeter_pos = line.find(":");
	    
	    // if there is no colon, skip to the next stage
	    if (delimeter_pos == std::string::npos) {
		++state;
		break;
	    }

	    // find the name and value of the header
	    auto name = line.substr(0, delimeter_pos);
	    auto value = line.substr(delimeter_pos+2/*skip the space after the colon*/);
	    // set the header
	    headers[name] = value;
	    break;
	}
	case 2: {
	    // get the body line by line
	    body += line;
	    break;
	}
	}
    }
}

Request::Method Request::get_method() const {
    return method;
}

std::string Request::get_path() const {
    return path;
}

std::string Request::get_version() const {
    return version;
}

std::string Request::get_header(const std::string header) const {
    try {
	return headers.at(header);
    } catch (...) {
	return "";
    }
}

std::string Request::get_body() const {
    return body;
}
