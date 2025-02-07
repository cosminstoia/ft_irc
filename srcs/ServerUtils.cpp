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
            // sendToClient(clientSocket, "CAP * LS :");
            printInfoToServer(INFO, "Sent CAP LS to client");
            //return;
        }
		std::string buffer = client.getBuffer();
        client.setBuffer(buffer.erase(0, pos + 2));
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
    {
        std::cout << fullMessage << std::endl;
        printErrorExit("send failed!", false);
    }
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

void Server::sendPeriodicPings(int clientSocket) 
{
    auto now = std::chrono::system_clock::now();
    // If you're using a mutex for thread safety, uncomment the following line
    // std::lock_guard<std::mutex> lock(mutex_);

    if (now - clients_[clientSocket].lastPingtime_ >= std::chrono::seconds(60)) {
        clients_[clientSocket].lastPingtime_ = now;
    }

    // Convert to time_t if needed
    std::time_t epoch_seconds = std::chrono::system_clock::to_time_t(now);
    std::string pingMessage = "PING " + std::to_string(epoch_seconds) + "\r\n";

    if (!clients_[clientSocket].awaitingPong_) // Don't send another PING if one is already pending
    {
        sendToClient(clientSocket, pingMessage);
        clients_[clientSocket].updatePingtime();
        clients_[clientSocket].awaitingPong_ = true;
        printInfoToServer(PING, "Sent PING to client");
    }
}