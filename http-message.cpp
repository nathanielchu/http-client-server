#include <iostream>

#include "http-message.h"

/*
 * HTTP message
 */
HttpMessage::HttpMessage(std::string protocol)
    : well_formed_(true)
{
    setProtocol(protocol);
}

HttpMessage::HttpMessage()
    : well_formed_(false)
{}

void HttpMessage::setProtocol(std::string protocol) {
    if (protocol == "HTTP/1.0")
        protocol_ = "HTTP/1.0";
    else if (protocol == "HTTP/1.1")
        protocol_ = "HTTP/1.1";
    else
        well_formed_ = false;
}

void HttpMessage::setWellFormed(bool well_formed) {
    well_formed_ = well_formed;
}

std::string HttpMessage::getProtocol() {
    return protocol_;
}

double HttpMessage::getVersion() {
    return std::stod(protocol_.substr(5,8));
}

bool HttpMessage::getWellFormed() {
    return well_formed_;
}

/*
 * HTTP request
 */
HttpRequest::HttpRequest(std::string host, std::string uri, std::string protocol)
    : HttpMessage(protocol), method_("GET")
{
    setUri(uri);
    setHost(host);
}

HttpRequest::HttpRequest()
    : HttpMessage()
{}

void HttpRequest::setUri(std::string uri) {
    uri_ = uri;
    if (uri == "" || uri[0] != '/')
        uri = "/" + uri;
    for (size_t i = 0; i < uri.length(); i++) {
        if (uri[i] == ' ') {
            well_formed_ = false;
            std::cerr << "Request not well formed: space in uri" << std::endl;
        }
    }
}

void HttpRequest::setHost(std::string host) {
    host_ = host;
    size_t len = host.length();
    if (host_[len-1] == '/')
        host_.pop_back();
    for (size_t i = 0; i < len; i++) {
        if (host[i] == ' ') {
            well_formed_ = false;
            std::cerr << "Request not well formed: space in host" << std::endl;
        }
    }
}

std::string HttpRequest::serialize() {
    if (getWellFormed() == false)
        return "not well formed";

    std::string req;
    req += method_ + " "
        + uri_ + " "
        + protocol_ + "\r\n"
        + "Host: " + host_ + "\r\n\r\n";
    return req;
}

std::string HttpRequest::getMethod() { return method_; }
std::string HttpRequest::getUri() { return uri_; }
std::string HttpRequest::getHost() { return host_; }

HttpRequest HttpRequest::parseRequest(std::string msg) {
    std::string delim = "\r\n";
    size_t pos = msg.find(delim);
    std::string request_line = msg.substr(0, pos);
    std::string header = msg.substr(pos+2); // second line
    std::cout << "request_line: " << request_line << std::endl;
    std::cout << "header: " << header << std::endl;

    // parse request_line
    delim = " ";
    pos = request_line.find(delim);
    size_t prev = 0;
    std::string method = request_line.substr(prev, pos-prev);
    // only handle GET requests
    if (method != "GET") {
        std::cerr << "only handle GET requests" << std::endl;
        return HttpRequest();
    }
    
    prev = pos + 1;
    pos = request_line.find(delim, prev);
    std::string uri = request_line.substr(prev, pos-prev);
    
    prev = pos + 1;
    pos = request_line.find('\r', prev);
    std::string protocol = request_line.substr(prev, pos-prev);
    if (protocol.substr(0, 5) != "HTTP/") {
        std::cerr << "http protocol not well formed" << std::endl;
        return HttpRequest();
    }

    // parse h
    // assume Host is on first header line
    prev = 0;
    pos = header.find(delim);
    if (header.substr(0, pos) != "Host:") {
        std::cerr << "request header host not well formed" << std::endl;
        return HttpRequest();
    }
    prev = pos + 1;
    pos = header.find('\r');
    std::string host = header.substr(prev, pos-prev);

    return HttpRequest(host, uri, protocol);
}
