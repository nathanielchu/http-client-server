#ifndef HTTP_MESSAGE
#define HTTP_MESSAGE

#include <string>

class HttpMessage {
public:
    HttpMessage(std::string protocol);
    HttpMessage(double version);
    void setProtocol(std::string protocol);
    void setProtocol(double version);
    void setWellFormed(bool well_formed);
    
    std::string getProtocol();
    double getVersion();
    bool getWellFormed();
protected:
    std::string protocol_;
    bool well_formed_;
};

class HttpRequest : public HttpMessage {
public:
    HttpRequest(std::string host, std::string uri, std::string protocol);
    HttpRequest(std::string protocol = "HTTP/1.0");
    void setMethod();
    void setUri(std::string uri);
    void setHost(std::string host);
    std::string serialize();

    std::string getMethod();
    std::string getUri();
    std::string getHost();

    static HttpRequest parseRequest(std::string msg);
protected:
    std::string method_;
    std::string uri_;
    std::string host_;
};

class HttpResponse : public HttpMessage {
public:
    HttpResponse(int status, double version, std::string body);
    HttpResponse(int status, double version = 1.0);
    std::string serialize();

    static HttpResponse parseResponse(std::string msg);
protected:
    int status_;
    double version_;
    std::string body_;
};

#endif // HTTP_MESSAGE
