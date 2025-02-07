#include "Server.hpp"
#include "Client.hpp"

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
    
    printInfoToServer(INFO, "Server listening on ip:port " + std::to_string(port_));
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

void Server::start() 
{
    printInfoToServer(INFO, "Waiting for conections...");
    while (isRunning_)
    {
        if (poll(pollFds_.data(), pollFds_.size(), 0) == -1 && errno != EINTR)
        {
            printErrorExit("Poll failed!", false);
            continue;
        }
        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].revents & POLLIN)
            {
                if (pollFds_[i].fd == serverSocket_)
                    acceptClient(pollFds_);
                else
                    handleClient(pollFds_[i].fd);
            }
        }
    }
    printInfoToServer(INFO, "Server shutting down...");
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
        return printInfoToServer(WARNING, "Max clients reached, closing connection.");
    }
    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    pollFds_.push_back({ clientSocket, POLLIN, 0 });
    clients_.emplace(clientSocket, Client(clientIp, clientSocket));
}

void Server::handleClient(int clientSocket) 
{
    auto it = clients_.find(clientSocket);
    if (it != clients_.end()) 
    {
        Client& client = it->second;
        char buffer[MAX_CHARS];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK) 
            {
                printErrorExit("recv failed!", false);
                return;
            }
        }
        buffer[bytesRead] = '\0';
        client.appendToReceiveBuffer(buffer, bytesRead);
        connectClient(clientSocket);        
    }
}