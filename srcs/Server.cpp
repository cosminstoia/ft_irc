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

    std::cout << "Listening on port " << port_ << "..." << std::endl;
}