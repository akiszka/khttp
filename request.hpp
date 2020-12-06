#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    enum class Method {
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH,
	_OTHER // a different, unimplemented method
    };

    Request(std::string&& req);
    Method get_method() const;
    std::string get_path() const;
    std::string get_version() const;
    std::string get_header(const std::string header) const;
    std::string get_body() const;
    
protected:
    Request::Method method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif // REQUEST_HPP
