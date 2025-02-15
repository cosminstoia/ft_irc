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
        printInfoToServer(ERROR, "Socket creation failed!", true);

    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        printInfoToServer(ERROR, "Failed to set socket options!", true);

    // only run in non-blocking mode. to be able to handle multiple clients
    if (fcntl(serverSocket_, F_SETFL, O_NONBLOCK) < 0) 
        printInfoToServer(ERROR, "Failed to set socket to non-blocking mode!", true);

    //configure server address
    //we use ipv4; for ipv6, we would use AF_INET6
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);
    
    //bind the socket to the server address
    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
        printInfoToServer(ERROR, "Bind failed!", true);

    //listen for connections
    if (listen(serverSocket_, MAX_Q_CLIENTS) < 0)
    printInfoToServer(ERROR, "Listen failed!", true);

    serverIp_ = inet_ntoa(serverAddr_.sin_addr);
    printInfoToServer(INFO, "Server listening on " + serverIp_ + ":" + std::to_string(port_), false);
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
    // asciiArt();
    printInfoToServer(INFO, "Waiting for conections...", false);
    while (isRunning_)
    {
        if (poll(pollFds_.data(), pollFds_.size(), 0) == -1 && errno != EINTR)
        {
            printInfoToServer(ERROR, "Poll failed!", false);
            continue;
        }
        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].revents & POLLIN)
            {
                if (pollFds_[i].fd == serverSocket_)
                    acceptClient(pollFds_);
                else
                {
                    auto it = clients_.find(pollFds_[i].fd);
                    if (it != clients_.end())
                        handleClient(it->second);
                    else
                        printInfoToServer(WARNING, "Client not found!", false);
                }
            }
        }
        pingClients();
    }
    printInfoToServer(INFO, "Server shutting down...", false);
}

void Server::acceptClient(std::vector<pollfd>& pollFds_) 
{
    sockaddr_in clientAddr = {};
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
        printInfoToServer(ERROR, "Accept failed!", false);
        return;
    }
    if (pollFds_.size() >= MAX_CLIENTS)
    {
        close(clientSocket);
        return printInfoToServer(WARNING, "Max clients reached, closing connection.", false);
    }
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    pollFds_.push_back({ clientSocket, POLLIN, 0 });
    clients_.emplace(clientSocket, Client(clientIp, clientSocket));
    printInfoToServer(CONNECTION, "Client connected from " + clientIp + " on socket " + std::to_string(clientSocket), false);
}

void Server::handleClient(Client& client) 
{
    char buffer[MAX_CHARS];
    ssize_t bytesRead = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0)
    {
        if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            std::cout << "No data to read.. revc faile.. why are we here ?" << std::endl;
            return; 
        }
        removeClient(client.getSocket());
        return;
    }
    buffer[bytesRead] = '\0';
    client.appendToReceiveBuffer(buffer, bytesRead);
    connectClient(client.getSocket());
}
