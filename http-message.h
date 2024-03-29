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

    std::string getMethod();
    std::string getUri();
    std::string getHost();

    std::string serialize();
    static HttpRequest parseRequest(std::string msg);
protected:
    std::string method_;
    std::string uri_;
    std::string host_;
};

class HttpResponse : public HttpMessage {
public:
    HttpResponse(int status, double version, std::string body, size_t contentlen_);
    HttpResponse(int status, double version = 1.0);
    void setStatus(int status);
    void setReason(int status);

    std::string getBody();
    size_t getContentlen();

    std::string serialize();
    static HttpResponse parseResponse(std::string msg, double version);
protected:
    int status_;
    std::string reason_phrase_;
    double version_;
    std::string body_;
    size_t contentlen_;
};

#endif // HTTP_MESSAGE
