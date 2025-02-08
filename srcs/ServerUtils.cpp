#include "Server.hpp"
#include "Client.hpp"

void Server::connectClient(int clientSocket) 
{
    Client& client = clients_[clientSocket];
    size_t pos = 0;
	while ((pos = client.getBuffer().find("\r\n")) != std::string::npos) 
    {
		std::string message = client.getBuffer().substr(0, pos);
        std::cout << "-----input------: " << message << std::endl;
		if (message.substr(0, 6) == "CAP LS")
        {
            std::string respond = "CAP * LS :\r\n";
            send(clientSocket, respond.c_str(), respond.size(), 0);
            printInfoToServer(INFO, "Sent CAP LS to client");
        }
		std::string buffer = client.getBuffer();
        client.setBuffer(buffer.erase(0, pos + 2));
		if(!parseInput(client, message))
            return;
	}
}

// this need to be more robust adn also delte the other staf in clients
// for now if i close a client and try to reconnect it will tel em username is in use
void Server::removeClient(int clientSocket)
{
    for (size_t i = 0; i < pollFds_.size(); i++)
    {
        if (pollFds_[i].fd == clientSocket)
        {
            close(clientSocket);
            pollFds_.erase(pollFds_.begin() + i);
            printInfoToServer(DISCONNECTION, "Client disconnected!");
            break;
        }
    }
}

void Server::sendToClient(int clientSocket, std::string const& message)
{
    std::string fullMessage = message + "\r\n";
    ssize_t bytesSend = send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
    if (bytesSend < 0)
    {
        std::cout << fullMessage << std::endl;
        printErrorExit("send failed!", false);
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
        sendToClient(clientSocket, ERR_NEEDMOREPARAMS(nick));
        return;
    }
    for (auto& pair : clients_)
    {
        if (pair.second.getNickName() == nick && pair.first != clientSocket)
        {
            sendToClient(clientSocket, ERR_NICKNAMEINUSE(nick));
            return;
        }
    }
    sendToClient(clientSocket, RPL_WELCOME(nick));
    sendToClient(clientSocket, RPL_YOURHOST);
    sendToClient(clientSocket, RPL_CREATED);
    sendToClient(clientSocket, RPL_MYINFO);
}

void Server::pingClients()
{
    std::vector<int> timeoutClients;
    for (auto& pair : clients_) 
    {
        int clientSocket = pair.first;
        Client& client = pair.second;

        //only ping pong it if logged in
        if(!client.isLoggedIn())
            continue;
        
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
            printInfoToServer(PING, "Sent PING to client on socket " + std::to_string(clientSocket));
        }
    }

    for (int socket : timeoutClients)
    {
        printInfoToServer(INFO, "Client on socket " + std::to_string(socket) + " timed out"); // debug now
        removeClient(socket);
    }
}