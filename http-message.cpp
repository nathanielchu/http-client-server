#include <iostream>
#include <sstream>
#include <string>

#include "http-message.h"

/*
 * HTTP message
 */
HttpMessage::HttpMessage(std::string protocol)
    : well_formed_(true)
{
    setProtocol(protocol);
}

HttpMessage::HttpMessage(double version)
    : well_formed_(true)
{
    setProtocol(version);
}

void HttpMessage::setProtocol(std::string protocol) {
    if (protocol == "HTTP/1.0")
        protocol_ = "HTTP/1.0";
    else if (protocol == "HTTP/1.1")
        protocol_ = "HTTP/1.1";
    else
        well_formed_ = false;
}

void HttpMessage::setProtocol(double version) {
    if (version == 1.0)
        protocol_ = "HTTP/1.0";
    else if (version == 1.1)
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

HttpRequest::HttpRequest(std::string protocol)
    : HttpMessage(protocol)
{
    well_formed_ = false;
}

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

std::string HttpRequest::getMethod() { return method_; }
std::string HttpRequest::getUri() { return uri_; }
std::string HttpRequest::getHost() { return host_; }

std::string HttpRequest::serialize() {
    if (well_formed_ == false)
        return "not well formed";

    std::ostringstream ss;
    ss << method_ << " "
        << uri_ << " "
        << protocol_ << "\r\n"
        << "Host: " << host_ << "\r\n\r\n";
    return ss.str();
}

HttpRequest HttpRequest::parseRequest(std::string msg) {
    size_t pos = msg.find("\r\n");
    std::string request_line = msg.substr(0, pos);
    std::string header = msg.substr(pos+2);

    // parse request_line
    std::string delim = " ";
    pos = request_line.find(delim);
    std::string method = request_line.substr(0, pos);
    // only handle GET requests
    if (method != "GET") {
        std::cerr << "only handle GET requests" << std::endl;
        return HttpRequest();
    }
    
    size_t prev = pos + 1;
    pos = request_line.find(delim, prev);
    std::string uri = request_line.substr(prev, pos-prev);
    
    prev = pos + 1;
    pos = request_line.find('\r', prev);
    std::string protocol = request_line.substr(prev, pos-prev);
    if (protocol.substr(0, 5) != "HTTP/") {
        std::cerr << "http protocol not well formed" << std::endl;
        return HttpRequest();
    }

    // parse header
    std::string host;
    std::istringstream ss(header);
    std::string line;
    while (std::getline(ss, line)) {
        pos = line.find(delim);
        if (line.substr(0, pos) == "Host:") {
            prev = pos + 1;
            pos = line.find('\r');
            host = line.substr(prev, pos-prev);
        }
    }
    if (host.empty()) {
        std::cerr << "http header not well formed" << std::endl;
        return HttpRequest(protocol);
    }

    return HttpRequest(host, uri, protocol);
}

/*
 * Http Response
 */
HttpResponse::HttpResponse(int status, double version, std::string body, size_t bodylen)
    : HttpMessage(version), body_(body), bodylen_(bodylen)
{
    setStatus(status);
    setReason(status);
}

HttpResponse::HttpResponse(int status, double version)
    : HttpMessage(version), body_(""), bodylen_(0)
{
    setStatus(status);
    setReason(status_);
}

void HttpResponse::setStatus(int status) {
    if (status > 200 && status < 300)
        status_ = 200;
    else if (status != 404 && status > 400 && status < 500)
        status_ = 400;
    else
        status_ = status;
}


void HttpResponse::setReason(int status) {
    switch(status) {
        case 200:
            reason_phrase_ = "OK";
            break;
        case 400:
            reason_phrase_ = "Bad request";
            break;
        case 404:
            reason_phrase_ = "404 Not found";
            break;
        default:
            well_formed_ = false;
            break;
    }
}

std::string HttpResponse::getBody() {
    return body_;
}

size_t HttpResponse::getBodylen() {
    return bodylen_;
}

std::string HttpResponse::serialize() {
    std::ostringstream ss;
    ss << protocol_ << " " << status_ << " " << reason_phrase_
        << "\r\n"
        << "Content-Type: text/html\r\n"
        << "Content-Length: "
        << bodylen_
        << "\r\n\r\n"
        << body_;
    return ss.str();
}

bool isNumber(std::string str) {
    std::istringstream ss(str);
    int num;
    char c;
    if (!(ss >> num) || (ss.get(c)))
        return false;
    return true;
}

HttpResponse HttpResponse::parseResponse(std::string msg, double version) {
    size_t pos = msg.find("\r\n");
    std::string status_line = msg.substr(0, pos);
    std::string header = msg.substr(pos+2);

    // parse status_line
    std::string delim = " ";
    pos = status_line.find(delim);
    std::string protocol = status_line.substr(0, pos);
    if (protocol.substr(0, 5) != "HTTP/") {
        std::cerr << "http protocol not well formed" << std::endl;
        return HttpResponse(400, version);
    }

    size_t prev = pos + 1;
    pos = status_line.find(delim, prev);
    std::string status = status_line.substr(prev, pos-prev);
    if (isNumber(status) == false) {
        std::cerr << "http status code not well formed" << std::endl;
        return HttpResponse(400, version);
    }

    // parse header
    prev = header.find("Content-Length:");
    prev = header.find(delim, prev) + 1;
    pos = header.find("\r\n", prev);
    std::string content_length = header.substr(prev, pos-prev);
    if (isNumber(content_length) == false) {
        std::cerr << "http entity header Content-Length not well formed" << std::endl;
        return HttpResponse(400, version);
    }

    pos = header.find("\r\n\r\n") + 4;
    if (pos == std::string::npos) {
        std::cerr << "http message-body not well formed" << std::endl;
        return HttpResponse(400, version);
    }
    std::string content = header.substr(pos, stoi(content_length));
    
    return HttpResponse(200, version, content, stoi(content_length));
}
