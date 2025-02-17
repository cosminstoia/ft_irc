#include "Server.hpp"
#include "Client.hpp"

static void parseMessage(const std::string& message, std::string& command, std::string& parameters)
{
    // std::cout << "[DEBUG] Raw Message: " << message << std::endl;
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
    // std::cout << "[DEBUG] Parsed command: [" << command << "], Parameters: [" << parameters << "]" << std::endl;
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
        else if (parameters == "END") 
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
                    client.setLoggedIn(true);
                    welcomeClient(client.getSocket());
                    return false;
                }
            }
            return true;
        }
        // Reject other commands until registered
        sendToClient(client.getSocket(), ERR_NOTREGISTERED(serverIp_));
        return false;
    }
    // reject pass after login
    if (command == "PASS")
    {
        sendToClient(client.getSocket(), "You are already logged in!");
        return false;
    }
    auto it = commandMap_.find(command);
    if (it != commandMap_.end())
    {
        it->second(client.getSocket(), parameters);
        return true;
    }
    return false;
}

bool Server::parseInitialInput(Client& client, const std::string command, std::string parameters) 
{
    if (command == "PASS") 
    {
        if (parameters.empty())
        {
            sendToClient(client.getSocket(), ERR_NEEDMOREPARAMS(serverIp_, "PASS"));
            return false;
        }
        else if (parameters == password_)
        {
            client.setPassword(parameters);
            printInfoToServer(INFO, "Authentication successful!", false);
            return true;
        }
        else
        {
            sendToClient(client.getSocket(), ERR_PASSWDMISMATCH(serverIp_));
            printInfoToServer(INFO, "Client introduced wrong password.", false);
            usleep(100000); // 100ms for message to be sent
            clients_.erase(client.getSocket());
            close(client.getSocket());
            return false;
        }
    }
    else if (command == "NICK")
    {
        if (parameters.empty())
        {
            sendToClient(client.getSocket(), ERR_NEEDMOREPARAMS(serverIp_, "NICK"));
            return false;
        }
        for (const auto& pair : clients_)
        {
            if (pair.second.getNickName() == parameters)
            {
                sendToClient(client.getSocket(), ERR_NICKNAMEINUSE(serverIp_, parameters));
                return false;
            }
        }
        client.setNickName(parameters);
        return true;
    }
    else if (command == "USER")
    {
        if (parameters.empty())
        {
            sendToClient(client.getSocket(), ERR_NEEDMOREPARAMS(serverIp_, "USER"));
            return false;
        }
        client.setUserName(parameters);
        return true;
    }
    return false;
}
