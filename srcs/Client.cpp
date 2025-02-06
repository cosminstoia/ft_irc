#include "Client.hpp"

Client::Client()
    : ipAddress_(""),
      socket_(-1),
      userName_(""),
      nickName_(""),
      loggedIn_(false),
      receiveBuffer_("")
{}
Client::Client(const std::string& ipAddress, int socket)
    : ipAddress_(ipAddress),
      socket_(socket),
      userName_(""),
      nickName_(""),
      loggedIn_(false),
      receiveBuffer_(""),
      clientPassword_("")
{}

bool Client::operator==(const Client& other) const 
{
    return this->nickName_ == other.nickName_;
}

Client::~Client()
{}

void Client::sendMessage(const std::string& message) 
{
    std::ostringstream infoStream;
    infoStream << "Socket ID: " << socket_ << " Username: " << nickName_;
    send(socket_, message.c_str(), message.size(), 0);
}

