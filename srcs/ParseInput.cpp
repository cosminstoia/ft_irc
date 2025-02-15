#include "Server.hpp"
#include "Client.hpp"

static void parseMessage(const std::string& message, std::string& command, std::string& parameters)
{
    size_t pos = 0;
    size_t end = message.size();

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
    std::string command;
    std::string parameters;
    parseMessage(message, command, parameters);
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
    // Handle PING and PONG commands regardless of login status
    if (command == "PING") 
    {
        cmdPing(client.getSocket(), parameters);
        return true;
    }
    else if (command == "PONG")
    {
        cmdPong(client.getSocket(), parameters);
        return true;
    }
    // Handle registration commands if not logged in
    if (!client.isLoggedIn())
    {
        // Only allow registration commands
        if (command == "PASS" || command == "NICK" || command == "USER")
        {
            if (parseInitialInput(client, command, parameters))
            {
                // Check if we have all required information
                if (!client.getNickName().empty() && 
                    !client.getUserName().empty() && 
                    !client.getPassword().empty())
                {
                    welcomeClient(client.getSocket());
                    client.setLoggedIn(true);
                    return false;
                }
            }
            return true;
        }
        // Reject other commands until registered
        sendToClient(client.getSocket(), ERR_NOTREGISTERED(serverIp_));
        return false;
    }
    auto it = commandMap_.find(command);
    if (it != commandMap_.end())
    {
        it->second(client.getSocket(), parameters);
        return true;
    }
    sendToClient(client.getSocket(), ERR_UNKNOWNCOMMAND(serverIp_, command));
    return false;
}

bool Server::parseInitialInput(Client& client, const std::string command, std::string parameters) 
{
    if (command == "PASS") 
    {
        if (parameters == getSPass() && !getSPass().empty()) 
        client.setPassword(parameters);
        else 
        {
            sendToClient(client.getSocket(), ERR_PASSWDMISMATCH(serverIp_));
            return false;
        }
    }
    else if (command == "NICK")
    {
        if (client.getNickName() != parameters) 
        client.setNickName(parameters);
        else
        {
            sendToClient(client.getSocket(), ERR_NICKNAMEINUSE(serverIp_, parameters));
            return false;;
        }
    }
    else if (command == "USER")
    {
        if (client.getUserName() != parameters) 
        client.setUserName(parameters);
        else
        return false;;
    }
    return true;
}
