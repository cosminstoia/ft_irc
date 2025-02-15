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
#include <csignal>
#include "Client.hpp"
#include "Channel.hpp"
#include "Macros.hpp"

enum messageType
{
    INFO,
    WARNING,
    CONNECTION,
    DISCONNECTION,
    CLIENT,
    SERVER,
    SUCCESS,
    BOT,
    ERROR,
    PING,
    PONG,
};

class Server
{
    private:
        int         port_;                      // Port number to listen on
        std::string password_;                  // Password to connect to the server
        int         serverSocket_;              // Server socket file descriptor
        struct sockaddr_in serverAddr_;         // Server address
        std::string serverIp_;                  // Server IP address
        std::vector<pollfd> pollFds_;           // Poll file descriptors
        std::map<std::string, Channel> channels_;
        std::map<int,Client> clients_;          // socket as key, Client as value
        bool        isRunning_;                 // Server running flag  

        // Command storage
        std::unordered_map<std::string, std::function<void(int, std::string const&)>> commandMap_;

        // Signal methods
        void        signalHandler(int signum); 
        void        setupSignalHandler();
        
        // Command methods
        void        setupCmds();
        void        cmdNick(int clientSocket, std::string const& params);
        void        cmdJoin(int clientSocket, std::string const& params);
        void        cmdUser(int clientSocket, std::string const& params);
        void        cmdPrivmsg(int clientSocket, std::string const& params);
        void        cmdQuit(int clientSocket, std::string const& params);
        bool        cmdPass(int clientSocket, std::string const& params);
        void        cmdPing(int clientSocket, std::string const& params);
        void        cmdPong(int clientSocket, std::string const& params);
        void        cmdTopic(int clientSocket, std::string const& params);
        void        cmdKick(int clientSocket, std::string const& params);
        void        cmdInvite(int clientSocket, std::string const& params);
        void        cmdMode(int clientSocket, std::string const& params);
        void        validateParams(int clientSocket, const std::string& params, const std::string& command);

        // Utils commands
        void        connectClient(int clientSocket);
        void        removeClient(int clientSocket);
        void        sendToClient(int clientSocket, std::string const& message);
        std::string getSPass() const;
        int         findClientByNick(const std::string& nick);

        // Pars Input
        bool        parseInput(Client& client, std::string const& message);
        bool        parseInitialInput(Client& client, const std::string command, std::string parameters);

    public:
        Server(int port, std::string const& password);
        ~Server();

        void       start();
        void       acceptClient(std::vector<pollfd>& pollFds_);
        void       handleClient(Client& client);

        static Server* instance; // static pointer to the server instance
        static void sSignalHandler(int signum); // static warp for signal handler

        void        welcomeClient(int clientSocket); 
        void        pingClients();

        //welcom ascii
        void        asciiArt();
};

// Print Functions
void       printInfoToServer(messageType type, std::string const& msg, bool exitP);
