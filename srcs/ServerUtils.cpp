#include "Server.hpp"
#include "Client.hpp"

void Server::connectClient(int clientSocket) 
{
    Client& client = clients_[clientSocket];
    size_t pos = 0;
	while ((pos = client.getBuffer().find("\r\n")) != std::string::npos) 
    {
		std::string message = client.getBuffer().substr(0, pos);
		std::string buffer = client.getBuffer();
        client.setBuffer(buffer.erase(0, pos + 2));
		if(!parseInput(client, message))
            return;
	}
}

void Server::removeClient(int clientSocket) 
{
    auto it = clients_.find(clientSocket);
    if (it != clients_.end()) 
    {
        Client& client = it->second;
        close(clientSocket);
        for (size_t i = 0; i < pollFds_.size(); i++) 
        {
            if (pollFds_[i].fd == clientSocket) 
            {
                pollFds_.erase(pollFds_.begin() + i);
                break;
            }
        }
        clients_.erase(it);
        printInfoToServer(DISCONNECTION, "Client disconnected: " + client.getNickName(), false);
    }
}

void Server::sendToClient(int clientSocket, std::string const& message)
{
    std::string fullMessage = message + "\r\n";
    ssize_t bytesSend = send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
    if (bytesSend < 0)
    {
        std::cout << fullMessage << std::endl;
        printInfoToServer(ERROR, "Send function failed!", false);
    }
}

std::string Server::getSPass() const
{
    return password_;
}


void Server::asciiArt()
{
    std::cout << GREEN R"(░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀░░░░░░░░░░░░        
░░░░░░░░░░░░░░░░░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀░░░░░░░░░░░░       
░░░░░░░░░░░░░░░░░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░░░░░░░░░░         
░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░█▀▀░█▀▄░░
░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░▀▀█░░█░░█▀█░█▀▄░░█░░█▀▀░█░█░░
░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░▀▀▀░▀▀░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░)" << std::endl;
}

void Server::welcomeClient(int clientSocket)
{
    Client& client = clients_[clientSocket];
    std::string nick = client.getNickName();
    if (nick.empty())
    {
        sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, nick));
        return;
    }
    for (auto& pair : clients_)
    {
        if (pair.second.getNickName() == nick && pair.first != clientSocket)
        {
            sendToClient(clientSocket, ERR_NICKNAMEINUSE(serverIp_, nick));
            printInfoToServer(WARNING, "User already connected", false);
            client.setLoggedIn(true);
            return;
        }
    }
    sendToClient(clientSocket, RPL_WELCOME(serverIp_, nick));
}

void Server::pingClients()
{
    std::vector<int> timeoutClients;
    for (auto& pair : clients_) 
    {
        int clientSocket = pair.first;
        Client& client = pair.second;
        if (client.hasTimedOut())
        {
            timeoutClients.push_back(clientSocket);
            continue;
        }
        if (client.needsPing())
        {
            std::string pingMessage = "PING :" + std::to_string(std::time(nullptr)) + "\r\n";
            sendToClient(clientSocket, pingMessage);
            client.setPingSent();
            printInfoToServer(PING, "Sent PING to client on socket " + std::to_string(clientSocket), false);
        }
    }
    for (int socket : timeoutClients)
    {
        printInfoToServer(INFO, "Client on socket " + std::to_string(socket) + " timed out", false); // debug now
        removeClient(socket);
    }
}

int Server::findClientByNick(const std::string& nick)
{
    for (const auto& pair : clients_)
    {
        if (pair.second.getNickName() == nick)
            return pair.first;
    }
    return -1;
}