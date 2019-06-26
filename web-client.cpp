#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <thread>
#include <iostream>

int main()
{
    // default arguments
    int option;
    int port = 3000;
    std::string host = "localhost";

    // process command line options using getopt

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host.c_str());
    if (!hostEntry) {
        std::cout << "No such host name: " << host << std::endl;
        exit(-1);
    }

    // setup socket address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    int server = socket(PF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }

    // allocate buffer
    int buflen = 1024;
    char* buf = new char[buflen+1];

    std::cout << "Client read line from input" << std::endl;
    // read a line from standard input
    std::string line;
    while (std::getline(std::cin, line)) {
        // write the data to the server
        send(server, line.c_str(), line.length(), 0);

        // read the response
        memset(buf, 0, buflen);
        recv(server, buf, buflen, 0);
    
        // print the response
        std::cout << "client received response:\n" << buf << std::endl;

    }

    // close socket
    close(server);
}
 
