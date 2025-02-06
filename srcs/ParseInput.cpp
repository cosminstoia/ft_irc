#include "Server.hpp"
#include "Client.hpp"

void parseMessage(const std::string& message, std::string& prefix, std::string& command, std::vector<std::string>& parameters)
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
    while (pos < end) 
    {
        while (pos < end && (message[pos] == ' ' || message[pos] == '\t'))
            ++pos;
        if (pos >= end) 
            break;
        if (message[pos] == ':') 
        {
            ++pos;
            parameters.push_back(message.substr(pos));
            break;
        } else {
            size_t paramEnd = message.find(' ', pos);
            if (paramEnd == std::string::npos) {
                parameters.push_back(message.substr(pos));
                break;
            } else {
                parameters.push_back(message.substr(pos, paramEnd - pos));
                pos = paramEnd;
            }
        }
    }
}

void Server::parseInput(Client& client, const std::string& message) 
{
    std::string prefix;
    std::string command;
    std::vector<std::string> parameters;

    // Parse the message
    parseMessage(message, prefix, command, parameters);

    //std::cout << "-----input------: " << message << std::endl;
    //std::cout << "-----command-----: " << command << std::endl;
    //std::cout << "-----parameters--:";
	if (command == "PASS")
    {
		std::string password = parameters[1];
        if(!checkAuthentification(client.getSocket(), message))
            return;
            
    }
    else if(command == "JOIN")
    {
        cmdJoin(client.getSocket(), parameters[1]);
    }
    else if (message.substr(0, 4) == "NICK") 
    {
		client.setNickName(message.substr(5));
		if (client.getNickName().length() > 15) 
        {
			std::cout << "NickName too long\n";
			return;
		}
	} 
    else if (message.substr(0, 4) == "USER") 
    {
		client.setUserName(message.substr(message.find(':') + 1));
	}
    std::cout << "nickname:" << client.getNickName() << std::endl;
    // Handle the command
    // if (command == "NICK") {
    //     if (parameters.size() != 1) {
    //         throw std::runtime_error("Invalid number of parameters for NICK command.");
    //     }
    //     cmdNick(client, parameters[0]);
    // } else if (command == "USER") {
    //     if (parameters.size() < 4) {
    //         throw std::runtime_error("Invalid number of parameters for USER command.");
    //     }
    //     cmdUser(client, parameters);
    // } else {
    //     // Handle other commands
    // }
}