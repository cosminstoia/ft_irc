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

void Server::printErrorExit(std::string const& msg, bool exitP)
{
    std::cerr << RED "Error: " RESET << msg << std::endl;
    if (exitP)
        exit(1);
}

void Server::printInfo(messageType type, const std::string &msg)
{
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm *tm_now = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%H:%M:%S");

    std::string typeStr;
    std::string color;

    switch (type)
    {
        case INFO:
            typeStr = "INFO";
            color = BLUE;
            break;
        case WARNING:
            typeStr = "WARNING";
            color = YELLOW;
            break;
        case CONNECTION:
            typeStr = "CONNECTION";
            color = GREEN;
            break;
        case DISCONNECTION:
            typeStr = "DISCONNECTION";
            color = RED;
            break;
        case PING:
            typeStr = "PING";
            color = GREEN;
            break;
        case PONG:
            typeStr = "PONG";
            color = GREEN;
            break;
        case CLIENT:
            typeStr = "CLIENT";
            color = BLUE;
            break;
        case SERVER:
            typeStr = "SERVER";
            color = BLUE;
            break;
        case TIMEOUT:
            typeStr = "TIMEOUT";
            color = RED;
            break;
        case SUCCESS:
            typeStr = "SUCCESS";
            color = GREEN;
            break;
        default:
            typeStr = "UNKNOWN";
            color = RESET;
            break;
    }
    std::cout << color << "[" << typeStr << "]"  << GRAY << " [" << oss.str()<< "] "
        << RESET << msg << std::endl;
}

void Server::setup()
{
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0)
        printErrorExit("Establishing socket...");
    
    printInfo(INFO, "Socket server has been created.");

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);

    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
        printErrorExit("Error binding socket.", true);
    
    printInfo(INFO, "Binding success...");

    if (listen(serverSocket_, 5) < 0)
        printErrorExit("Error listening...", true);
}


void Server::start() 
{
    std::cout << GREEN R"(░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀░░░░░░░░░░░░░        
░░░░░░░░░░░░░░░░░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀░░░░░░░░░░░░░       
░░░░░░░░░░░░░░░░░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░░░░░░░░░░░         
░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░█▀▀░█▀▄░░░
░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░▀▀█░░█░░█▀█░█▀▄░░█░░█▀▀░█░█░░░
░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░▀▀▀░▀▀░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░)" << std::endl;
    printInfo(INFO, "Server listening on ip:port " + std::to_string(port_));
    printInfo(INFO, "Waiting for conections...");
    fd_set readfds;
    while (true) // or sometign lieka bool 
    {
        FD_ZERO(&readfds);
        FD_SET(serverSocket_, &readfds);
        select(serverSocket_ + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(serverSocket_, &readfds))
        {
            acceptClient();
        }
    }
}

void Server::acceptClient() 
{
    int clientSocket = accept(serverSocket_, NULL, NULL);
    if (clientSocket < 0)
    {
        printErrorExit("Accept failed!", false);
        return;
    }
    // clientSocket.push_back(clientSocket);
    printInfo(INFO, "New client connected.");
}

void Server::handleClient(int clientSocket) 
{
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0)
    {
        close(clientSocket);
        printInfo(INFO, "Client disconnected");
        return;
    }
    // Basic message handling
    buffer[bytesRead] = '\0';
    printInfo(CLIENT, std::string("Received: ") + buffer);
}
