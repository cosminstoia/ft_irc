#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "Client.hpp"

class Server
{
    public:
        Server(int port, std::string password);
        ~Server();

        void       setup();
        void       start();
        void       printErrorExit(const std::string &msg);
        void       acceptClient();
        void       handleClient(int clientSocket);
        // void       executeCommand(int clientSocket, const std::string &command);

    private:
        int         port_;                      // Port number to listen on
        std::string password_;                  // Password to connect to the server
        int         serverSocket_;             // Server socket file descriptor
        struct sockaddr_in serverAddr_;        // Server address
        std::vector<int> clientSockets_;       // Client socket file descriptors
};