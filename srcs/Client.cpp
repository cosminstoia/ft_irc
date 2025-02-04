#include "Client.hpp"

Client::Client(const std::string& IpAddress, int socketDescriptor)
    : IpAddress_(IpAddress),
    socketDescriptor_(socketDescriptor)
    //loggedIn_(false)
{}

bool Client::operator==(const Client& other) const 
{
    return this->nickName_ == other.nickName_;
}

Client::~Client()
{
}

void Client::sendMessage(const std::string& message) const 
{
    std::ostringstream infoStream;
    infoStream << "Socket ID: " << socketDescriptor_ << " Username: " << nickName_;
    send(socketDescriptor_, message.c_str(), message.size(), 0);
}