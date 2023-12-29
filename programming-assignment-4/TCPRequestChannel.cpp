#include "TCPRequestChannel.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    //if server
    int p = atoi(_port_no.c_str());
    if (_ip_address == "") {
        //set up vars
        struct sockaddr_in server_addr;
        int server_sock, bind_stat, listen_stat;

        //socket - make socket - socket(int domain, int type, int protocol)
        //AF_INET = IPv4
        //SOCK_STREAM = TCP
        //0
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            perror("bad sock");
            exit(-1);
        }
        this->sockfd = server_sock;


        //provide necessary machine nfo for sockaddr_in
        //address family = IPv4
        //IPv4 address, use current IPv4 address (INADDR_ANY)
        //connection port
        //convert short from host byte order to network byte order
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(short(p));


        //bind - assign address to socket - bind(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
        bind_stat = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(bind_stat < 0) {
            perror("bad bind");
            exit(-1);
        }

        //listen - listen for client - listen(int sockfd, int backlog)
        listen_stat = listen(sockfd, 30); //is 30 right?
        if(listen_stat < 0){
            perror("bad listen");
            exit(-1);
        }

        //accept connection
        //this->accept_conn();

        //writen in a seperate method        
    }
    else { //for the client

        //set up vars
        struct sockaddr_in client_addr;
        int client_sock;

        //socket - make socket - socket(int domain, int type, int protocol)
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            perror("bad sock");
            exit(-1);
        }
        this->sockfd = client_sock;


        //address family = IPv4
        //connection port
        //convert short from host byte order to network byte order
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(short(p));

        //convert IP address from c-str to binary rep. for sin addr
        inet_pton(AF_INET, _ip_address.c_str(), &client_addr.sin_addr);

        //connect - connect to listening socket - connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
        connect(sockfd,(struct sockaddr*)&client_addr, sizeof(client_addr));

        //generate server's info based on parameters 
    }
    //  create scoket on the specified
    //      -specify domain, type and protocol
    //  bind the socket to addr set-sets up listening
    //  mark socket as listening

    //if client
    //  create scoket on the specified
    //      -specify domain, type and protocol
    //  connect socket to ip addr of the server
}

TCPRequestChannel::TCPRequestChannel(int _sockfd) : sockfd(_sockfd) {
    // Assign an existing socket to the object's socket file descriptor
}

TCPRequestChannel::~TCPRequestChannel () {
    //close the sockfd - close(int fd)
    close(this->sockfd);
}

int TCPRequestChannel::accept_conn() {
    //struct sockaddr_storage
    //implementing accept(...) -ret val the sockfd of client

    //accept - accept connection
    //socket file descriptor for accepted connection
    //accept connection
    //return socket file desripton
    struct sockaddr_storage storage;
    socklen_t client_len = sizeof(storage);
    int client_sockfd = accept(this->sockfd, (struct sockaddr*) &storage, &client_len);
    if (client_sockfd < 0) {
        perror("Accept failed");
    }
    return client_sockfd;
}



//read/write recieve/send
int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    ssize_t no_bytes; //# of bytes to read
    no_bytes = read(sockfd, msgbuf, msgsize); //read from socket - read(int fd, void *buf, size_t count)
    return no_bytes; //return # of bytes read
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    ssize_t no_bytes; //# of bytes to write
    no_bytes = write(sockfd, msgbuf, msgsize); //write to socket - write(int fd, void *buf, size_t count)
    return no_bytes; //return # of bytes written
}
