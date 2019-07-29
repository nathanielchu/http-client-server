#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <string>
#include <iostream>
#include <fstream>

#include "http-message.h"

#define MAXDATASIZE 1024

int write_file(const std::string path, const char *contents, bool is_head) {
    std::ios_base::openmode flags = (is_head) ? std::ios::binary : (std::ios::binary | std::ios::app);
    std::ofstream ofs(path.c_str(), flags);
    if(ofs.is_open()) {
        ofs.write(contents, strlen(contents));
        ofs.close();
        return 0;
    }

    return -1;
}

int main(int argc, char **argv)
{
    // process command line arguments
    if (argc < 2) {
        std::cerr << "usage: web-client [URL] [URL] ..." << std::endl;
        exit(-1);
    }

    std::string uris[argc - 1];
    std::string hosts[argc - 1];
    std::string ports[argc - 1];
    for (int i = 0; i < argc - 1; i++) {
        std::string url = argv[i + 1];
        if (url.substr(0,7) == "http://") {
            url = url.substr(7);
        }

        size_t pos = 0;
        std::string delimiter = "/";
        if ((pos = url.find(delimiter)) != std::string::npos) {
            uris[i] = url.substr(pos + delimiter.length());
            url = url.substr(0, pos);
        } else {
            std::cerr << "parsing arguments with delimiter /" << std::endl;
            exit(-1);
        }

        pos = 0;
        delimiter = ":";
        if ((pos = url.find(delimiter)) != std::string::npos) {
            hosts[i] = url.substr(0, pos);
            ports[i] = url.substr(pos + delimiter.length() );
        } else {
            std::cerr << "parse arguments with delimiter :" << std::endl;
            exit(-1);
        }
    }

	std::cout << "uris: " << std::endl;
	for (int i = 0; i < argc - 1; i++) {
		std::cout << "  " << uris[i] << std::endl;
	}
	std::cout << "hosts: " << std::endl;
	for (int i = 0; i < argc - 1; i++) {
		std::cout << "  " << hosts[i] << std::endl;
	}
	std::cout << "ports: " << std::endl;
	for (int i = 0; i < argc - 1; i++) {
		std::cout << "  " << ports[i] << std::endl;
	}

    // for each url
    for (int i = 0; i < argc - 1; i++) {
        const char *host = hosts[i].c_str();
        const char *port = ports[i].c_str();

        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        char ipstr[INET_ADDRSTRLEN];
        int rv;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(host, port, &hints, &servinfo)) < 0) {
            std::cerr << "client: getaddrinfo: " << gai_strerror(rv) << std::endl;
            exit(1);
        }

        // loop through all the results and connect to the first we can
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL) {
            std::cerr << "client: failed to connect" << std::endl;
            exit(1);
        }

        inet_ntop(p->ai_family, &(((struct sockaddr_in *)&(p->ai_addr))->sin_addr), ipstr, sizeof(ipstr));
        std::cout << "client: connecting to " << ipstr << std::endl;

        freeaddrinfo(servinfo);

        // send request
        HttpRequest req = HttpRequest(hosts[i] + ":" + ports[i], uris[i], "HTTP/1.0");
        std::string req_msg = req.serialize();
        std::cout << "client: serialize req:\n" << req.serialize() << std::endl;
        if (req_msg.length() > MAXDATASIZE) {
            std::cerr << "client: message length" << std::endl;
            exit(1);
        }
        if (send(sockfd, req_msg.c_str(), req_msg.length(), 0) < 0) {
            perror("client: send");
            exit(1);
        }

        char buf[MAXDATASIZE] = {'\0'};
        int numbytes = 0;

        // recv response
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) < 0) {
            perror("client: recv");
            exit(1);
        }

        buf[numbytes] = '\0';
        std::cout << "client: recv\n" << buf << std::endl;

        // parse response
        HttpResponse res = HttpResponse::parseResponse(std::string(buf), req.getVersion());
        if (res.getWellFormed() == false) {
            std::cout << "client: res not well formed" << std::endl;
        } else {
            // write file
            std::cout << "client: serialize res:\n" << res.serialize() << std::endl;
            write_file(uris[i], res.getBody().c_str(), true);
        }

        // if more content to be received
        size_t contentlen = res.getContentlen();
        size_t bodylen = res.getBody().length();
        while (bodylen < contentlen) {
            // recv response
            memset( buf, '\0', sizeof(char)*MAXDATASIZE );
            numbytes = 0;
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) < 0) {
                perror("client: recv");
                exit(1);
            }

            buf[numbytes] = '\0';
            std::cout << "client: recv\n" << buf << std::endl;
            // append to file
            write_file(uris[i], buf, false);
            
            bodylen += res.getBody().length();
        }

        close(sockfd);
    }

    return 0;
}
