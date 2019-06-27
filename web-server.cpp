#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>


void handleClient(int client)
{
    char * buf;
    int buflen;
    int nread;

    // allocate buffer
    buflen = 1024;
    buf = new char[buflen+1];

    // loop to handle all requests
    while (true) {
        // read a request
        memset(buf, 0, buflen);
        nread = recv(client, buf, buflen, 0);
        if (nread == 0)
            break;
        std::cout << "server received request:\n" << buf << std::endl;

        // send a response
        send(client, buf, nread, 0);
    }
    close(client);
}

void join_thread(std::thread& t)
{
    t.join();
}

int main(int argc, char **argv)
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t clientlen = sizeof(client_addr);
    int option, port, reuse;
    int server, client;

    // setup default arguments
    port = 3000;

    // process command line options using getopt()
    while ((option = getopt(argc, argv, "p:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);

                break;
            default:
                std::cout << "server [-p port]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    // setup socket address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server = socket(PF_INET, SOCK_STREAM, 0);
    if (!server) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    reuse = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and port
    if (bind(server, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server, SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }

    std::vector<std::thread> threads;

    // accept clients
    while ((client = accept(server, (struct sockaddr *)&client_addr, &clientlen)) > 0) {
        threads.push_back(std::thread(handleClient, client));
    }
    close(server);
    std::for_each(threads.begin(), threads.end(), join_thread);
}
