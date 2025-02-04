#include "Server.hpp"

void Server::setupCmds()
{
    commandMap_["NICK"] = std::bind(&Server::cmdNick, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["USER"] = std::bind(&Server::cmdUser, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["JOIN"] = std::bind(&Server::cmdJoin, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PRIVMSG"] = std::bind(&Server::cmdPrivmsg, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["QUIT"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["TOPIC"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["KICK"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
}

void Server::cmdNick(int clientSocket, std::string const& params)
{
    if (params.empty())
    {
        sendToClient(clientSocket, "No nickname provided!");
        return;
    }
    //clients_[clientSocket] = params;
    sendToClient(clientSocket, "NICKNAME_SET: " + params);
    printInfo(CLIENT, "Nickname set to: " + params, clientSocket);
}

void Server::cmdUser(int clientSocket, std::string const& params)
{
    if (params.empty())
    {
        sendToClient(clientSocket, "No username given");
        return;
    }
    sendToClient(clientSocket, "USERNAME_SET: " + params);
    printInfo(INFO, "Username set to: " + params, clientSocket);
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
    if (params.empty())
    {
        sendToClient(clientSocket, "No channel given");
        return;
    }
    sendToClient(clientSocket, "JOINED: " + params);
    printInfo(INFO, "Client joined channel " + params, clientSocket);
}
void Server::cmdPrivmsg(int clientSocket, std::string const& params)
{
    if (params.empty())
    {
        sendToClient(clientSocket, "No message provided");
        return;
    }
    size_t sp = params.find(' ');
    if (sp == std::string::npos)
    {
        sendToClient(clientSocket, "No recipient provided");
        return;
    }
    std::string recipient = params.substr(0, sp);
    std::string message = params.substr(sp + 1);
    sendToClient(clientSocket, "PRIVMSG sent to " + recipient + ": " + message);
    printInfo(CLIENT, "PRIVMSG sent to " + recipient + ": " + message, clientSocket);
}

void Server::cmdQuit(int clientSocket, std::string const& params)
{
    (void)params;
    close(clientSocket);
    for (size_t i = 0; i < pollFds_.size(); i++)
    {
        if (pollFds_[i].fd == clientSocket)
        {
            pollFds_.erase(pollFds_.begin() + i);
            break;
        }
    }
    printInfo(DISCONNECTION, "Client disconnected!", clientSocket);
}

