#pragma once

#include "Server.hpp"
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

class Server;

class Bot 
{
    private:
        using BotCommandFunction = void (Bot::*)(Server& server, int clientSocket, const std::string& recipient);
        std::map<std::string, BotCommandFunction> botCommand_;
    public:
        Bot();
        ~Bot();
        void helpCommand(Server& server, int clientSocket, const std::string& recipient);
        void motivationCommand(Server& server, int clientSocket, const std::string& recipient);
        void executeCommand(Server& server, int clientSocket, const std::string& recipient, const std::string& command);
        void botPeriodicBroadcast(Server& server, int clientSocket, const std::string& recipient);
};