#include "Server.hpp"

void Server::setupCmds()
{
    commandMap_["NICK"] = std::bind(&Server::cmdNick, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["USER"] = std::bind(&Server::cmdUser, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["JOIN"] = std::bind(&Server::cmdJoin, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PRIVMSG"] = std::bind(&Server::cmdPrivmsg, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["QUIT"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PING"] = std::bind(&Server::cmdPing, this, std::placeholders::_1, std::placeholders::_2);
}

void Server::cmdNick(int clientSocket, std::string const& params)
{
    (void)clientSocket;
    (void)params;
}

void Server::cmdUser(int clientSocket, std::string const& params)
{
    (void)clientSocket;
    (void)params;
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
        (void)clientSocket;
    (void)params;
}
void Server::cmdPrivmsg(int clientSocket, std::string const& params)
{
        (void)clientSocket;
    (void)params;
}

void Server::cmdQuit(int clientSocket, std::string const& params)
{
        (void)clientSocket;
    (void)params;
}

void Server::cmdPing(int clientSocket, std::string const& params)
{
        (void)clientSocket;
    (void)params;
}

