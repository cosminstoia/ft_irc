#pragma once

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
class Client 
{
    private:
        std::string receiveBuffer_;
        std::string IpAddress_;
        int         socketDescriptor_;
        std::string userName_;
        std::string nickName_;
        bool        loggedIn_;
    public:
        Client() : socketDescriptor_(-1), loggedIn_(false) {}
        Client(const std::string& IpAddress, int socketDescriptor);
        bool operator==(const Client& other) const;
        void sendMessage(const std::string& message) const;
        ~Client();

        void setLoggedIn(bool loggedIn) { loggedIn_ = loggedIn; }
        bool isLoggedIn() const { return loggedIn_; };
};