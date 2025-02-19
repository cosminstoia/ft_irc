#include "Server.hpp"
#include "Client.hpp"

static void parseMessage(const std::string& message, std::string& command, std::string& parameters)
{
    size_t pos = 0;
    size_t end = message.size();

    command.clear();
    parameters.clear();

    if (message.empty()) // skip empty messages
        return;
    while (pos < end && (message[pos] == ' ' || message[pos] == '\t')) //skip leading spaces
        ++pos;

    // handle prefix wiht : 
    if (pos < end && message[pos] == ':')
    {
        ++pos;
        size_t prefixEnd = message.find(' ', pos);
        if (prefixEnd == std::string::npos)
            return; // ignore incomplete messages
        pos = prefixEnd + 1;
    }
    while (pos < end && (message[pos] == ' ' || message[pos] == '\t')) // skip spaces after prefix
        ++pos;

    // parse command
    size_t commandEnd = message.find(' ', pos);
    if (commandEnd == std::string::npos)
        commandEnd = end;
    command = message.substr(pos, commandEnd - pos);
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    // skip spaces after command
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

    // 1. Always allow CAP commands
    if (command == "CAP") 
    {
        if (parameters == "LS 302") 
        {
            sendToClient(client.getSocket(), "CAP * LS :");
            return true;
        }
        else if (parameters == "END") 
            return true;
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

    // 2. If not logged in, only allow registration commands
    if (!client.isLoggedIn())
    {
        if (command == "PASS" || command == "NICK" || command == "USER")
        {
            bool result = parseInitialInput(client, command, parameters);
            if (result && 
            !client.getNickName().empty() && 
            !client.getUserName().empty() && 
            !client.getPassword().empty())
            {
                client.setLoggedIn(true);
                welcomeClient(client.getSocket());
                return true;
            }
            return result;
        }
        sendToClient(client.getSocket(), ERR_NOTREGISTERED(serverIp_));
        return false;
    }
    // 3. Handle regular commands for logged-in users
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
    else if (it == commandMap_.end())
        printInfoToServer(ERROR, "Client " + client.getNickName() + " sent unknown command: " + command, false);
    return false;
}

bool Server::parseInitialInput(Client& client, const std::string command, std::string parameters) 
{
    // we only allow PASS, NICK, and USER commands before login
    if (command == "PASS") 
    {
        if (parameters.empty())
        {
            sendToClient(client.getSocket(), ERR_NEEDMOREPARAMS(serverIp_, "PASS"));
            return false;
        }
        if (parameters != password_)
        {
            sendToClient(client.getSocket(), ERR_PASSWDMISMATCH(serverIp_));
            printInfoToServer(INFO, "Client introduced wrong password!", false);
            usleep(100000);
            clients_.erase(client.getSocket());
            close(client.getSocket());
            return false;
        }
        client.setPassword(parameters);
        printInfoToServer(INFO, "Client introduced correct password!", false);
        return true;
    }
    if (command == "NICK")
    {
        cmdNick(client.getSocket(), parameters);
        return true;
    }
    if (command == "USER")
    {
        // Split USER parameters: username hostname servername :realname
        std::istringstream iss(parameters);
        std::string username, hostname, servername, realname;
        if (!(iss >> username >> hostname >> servername))
        {
            sendToClient(client.getSocket(), ERR_NEEDMOREPARAMS(serverIp_, "USER"));
            return false;
        }
        // get realname , its all after the first colon
        size_t pos = parameters.find(':');
        if (pos != std::string::npos)
            realname = parameters.substr(pos + 1);
        else
            realname = username; // default to username if no realname provided
        client.setUserName(username);
        client.setRealName(realname);
        return true;
    }
    return false;
}
