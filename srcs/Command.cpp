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
    commandMap_["PASS"] = std::bind(&Server::cmdPass, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PONG"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
}

void Server::cmdNick(int clientSocket, std::string const& params)
{
    if (params.empty()) 
    {
        sendToClient(clientSocket, "461 NICK :Not enough parameters\r\n");
        return;
    }

    for (const auto& pair : clients_) 
    {
        if (pair.second.getNickName() == params) 
        {
            sendToClient(clientSocket, ERR_NICKNAMEINUSE(params));
            return;
        }
    }
    clients_[clientSocket].setNickName(params);
    if (!clients_[clientSocket].getUserName().empty() && !clients_[clientSocket].isLoggedIn()) 
        welcomeClient(clientSocket);
}

void Server::cmdUser(int clientSocket, std::string const& params)
{
    if (params.empty()) 
    {
        sendToClient(clientSocket, ERR_NEEDMOREPARAMS(params));
        return;
    }
    Client& client = clients_[clientSocket];
    client.setUserName(params);
    if (!client.getNickName().empty() && !client.isLoggedIn())
        welcomeClient(clientSocket);
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
    if (params.empty())
    {
        sendToClient(clientSocket, "No channel given");
        return;
    }
    sendToClient(clientSocket, "JOINED: " + params);
    printInfoToServer(INFO, "Client joined channel " + params, false);
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
    printInfoToServer(INFO, "PRIVMSG sent to " + recipient + ": " + message, false);
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
    removeClient(clientSocket);
}

bool Server::cmdPass(int clientSocket, std::string const& params)
{
    std::istringstream iss(params);
    std::string pass;
    iss >> pass;
    if (pass == password_ && !pass.empty())
    {
        clients_[clientSocket].setLoggedIn(true);
        printInfoToServer(INFO, "Authentication successful!", false);
        sendToClient(clientSocket, "Authentication successful!");
        return true;
    }
    else
    {
        printInfoToServer(ERROR, " Incorrect password! Disconnecting!", false);
        sendToClient(clientSocket, "Incorrect password! Disconnecting!");
        close(clientSocket);
        clients_.erase(clientSocket);
        return false;
    }
}

void Server::cmdPong(int clientSocket, std::string const& params)
{
    (void)params;
    auto it = clients_.find(clientSocket);
    if (it != clients_.end())
    {
        it->second.updatePongReceived();
        printInfoToServer(PONG, "Received PONG from client on socket " + std::to_string(clientSocket), false);
    }
}