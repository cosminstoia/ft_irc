#include "Server.hpp"

Server::Server(int port,std::string const& password)
{
    port_ = port;
    password_ = password;
    serverSocket_ = -1; // init to -1 adn set up in setup()
    serverIp_ = "nop";
    setupCmds();
    
    //setup server address
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0)
        printErrorExit("Socket creation failed!", true);

    // non blocking mode
    fcntl(serverSocket_, F_SETFL, O_NONBLOCK); // check adn close fd if
    printInfo(INFO, "Socket server created.");

    //coonfigure server address
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);
    
    //bind the socket to the server address
    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
    {
        close(serverSocket_),
        printErrorExit("Bind failed!", true);
    }

    printInfo(INFO, "Socket binded to port " + std::to_string(port_));

    //listen for connections
    if (listen(serverSocket_, 5) < 0)
        printErrorExit("Listen failed!", true);
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

void Server::printInfo(messageType type, std::string const& msg)
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
    while (true) // or sometign like bool isRunning = true; while (isRunning)
    {
        int ret = poll(pollFds_.data(), pollFds_.size(), -1);
        if (ret < 0)
        {
            printErrorExit("Poll failed!", false);
            continue;
        }
        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].revents & POLLIN)
            {
                if (pollFds_[i].fd == serverSocket_)
                {
                    acceptClient(pollFds_);
                }
                else
                {
                    handleClient(pollFds_[i].fd);
                }
            }
        }
    }
}

void Server::acceptClient(std::vector<pollfd>& pollFds_) 
{
    int clientSocket = accept(serverSocket_, NULL, NULL);
    if (clientSocket < 0)
    {
        printErrorExit("Accept failed!", false);
        return;
    }

    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    
    pollfd clientPollFd = {clientSocket, POLLIN, 0};
    pollFds_.push_back(clientPollFd);

    printInfo(CONNECTION, "New client connected.");

}

void Server::handleClient(int clientSocket) 
{
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0)
    {
        close(clientSocket);
        printInfo(INFO, "Client disconnected!");

        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].fd == clientSocket)
            {
                pollFds_.erase(pollFds_.begin() + i);
                break;
            }
        }
        return;
    }

    buffer[bytesRead] = '\0';
    std::string message(buffer);
    printInfo(CLIENT, std::string("Received: ") + message);

    if (isCommand(message))
    {
        std::istringstream iss(message);
        std::string command, params;

        iss >> command;

        if (!command.empty() && command[0] == '/')
            command = command.erase(0, 1);

        std::getline(iss, params);

        auto it = commandMap_.find(command);

        if (it != commandMap_.end())
        {
            it->second(clientSocket, params);
        }
        else
        {
            sendToClient(clientSocket, "Unknown command: " + command);
        }
    }
}


void Server::sendToClient(int clientSocket, std::string const& message)
{
    std::string fullMessage = message + "\r\n";
    send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
}

bool Server::isCommand(std::string const& input)
{
    if (input.empty())
        return false;

    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (!command.empty() && command[0] == '/') // 
        command = command.erase(0, 1);

    return commandMap_.find(command) != commandMap_.end();
}