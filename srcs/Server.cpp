#include "Server.hpp"
#include "Client.hpp"

Server::Server(int port, std::string const& password)
{
    port_ = port;
    password_ = password;
    serverSocket_ = -1;
    serverIp_ = "nop";
    isRunning_ = true;
    instance = this;
    bot_ = std::make_unique<Bot>();

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);

    if (!setupServerSocket(serverSocket_, serverAddr_))
        printInfoToServer(ERROR, "Failed to setup server socket!", true);

    serverIp_ = inet_ntoa(serverAddr_.sin_addr);
    pollFds_.push_back((struct pollfd){serverSocket_, POLLIN, 0});
    setupCmds();
    setupSignalHandler();
    printInfoToServer(INFO, "Server listening on " + serverIp_ + ":" + std::to_string(port_), false);
}

bool Server::setupServerSocket(int& serverSocket, const sockaddr_in& addr)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printInfoToServer(ERROR, "Socket creation failed!", true);
        return false;
    }
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printInfoToServer(ERROR, "Failed to set socket options!", true);
        return false;
    }
    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        printInfoToServer(ERROR, "Failed to set socket to non-blocking mode!", true);
        return false;
    }
    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printInfoToServer(ERROR, "Bind failed!", true);
        return false;
    }
    if (listen(serverSocket, MAX_Q_CLIENTS) < 0)
    {
        printInfoToServer(ERROR, "Listen failed!", true);
        return false;
    }
    return true;
}

Server::~Server()
{
    if (serverSocket_ != -1) 
    {
        cleanupExit();
    }
}

void Server::cleanupExit()
{
    for (auto& [fd, client] : clients_)
    {
        try 
        {
            send(fd, "Server shutting down...\r\n", 24, 0);
        }
        catch (std::exception const& e)
        {
            printInfoToServer(ERROR, "Failed to send message to client: " + std::string(e.what()), false);
        } 
    }
    clients_.clear();
    if (serverSocket_ != -1)
    {
        close(serverSocket_);
        serverSocket_ = -1;
    }
    for (auto const& clientFd : pollFds_)
    {
        if (clientFd.fd >= 0)
            close(clientFd.fd);
    }
    pollFds_.clear();
    bot_.reset();
    instance = nullptr; // reset the static instance
}

void Server::start() 
{
    asciiArt();
    printInfoToServer(INFO, "Waiting for conections...", false);
    while (isRunning_)
    {
        //poll check all fd and update pollFds_ with the events then deal with them if any 
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
                    handleClient(clients_[pollFds_[i].fd]);
            }
        }
        pingClients();
    }
    printInfoToServer(INFO, "Server shutting down...", false);
    cleanupExit();
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
    // if successful, add client to pollFds and create new clients
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
            std::cout << "Recv failed! and errno is EAGAIN or EWOULDBLOCK" << std::endl;
            return; 
        }
        removeClient(client.getSocket());
        return;
    }
    // if successful, append buffer to client's receive buffer and process it
    buffer[bytesRead] = '\0';
    client.appendToReceiveBuffer(buffer, bytesRead);
    connectClient(client.getSocket());
}

void Server::sendMsgToChannel(int clientSocket, const std::string& recipient, const std::string& message) 
{
    std::string fullMessage = ":Bot!bot@" + serverIp_ + " PRIVMSG " + recipient + " :" + message;
    sendToClient(clientSocket, fullMessage);
}
