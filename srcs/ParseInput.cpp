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
    // Always allow CAP commands
    if (command == "CAP") 
    {
        if (parameters == "LS 302") 
        {
            sendToClient(client.getSocket(), "CAP * LS :\r\n");
            return true;
        }
        if (parameters == "END") 
        {
            return true;
        }
    }
    // Handle registration commands if not logged in
    if (!client.isLoggedIn())
    {
        // Only allow registration commands
        if (command == "PASS" || command == "NICK" || command == "USER")
        {
            if (parseInitialInput(client, message))
            {
                // Check if we have all required information
                if (!client.getNickName().empty() && 
                    !client.getUserName().empty() && 
                    !client.getPassword().empty()) 
                {
                    client.setLoggedIn(true);
                    welcomeClient(client.getSocket());
                }
            }
            return true;
        }
        // Reject other commands until registered
        sendToClient(client.getSocket(), ERR_NOTREGISTERED);
        return false;
    }
    // Always handle PING/PONG regardless of registration
    if (command == "PING") 
    {
        std::string response = "PONG :" + parameters + "\r\n";
        sendToClient(client.getSocket(), response);
        return true;
    }
    else if (command == "PONG")
    {
        cmdPong(client.getSocket(), parameters);
        return true;
    }
    // Handle normal commands for registered clients
    auto it = commandMap_.find(command);
    if (it != commandMap_.end())
    {
        it->second(client.getSocket(), parameters);
        return true;
    }
    return false;
}

bool Server::parseInitialInput(Client& client, const std::string& message) 
{
    std::string prefix;
    std::string command;
    std::string parameters;

    parseMessage(message, prefix, command, parameters);

     if (command == "PASS") 
     {
        if (parameters == getSPass()) 
        {
            client.setPassword(parameters);
            return true;
        } 
        else 
        {
            sendToClient(client.getSocket(), ERR_PASSWDMISMATCH);
            return false;
        }
    }
    else if (command == "NICK")
    {
        for (const auto& pair : clients_) 
        {
            if (pair.second.getNickName() == parameters) 
            {
                sendToClient(client.getSocket(), ERR_NICKNAMEINUSE(parameters));
                return false;
            }
        }
        client.setNickName(parameters);
        return true;
    }
    else if (command == "USER")
    {
        client.setUserName(parameters);
        return true;
    }
    return false;
}