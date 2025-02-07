#include "Client.hpp"

Client::Client()
    : ipAddress_(""),
      socket_(-1),
      userName_(""),
      nickName_(""),
      loggedIn_(false),
      receiveBuffer_(""),
      bytes_(0),
      lastActivity_(),
      lastPingtime_()
{}
Client::Client(const std::string& ipAddress, int socket)
    : ipAddress_(ipAddress),
      socket_(socket),
      userName_(""),
      nickName_(""),
      loggedIn_(false),
      receiveBuffer_(""),
      clientPassword_(""),
      bytes_(0),
      lastActivity_(),
      lastPingtime_()
{}

bool Client::operator==(const Client& other) const 
{
    return this->nickName_ == other.nickName_;
}

Client::~Client()
{}
