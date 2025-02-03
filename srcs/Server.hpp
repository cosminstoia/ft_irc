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
#include <fcntl.h>
#include <map>
#include <set>
#include <unordered_map>

#include "Client.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"

#define BUFF 1024
#define MAX_CLIENTS 10



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
        Server(int port, std::string const& password);
        ~Server();

        void       start();
        void       printErrorExit(std::string const& msg, bool exitP = false);
        void       printInfo(messageType type, std::string const& msg);
        void       acceptClient(std::vector<pollfd>& pollFds_);
        void       handleClient(int clientSocket);

    private:
        int         port_;                      // Port number to listen on
        std::string password_;                  // Password to connect to the server
        int         serverSocket_;             // Server socket file descriptor
        struct sockaddr_in serverAddr_;        // Server address
        std::vector<int> clientSockets_;       // Client socket file descriptors
        std::string serverIp_;                 // Server IP address
        std::vector<pollfd> pollFds_;           // Poll file descriptors
        std::map<int, std::string> clients_; // map client sockets

        std::unordered_map<std::string, std::function<void(int, std::string const&)>> commandMap_;

        //methods
        void        sendToClient(int clientSocket, std::string const& message);

        // Commands + we use a unique prefix for each command 
        // (not map because we need fast access and don't care about order)
        bool        isCommand(std::string const& message);
        void        setupCmds();
        void        cmdNick(int clientSocket, std::string const& params);
        void        cmdJoin(int clientSocket, std::string const& params);
        void        cmdUser(int clientSocket, std::string const& params);
        void        cmdPrivmsg(int clientSocket, std::string const& params);
        void        cmdQuit(int clientSocket, std::string const& params);
        void        cmdPing(int clientSocket, std::string const& params);
};