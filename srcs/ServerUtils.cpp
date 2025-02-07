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
            sendToClient(clientSocket, "CAP * LS :\r\n");
            printInfoToServer(INFO, "Sent CAP LS to client");
            //return;
        }
        else if (message.substr(0, 4) == "PING")
        {
            printInfoToServer(PING, "Received PING from client");
            sendToClient(clientSocket, "PONG " + message.substr(5));
            printInfoToServer(PONG, "Sent PONG to client");
            client.updateActivity();
            // return;
        }
		std::string buffer = client.getBuffer();
        buffer.erase(0, pos + 2);
        client.setBuffer(buffer);
		if(!parseInput(client, message))
            return;
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
        printErrorExit("send failed!", false);
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
