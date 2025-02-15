#include "Server.hpp"

void Server::setupCmds()
{
    commandMap_["NICK"] = std::bind(&Server::cmdNick, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["USER"] = std::bind(&Server::cmdUser, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["JOIN"] = std::bind(&Server::cmdJoin, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PRIVMSG"] = std::bind(&Server::cmdPrivmsg, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["QUIT"] = std::bind(&Server::cmdQuit, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["TOPIC"] = std::bind(&Server::cmdTopic, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["KICK"] = std::bind(&Server::cmdKick, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PASS"] = std::bind(&Server::cmdPass, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PING"] = std::bind(&Server::cmdPing, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PONG"] = std::bind(&Server::cmdPong, this, std::placeholders::_1, std::placeholders::_2);
}

void Server::validateParams(int clientSocket, const std::string& params, const std::string& command)
{
    if (params.empty())
    {
        sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, command));
        return;
    }
}

void Server::cmdNick(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "NICK");
    for (const auto& pair : clients_) 
    {
        if (pair.second.getNickName() == params) 
        {
            sendToClient(clientSocket, ERR_NICKNAMEINUSE(serverIp_, params));
            return;
        }
    }
    clients_[clientSocket].setNickName(params);
    if (!clients_[clientSocket].getUserName().empty() && !clients_[clientSocket].isLoggedIn()) 
        welcomeClient(clientSocket);
}

void Server::cmdUser(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "USER");
    Client& client = clients_[clientSocket];
    client.setUserName(params);
    if (!client.getNickName().empty() && !client.isLoggedIn())
        welcomeClient(clientSocket);
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "JOIN");
    std::string channelName = params;
    if (channels_.find(channelName) == channels_.end())
    {
        channels_.emplace(channelName, Channel(channelName)); // Explicitly construct the channel
        printInfoToServer(INFO, "Channel " + channelName + " created", false);
    }
    Channel& channel = channels_[channelName];
    if (channel.isInviteOnly() && !channel.isInvited(clientSocket))
    {
        sendToClient(clientSocket, ERR_INVITEONLYCHAN(serverIp_, channelName));
        return;
    }
    if (channel.isFull())
    {
        sendToClient(clientSocket, ERR_CHANNELISFULL(serverIp_, channelName));
        return;
    }
    channel.addMember(clientSocket);
    sendToClient(clientSocket, "JOINED: " + params);
    printInfoToServer(INFO, "Client joined channel " + channelName, false);
}

void Server::cmdPrivmsg(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "PRIVMSG");
    size_t sp = params.find(' ');
    if (sp == std::string::npos)
    {
        sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "PRIVMSG"));
        return;
    }
    std::string recipient = params.substr(0, sp);
    std::string message = params.substr(sp + 1);
    if (recipient[1] == '#')
    {
        if (channels_.find(recipient) == channels_.end())
        {
            sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, recipient));
            return;
        }

        Channel& channel = channels_[recipient];
        if (!channel.isMember(clientSocket))
        {
            sendToClient(clientSocket, ERR_CANNOTSENDTOCHAN(serverIp_, recipient));
            return;
        }
        // Broadcast the message to all members of the channel except the sender
        std::string senderNick = clients_[clientSocket].getNickName();
        std::string privmsg = ":" + senderNick + " PRIVMSG " + recipient + " :" + message + "\r\n";
        for (int memberSocket : channel.getMembers())
        {
            if (memberSocket != clientSocket)
                sendToClient(memberSocket, privmsg);
        }
        printInfoToServer(INFO, "PRIVMSG sent to " + recipient + ": " + message, false);
    }
    else if (!message.empty() && message[1] == '!')
    {
        printInfoToServer(INFO, "PRIVMSG sent to " + recipient + ": " + message, false);
        bot_->executeCommand(*this, clientSocket, recipient, message.substr(1));
    }
    else
    {

        int recipientSocket = findClientByNick(recipient);
        if (recipientSocket == -1)
        {
            sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, recipient));
            return;
        }
        std::string senderNick = clients_[clientSocket].getNickName();
        std::string privmsg = ":" + senderNick + " PRIVMSG " + recipient + " :" + message + "\r\n";
        sendToClient(recipientSocket, privmsg);
        printInfoToServer(INFO, "PRIVMSG sent to " + recipient + ": " + message, false);
    }
}

void Server::cmdQuit(int clientSocket, std::string const& params)
{
    (void)params;
    close(clientSocket);
    for (size_t i = 0; i < pollFds_.size(); i++)
    {
        if (pollFds_[i].fd == clientSocket)
        {
            pollFds_.erase(pollFds_.begin() + i);
            break;
        }
    }
    removeClient(clientSocket);
}

bool Server::cmdPass(int clientSocket, std::string const& params)
{
    std::istringstream iss(params);
    std::string pass;
    iss >> pass;
    if (pass == password_ && !pass.empty())
    {
        clients_[clientSocket].setLoggedIn(true);
        printInfoToServer(INFO, "Authentication successful!", false);
        sendToClient(clientSocket, "Authentication successful!");
        return true;
    }
    else
    {
        printInfoToServer(ERROR, " Incorrect password! Disconnecting!", false);
        sendToClient(clientSocket, "Incorrect password! Disconnecting!");
        close(clientSocket);
        clients_.erase(clientSocket);
        return false;
    }
}

void Server::cmdPing(int clientSocket, std::string const& params)
{
    std::string response = "PONG :" + serverIp_ + " :" + params + "\r\n";
    sendToClient(clientSocket, response);
}

void Server::cmdPong(int clientSocket, std::string const& params)
{
    (void)params;
    auto it = clients_.find(clientSocket);
    if (it != clients_.end())
    {
        it->second.updatePongReceived();
        printInfoToServer(PONG, "Received PONG from client on socket " + std::to_string(clientSocket), false);
    }
}

void Server::cmdTopic(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "TOPIC");
    std::istringstream iss(params);
    std::string channelName, topic;
    iss >> channelName;
    std::getline(iss, topic);
    topic.erase(0, topic.find_first_not_of(' '));

    if (channels_.find(channelName) == channels_.end())
    {
        sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));
        return;
    }
    Channel& channel = channels_[channelName];
    Client& client = clients_[clientSocket];
    if (topic.empty())
    {
        std::string currentTopic = channel.getTopic();
        if (currentTopic.empty())
            sendToClient(clientSocket, RPL_NOTOPIC(serverIp_, client.getNickName(), channelName));
        else
            sendToClient(clientSocket, RPL_TOPIC(serverIp_, client.getNickName(), channelName, currentTopic));
        return;
    }
    if (channel.isTopicRestricted() && !channel.isOperator(clientSocket))
    {
        sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, channelName));
        return;
    }
    channel.setTopic(topic);
    sendToClient(clientSocket, RPL_TOPIC(serverIp_, client.getNickName(), channelName, topic));
    printInfoToServer(INFO, "Topic for channel " + channelName + " changed to: " + topic, false);
}

void Server::cmdKick(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "KICK");
    std::istringstream iss(params);
    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason); // Remaining part is the reason
    reason.erase(0, reason.find_first_not_of(' ')); // Trim leading spaces
    

    if (channels_.find(channelName) == channels_.end())
    {
        sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));
        return;
    }
    Channel& channel = channels_[channelName];
    Client& client = clients_[clientSocket];
    // Ensure the client is an operator of the channel
    if (!channel.isOperator(clientSocket))
    {
        sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, channelName));
        return;
    }
    // Find the target client
    int targetSocket = -1;
    for (const auto& pair : clients_)
    {
        if (pair.second.getNickName() == targetNick)
        {
            targetSocket = pair.first;
            break;
        }
    }
    if (targetSocket == -1)
    {
        sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));
        return;
    }
    // Ensure the target is in the channel
    if (!channel.isMember(targetSocket))
    {
        sendToClient(clientSocket, ERR_USERNOTINCHANNEL(serverIp_, targetNick, channelName));
        return;
    }
    // Kick the user from the channel
    channel.removeClient(targetSocket);
    removeClient(targetSocket);
    sendToClient(targetSocket, "You have been kicked from " + channelName + " by " + client.getNickName() + " :" + reason);
    sendToClient(clientSocket, "You have kicked " + targetNick + " from " + channelName);
    printInfoToServer(INFO, client.getNickName() + " kicked " + targetNick + " from " + channelName, false);
}

void Server::cmdInvite(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "INVITE");
    std::istringstream iss(params);
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;
    if (channels_.find(channelName) == channels_.end())
    {
        sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));
        return;
    }
    Channel& channel = channels_[channelName];
    Client& client = clients_[clientSocket];
    if (!channel.isClientInChannel(clientSocket))
    {
        sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, channelName));
        return;
    }
    int targetSocket = -1;
    for (const auto& pair : clients_)
    {
        if (pair.second.getNickName() == targetNick)
        {
            targetSocket = pair.first;
            break;
        }
    }
    if (targetSocket == -1)
    {
        sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));
        return;
    }
    sendToClient(targetSocket, RPL_INVITE(serverIp_, client.getNickName(), targetNick, channelName));
    sendToClient(clientSocket, "You have invited " + targetNick + " to " + channelName);
    printInfoToServer(INFO, client.getNickName() + " invited " + targetNick + " to " + channelName, false);
}

void Server::cmdMode(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "MODE");
    std::istringstream iss(params);
    std::string target, mode;
    iss >> target >> mode;

    // Check if the target is a channel
    if (channels_.find(target) != channels_.end())
    {
        Channel& channel = channels_[target];
        if (!channel.isMember(clientSocket))
        {
            sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, target));
            return;
        }

        if (!channel.isOperator(clientSocket))
        {
            sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, target));
            return;
        }
        // Handle invite-only mode (+i/-i)
        if (mode == "+i")
        {
            channel.setInviteOnly(true);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+i", ""));
        }
        else if (mode == "-i")
        {
            channel.setInviteOnly(false);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-i", ""));
        }
        else
        {
            sendToClient(clientSocket, ERR_UNKNOWNMODE(serverIp_, mode));
        }
    }
    else
    {
        sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, target));
    }
}