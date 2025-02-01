#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <arpa/inet.h>

#include "Client.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"


enum messageType
{
    INFO,
    WARNING,
    CONNECTION,
    DISCONNECTION,
    PING,
    PONG,
    CLIENT,
    SERVER,
    TIMEOUT,
    SUCCESS,
};

class Server
{
    public:
        Server(int port, std::string password);
        ~Server();

        void       setup();
        void       start();
        void       printErrorExit(std::string const& msg, bool exitP = false);
        void       printInfo(messageType type, std::string const& msg);
        void       acceptClient();
        void       handleClient(int clientSocket);
        // void       executeCommand(int clientSocket, const std::string &command);

    private:
        int         port_;                      // Port number to listen on
        std::string password_;                  // Password to connect to the server
        int         serverSocket_;             // Server socket file descriptor
        struct sockaddr_in serverAddr_;        // Server address
        std::vector<int> clientSockets_;       // Client socket file descriptors
        // std::string getIPAddress();
};