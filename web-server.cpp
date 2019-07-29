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
#include <sys/wait.h>
#include <signal.h>

#include <string>
#include <iostream>
#include <fstream>

#include "http-message.h"

#define MAXDATASIZE 1024

void sigchild_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(int argc, char **argv)
{
    // process command line arguments
    if (argc != 4) {
        std::cerr << "usage: web-server [hostname] [port] [file-dir]" << std::endl;
        exit(1);
    }
    std::string host = argv[1];
    std::string port = argv[2];
    std::string dir = argv[3];

    int sockfd, newfd; // listen on sock_fd, new connection on newfd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage client_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int reuse = 1;
    char ipstr[INET_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "server: getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)  {
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchild_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    std::cout << "server: waiting for connections..." << std::endl;
    while (true) { // main accept() loop
        sin_size = sizeof(client_addr);
        newfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (newfd < 0) {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.ss_family, &(((struct sockaddr_in *)&client_addr)->sin_addr), ipstr, sizeof(ipstr));
        std::cout << "server: got connection from " << ipstr << std::endl;

        if (!fork()) { // this is the child process
            close(sockfd); // child does'nt need the listener
            char buf[MAXDATASIZE] = {'\0'};
            int numbytes = 0;

            // recv request
            if ((numbytes = recv(newfd, buf, MAXDATASIZE - 1, 0)) < 0) {
                perror("server: recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            std::cout << "server: recv" << std::endl;

            // parse request
            HttpRequest req = HttpRequest::parseRequest(std::string(buf));
            int status = 200;
            if (req.getWellFormed() == false) {
                std::cout << "server: req not well formed" << std::endl;
                status = 400;
            }

            // read file and send response
            std::string uri = req.getUri();
            char filename[dir.length() + uri.length() + 2] = {'\0'};
            if (dir.substr(0,1) != ".")
                strcpy(filename, ".");
            strcat(filename, dir.c_str());
            strcat(filename, "/");
            strcat(filename, uri.c_str());

            std::ifstream ifs(filename, std::ios::binary);
            std::streampos fsize = ifs.tellg();
            ifs.seekg(0, std::ios::end);
            fsize = ifs.tellg() - fsize;
            ifs.clear();
            ifs.seekg(0, std::ios::beg);
            if (ifs) {
                // read file
                memset( buf, '\0', sizeof(char)*MAXDATASIZE );
                ifs.read(buf, MAXDATASIZE - 1);
                buf[MAXDATASIZE - 1] = '\0';

                // send response
                std::string body(buf);
                HttpResponse res = (status == 200) ? HttpResponse(status, req.getVersion(), body, fsize) : HttpResponse(status);
                std::string res_msg = res.serialize();
                std::cout << "server: send res:" << std::endl;
                if (send(newfd, res_msg.c_str(), res_msg.length(), 0) < 0) {
                    perror("server: send");
                    exit(1);
                }
            }
            while (ifs) {
                // read file
                memset( buf, '\0', sizeof(char)*MAXDATASIZE );
                ifs.read(buf, MAXDATASIZE - 1);
                buf[MAXDATASIZE - 1] = '\0';

                // send response
                std::string body(buf);
                if (send(newfd, buf, MAXDATASIZE - 1, 0) < 0) {
                    perror("server: send");
                    exit(1);
                }
            }

            close(newfd);
            exit(0);
        }
        close(newfd); // this is the parent process
    }

    return 0;
}
