#include "Server.hpp"


// static member initialization
Server* Server::instance = nullptr;

Server::Server(int port, std::string const& password)
{
    port_ = port;
    password_ = password;
    serverSocket_ = -1;
    serverIp_ = "nop";
    isRunning_ = true;
    instance = this; // to be able to use in signal handler

    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);     //setup server address
    if (serverSocket_ < 0)
        printErrorExit("Socket creation failed!", true);

    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        printErrorExit("Failed to set socket options!", true);

    // only run in non-blocking mode. to be able to handle multiple clients
    if (fcntl(serverSocket_, F_SETFL, O_NONBLOCK) < 0) 
        printErrorExit("Failed to set socket to non-blocking mode!", true);

    //configure server address
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);
    
    //bind the socket to the server address
    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
        printErrorExit("Bind failed!", true);

    //listen for connections
    if (listen(serverSocket_, MAX_Q_CLIENTS) < 0)
        printErrorExit("Listen failed!", true);
    
    printInfo(INFO, "Server listening on ip:port " + std::to_string(port_));
    pollFds_.push_back((struct pollfd){serverSocket_, POLLIN, 0});
    setupCmds();
    setupSignalHandler();
}

Server::~Server()
{
    if (serverSocket_ != -1)
        close(serverSocket_);
    
    for (auto const& clientFd : pollFds_)
    {
        if (clientFd.fd >= 0)
            close(clientFd.fd);
    }
    instance = nullptr; // reset the static instance
}

void Server::printErrorExit(std::string const& msg, bool exitP)
{
    std::cerr << RED "Error: " RESET << msg << std::endl;
    if (exitP)
        exit(1);
}

void Server::printInfo(messageType type, std::string const& msg, int clientSocket)
{
    (void)clientSocket;
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm *tm_now = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%H:%M:%S");

    std::string typeStr;
    std::string color;
    switch (type)
    {
        case INFO:  typeStr = "INFO";   color = BLUE;   break;
        case WARNING:   typeStr = "WARNING";    color = YELLOW; break;
        case CONNECTION:typeStr = "CONNECTION"; color = GREEN;  break;
        case DISCONNECTION: typeStr = "DISCONNECTION"; color = RED; break;
        case BOT:  typeStr = "BOT";   color = GRAY;   break;
        default:        typeStr = "UNKNOWN";    color = RESET; break;
    }

    std::cout << color << "[" << typeStr << "]" << GRAY " [" << oss.str() << "] "
                << RESET << msg << std::endl;
}

void Server::start() 
{
    std::cout << GREEN R"(░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀░░░░░░░░░░░░        
░░░░░░░░░░░░░░░░░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀░░░░░░░░░░░░       
░░░░░░░░░░░░░░░░░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░░░░░░░░░░         
░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░█▀▀░█▀▄░░
░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░▀▀█░░█░░█▀█░█▀▄░░█░░█▀▀░█░█░░
░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░▀▀▀░▀▀░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░)" << std::endl;
    printInfo(INFO, "Waiting for conections...");
    while (isRunning_)
    {
        // poll retrun -1 when interrupted by signal, when this happens, errno is set to EINTR
        // we dont want to print error in this case so we check for errno not EINTR
        if (poll(pollFds_.data(), pollFds_.size(), 0) == -1 && errno != EINTR)
        {
            printErrorExit("Poll failed!", false);
            continue;
        }
        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].revents & POLLIN)
            {
                if (pollFds_[i].fd == serverSocket_){
                    acceptClient(pollFds_);}
                else
                    handleClient(pollFds_[i].fd);
            }
        }
    }
    printInfo(INFO, "Server shutting down...");
}

void Server::acceptClient(std::vector<pollfd>& pollFds_) 
{
    sockaddr_in clientAddr = {};
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
        printErrorExit("Accept failed!", false);
        return;
    }

    if (pollFds_.size() >= MAX_CLIENTS)
    {
        close(clientSocket);
        return printInfo(WARNING, "Max clients reached, closing connection.");
    }

    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);

    pollFds_.push_back({ clientSocket, POLLIN, 0 });
    clients_.emplace(clientSocket, Client(clientIp, clientSocket));

    sendToClient(clientSocket, "Please enter password:");
    std::cout << "1111" << std::endl;
}

bool Server::checkAuthentification(int clientSocket, std::string const& msg)
{
    std::istringstream iss(msg);
    std::string cmd, pass;
    iss >> cmd >> pass;
    std::cout << "2222222" << std::endl;

    if (cmd == "PASS")
    {
        if (pass == password_)
        {
            clients_[clientSocket].setLoggedIn(true);
            sendToClient(clientSocket, "Authentication successful!");
            return true;
        }
        else
        {
            sendToClient(clientSocket, "Incorrect password! Disconnecting!");
            close(clientSocket);
            clients_.erase(clientSocket);
            std::cout << "3333333" << std::endl;
            return false;
        }
    }

    sendToClient(clientSocket, "Unknown command! Enter PASS first!");
    return false;
}

void Server::handleClient(int clientSocket) 
{
    char buffer[MAX_CHARS];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0)
    {
        removeClient(clientSocket); 
        return;
    }

    std::string message(buffer, bytesRead);
    printInfo(CLIENT, "Received: " + message);


    if (!clients_.count(clientSocket))
    {
        printInfo(WARNING, "Client not found. Disconnecting!", clientSocket);
        removeClient(clientSocket);
        return;
    }

    if (!clients_[clientSocket].isLoggedIn())
    {
        if (!checkAuthentification(clientSocket, message))
            return;
    }
    else
        processMessage(clientSocket, message);
}

void Server::processMessage(int clientSocket, std::string const& message)
{
    printInfo(CLIENT, "Received: " + message);

    auto it = clients_.find(clientSocket);
    if (it == clients_.end())
    {
        printInfo(WARNING, "Client not found. Disconnecting!", clientSocket);
        removeClient(clientSocket);
        return;
    }
    Client& client = it->second;

     // If not logged in, force authentication
    if (!client.isLoggedIn()) {
        if (!checkAuthentification(clientSocket, message)) {
            return;  // If authentication fails, stop processing
        }
    }
    
    // Process commands only after authentication
    std::istringstream iss(message);
    std::string command, params;
    iss >> command;
    std::getline(iss, params);

    if (!command.empty() && command[0] == '/')
        command = command.erase(0, 1);

    auto itCmd = commandMap_.find(command);
    if (itCmd != commandMap_.end())
        itCmd->second(clientSocket, params);
    else
        sendToClient(clientSocket, "Unknown command: " + command);
}

void Server::removeClient(int clientSocket)
{
    for (size_t i = 0; i < pollFds_.size(); i++)
    {
        if (pollFds_[i].fd == clientSocket)
        {
            close(clientSocket);
            pollFds_.erase(pollFds_.begin() + i);
            printInfo(DISCONNECTION, "Client disconnected!", clientSocket);
            break;
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

void Server::signalHandler(int signum)
{
    if (signum == SIGINT && instance)    
        instance->isRunning_ = false;
}

void Server::sSignalHandler(int signum)
{
    if (instance)
        instance->signalHandler(signum);
}

void Server::setupSignalHandler()
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = Server::sSignalHandler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1)
        printErrorExit("Signal handler failed!", true);
}