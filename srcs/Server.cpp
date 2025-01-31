#include "Server.hpp"

Server::Server(int port, std::string password) : port_(port), password_(password)
{
    serverSocket_ = -1; // init to -1 adn set up in setup()
}

Server::~Server()
{
    if (serverSocket_ != -1)
        close(serverSocket_);
}

void Server::printErrorExit(const std::string &msg)
{
    std::cerr << "Error: " << msg << std::endl;
    exit(1);
}

void Server::setup()
{
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0)
        printErrorExit("Error establishing socket...");
    

    std::cout << "Socket server has been created..." << std::endl;

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);

    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
        printErrorExit("Error binding socket...");
    
    std::cout << "Binding success..." << std::endl;

    if (listen(serverSocket_, 5) < 0)
        printErrorExit("Error listening...");
    
    std::cout << "__        _______ _     ____ ___  __  __ _____ \n";
    std::cout << "\\ \\      / / ____| |   / ___/ _ \\|  \\/  | ____|\n";
    std::cout << " \\ \\ /\\ / /|  _| | |  | |  | | | | |\\/| |  _|  \n";
    std::cout << "  \\ V  V / | |___| |__| |__| |_| | |  | | |___ \n";
    std::cout << "   \\_/\\_/  |_____|_____\\____\\___/|_|  |_|_____|\n\n";

    std::cout << "Listening on port " << port_ << "..." << std::endl;
}


void Server::start() 
{
    fd_set readfds;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket_, &readfds);

        select(serverSocket_ + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(serverSocket_, &readfds)) {
            acceptClient();
        }
    }
}

void Server::acceptClient() 
{
    int clientSocket = accept(serverSocket_, NULL, NULL);
    if (clientSocket < 0) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }
    // clientSocket.push_back(clientSocket);
    std::cout << "New client connected" << std::endl;
}

void Server::handleClient(int clientSocket) 
{
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    // Basic message handling
    buffer[bytesRead] = '\0';
    std::cout << "Received: " << buffer << std::endl;
}
