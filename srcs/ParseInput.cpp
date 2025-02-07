#include "Server.hpp"
#include "Client.hpp"

void parseMessage(const std::string& message, std::string& prefix, std::string& command, std::string& parameters)
{
    size_t pos = 0;
    size_t end = message.size();

    prefix.clear();
    command.clear();
    parameters.clear();
    while (pos < end && (message[pos] == ' ' || message[pos] == '\t'))
        ++pos;
    if (pos < end && message[pos] == ':') 
    {
        ++pos;
        size_t prefixEnd = message.find(' ', pos);
        if (prefixEnd == std::string::npos)
            throw std::runtime_error("Error: Invalid message format (missing command).");
        prefix = message.substr(pos, prefixEnd - pos);
        pos = prefixEnd + 1;
    }
    while (pos < end && (message[pos] == ' ' || message[pos] == '\t'))
        ++pos;

    size_t commandEnd = message.find(' ', pos);
    if (commandEnd == std::string::npos)
        commandEnd = end;
    command = message.substr(pos, commandEnd - pos);
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
    pos = commandEnd;
    while (pos < end && (message[pos] == ' ' || message[pos] == '\t'))
        ++pos;
    if (pos < end) 
        parameters = message.substr(pos);
}

bool Server::parseInput(Client& client, const std::string& message) 
{
    std::string prefix;
    std::string command;
    std::string parameters;

    parseMessage(message, prefix, command, parameters);
    auto it = commandMap_.find(command);
    if (it != commandMap_.end()) 
    {
        it->second(client.getSocket(), parameters);
        return true;
    } 
    else 
    {
        printInfoToServer(ERROR, "Unknown command!");
        sendToClient(client.getSocket(), "Unknown command!");
        return false;
    }
}