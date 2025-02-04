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
        //bool        loggedIn_;
    public:
        Client(const std::string& address, int socketDescriptor);
        bool operator==(const Client& other) const;
        void sendMessage(const std::string& message) const;
        ~Client();
};