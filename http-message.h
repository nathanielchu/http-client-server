#ifndef HTTP_MESSAGE
#define HTTP_MESSAGE

#include <string>

class HttpMessage {
public:
    HttpMessage(std::string protocol);
    HttpMessage();
    void setProtocol(std::string protocol);
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
    HttpRequest();
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

#endif // HTTP_MESSAGE
