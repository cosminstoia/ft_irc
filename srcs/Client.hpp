#pragma once

#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

class Client
{
    private:
        std::string ip; // ip address of the server
        int clientSocket; // file descriptor to store the values returned by the socket system call and the accept system call.
        int portNum; // storing port number on which the accepts connections (the port must be the same for client and server)
        bool isExit; // bool variable which will be used to end the loop
        int buffSize; // The client reads characters from the socket connection into a dynamic variable (buffer)
        char *buffer;
        struct sockaddr_in serverAddr; // structure containing an internet address. This structure is already defined in netinet/in.h , serverAddr will contain the address of the server
    public:
        Client(std::string ip, int clientSocket);
        ~Client();
        bool connectToServer();
        void communicate();
};