#pragma once

#include <string>
#include <sys/socket.h>
#include <sstream> 
#include <iostream>
#include <set>
#include <time.h>
#include "Macros.hpp"

class Client
{
    private:
        std::string             ipAddress_;
        int                     socket_;
        std::string             userName_;
        std::string             nickName_;
        bool                    loggedIn_;
        std::string             receiveBuffer_;
        std::string             clientPassword_;
        std::set<std::string>   joinedChannels_;
        int                     bytes_;
        bool                    awaitingPong_;

        std::chrono::system_clock::time_point lastPing_;
        std::chrono::system_clock::time_point lastPong_;

    public:
        Client();
        Client(const std::string& ipAddress, int socket);
        bool operator==(const Client& other) const;
        ~Client();

        void setLoggedIn(bool loggedIn) { loggedIn_ = loggedIn; }
        bool isLoggedIn() const { return loggedIn_; };
        
        // Getter and setter for receiveBuffer_
        void appendToReceiveBuffer(const std::string& data, int bytes) { receiveBuffer_ += data; bytes_ = bytes;}
        std::string& getReceiveBuffer() { return receiveBuffer_; }
        
        // Getters and setters for userName_ and nickName_
        void setUserName(const std::string& userName) { userName_ = userName; }
        void setNickName(const std::string& nickName) { nickName_ = nickName; }
        void setPassword(const std::string& password) { clientPassword_ = password; }
        void setBuffer(const std::string& buffer) { receiveBuffer_ = buffer; }
        const std::string& getUserName() const { return userName_; }
        const std::string& getNickName() const { return nickName_; }
        const std::string& getPassword() const { return clientPassword_; }
        const std::string& getBuffer() const { return receiveBuffer_; }
        const int& getSocket() const { return socket_; }
        const int& getBytes() const { return bytes_; }
        void joinChannel(const std::string& channel) { joinedChannels_.insert(channel); }
        
        // ping pong mechanism
        void updatePongReceived() { lastPong_ = std::chrono::system_clock::now(); awaitingPong_ = false; }
        void setPingSent() { lastPing_ = std::chrono::system_clock::now(); awaitingPong_ = true; }
        bool needsPing() const 
        {
            auto now = std::chrono::system_clock::now();
            return !awaitingPong_ && 
               std::chrono::duration_cast<std::chrono::seconds>(now - lastPing_).count() >= PING_INTERVAL;
        }
        bool hasTimedOut() const 
        {
            auto now = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(now - lastPong_).count() >= PING_TIMEOUT;
        }
};