#include "Server.hpp"
#include "Client.hpp"

void Server::connectClient(int clientSocket) 
{
    Client& client = clients_[clientSocket];
    size_t pos = 0;
	while ((pos = client.getBuffer().find("\r\n")) != std::string::npos) 
    {
		std::string message = client.getBuffer().substr(0, pos);
		if (message.substr(0, 6) == "CAP LS") 
			client.sendMessage("CAP * LS :\r\n");
		std::string buffer = client.getBuffer();
        buffer.erase(0, pos + 2);
        client.setBuffer(buffer);
		parseInput(client, message);
	}
}

void Server::removeClient(int clientSocket)
{
    for (size_t i = 0; i < pollFds_.size(); i++)
    {
        if (pollFds_[i].fd == clientSocket)
        {
            close(clientSocket);
            pollFds_.erase(pollFds_.begin() + i);
            printInfo(DISCONNECTION, "Client disconnected!");
            break;
        }
    }
}

void Server::sendToClient(int clientSocket, std::string const& message)
{
    (void)clientSocket;
    std::string fullMessage = message + "\r\n";
    std::cout << fullMessage;
    send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
}

bool Server::checkAuthentification(int clientSocket, std::string const& msg)
{
    std::istringstream iss(msg);
    std::string cmd, pass;
    iss >> cmd >> pass;
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
        return false;
    }
    sendToClient(clientSocket, "Unknown command! Enter PASS first!");
    return false;
}