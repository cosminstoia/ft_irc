#include "Client.hpp"

Client::Client()
    : ipAddress_(""),
      socket_(-1),
      userName_(""),
      nickName_(""),
      loggedIn_(false),
      receiveBuffer_(""),
      bytes_(0),
      lastPing_(std::chrono::system_clock::now()),
      lastPong_(std::chrono::system_clock::now())
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
      lastPing_(std::chrono::system_clock::now()),
      lastPong_(std::chrono::system_clock::now())
{}

bool Client::operator==(const Client& other) const 
{
    return this->nickName_ == other.nickName_;
}

Client::~Client()
{}

// ping pong mechanism
void Client::updatePongReceived()
{
  lastPong_ = std::chrono::system_clock::now();
  awaitingPong_ = false;
}

void Client::setPingSent()
{
  lastPing_ = std::chrono::system_clock::now();
  awaitingPong_ = true;
}
bool Client::needsPing() const 
{
  auto now = std::chrono::system_clock::now();
  return !awaitingPong_ && std::chrono::duration_cast<std::chrono::seconds>(now - lastPing_).count() >= PING_INTERVAL;
}

bool Client::hasTimedOut() const 
{
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(now - lastPong_).count() >= PING_TIMEOUT;
}
