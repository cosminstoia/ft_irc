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
    clients_[clientSocket].setUserName(params);
    if (!clients_[clientSocket].getNickName().empty() && !clients_[clientSocket].isLoggedIn()) 
        welcomeClient(clientSocket);
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "JOIN");
    std::string channelName = params;
    if (channels_.find(channelName) == channels_.end())
    {
        channels_.emplace(channelName, Channel(channelName)); // Explicitly construct the channel
        printInfoToServer(INFO, "Channel " + channelName + " created by " + clients_[clientSocket].getNickName(), false);
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
    // Send JOIN confirmation to the joining client
    std::string joinMsg = ":" + clients_[clientSocket].getNickName() + "!" + 
                         clients_[clientSocket].getUserName() + "@" + serverIp_ + 
                         " JOIN :" + channelName + "\r\n";
    sendToClient(clientSocket, joinMsg);
    // Send topic if it exists
    if (channel.getTopic().empty())
        sendToClient(clientSocket, RPL_NOTOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName));
    else
        sendToClient(clientSocket, RPL_TOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName, channel.getTopic()));
    for (int memberSocket : channel.getMembers())
    {
        if (memberSocket != clientSocket)
            sendToClient(memberSocket, joinMsg);
    }
    
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
    std::cout << "Recipient: " << recipient << " Message: " << message << std::endl; //debug

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
    std::string nickName = clients_[clientSocket].getNickName();
    removeClient(clientSocket);
    printInfoToServer(INFO, "Client on socket " + std::to_string(clientSocket) + 
                     " with name " + nickName + " has quit", false);
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
    if (topic.empty())
    {
        std::string currentTopic = channel.getTopic();
        if (currentTopic.empty())
            sendToClient(clientSocket, RPL_NOTOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName));
        else
            sendToClient(clientSocket, RPL_TOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName, currentTopic));
        return;
    }
    if (channel.isTopicRestricted() && !channel.isOperator(clientSocket))
    {
        sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, channelName));
        return;
    }
    channel.setTopic(topic);
    sendToClient(clientSocket, RPL_TOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName, topic));

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
    if (!channel.isOperator(clientSocket))
    {
        sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, channelName));
        return;
    }
    int targetSocket = findClientByNick(targetNick);
    if (targetSocket == -1 || !channel.isMember(targetSocket))
    {
        sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));
        return;
    }
    channel.removeClient(targetSocket);
    removeClient(targetSocket);

    sendToClient(targetSocket, "You have been kicked from " + channelName + " by " + clients_[clientSocket].getNickName() + " :" + reason);
    sendToClient(clientSocket, "You have kicked " + targetNick + " from " + channelName);
    printInfoToServer(INFO, clients_[clientSocket].getNickName() + " kicked " + targetNick + " from " + channelName, false);
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
    if (!channel.isMember(clientSocket))
    {
        sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, channelName));
        return;
    }
    int targetSocket = findClientByNick(targetNick);
    if (targetSocket == -1)
    {
        sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));
        return;
    }
    channel.addInvite(targetSocket);
    sendToClient(targetSocket, RPL_INVITE(serverIp_, clients_[clientSocket].getNickName(), targetNick, channelName));
    sendToClient(clientSocket, "You have invited " + targetNick + " to " + channelName);

    printInfoToServer(INFO, clients_[clientSocket].getNickName() + " invited " + targetNick + " to " + channelName, false);
}

void Server::cmdMode(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "MODE");
    std::istringstream iss(params);
    std::string target, mode, modeParams;
    iss >> target >> mode >> modeParams;

    modeParams.erase(0, modeParams.find_first_not_of(' '));

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
        // Handle channel modes
        if (mode == "+i") // Set invite-only
        {
            channel.setInviteOnly(true);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+i", ""));
        }
        else if (mode == "-i") 
        {
            channel.setInviteOnly(false);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-i", ""));
        }
        else if (mode == "+t") // Restrict topic changes to operators
        {
            channel.topicRestricted(true);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+t", ""));
        }
        else if (mode == "-t") 
        {
            channel.topicRestricted(false);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-t", ""));
        }
        else if (mode == "+k") // Set channel key (password)
        {
            if (modeParams.empty())
            {
                sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                return;
            }
            channel.setPassword(modeParams);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+k", modeParams));
        }
        else if (mode == "-k") // Remove channel key
        {
            channel.setPassword(""); // Clear the password
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-k", ""));
        }
        else if (mode == "+o") // Grant operator status
        {
            if (modeParams.empty())
            {
                sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                return;
            }
            int targetSocket = findClientByNick(modeParams);
            if (targetSocket == -1)
            {
                sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, modeParams));
                return;
            }
            channel.addOperator(targetSocket);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+o", modeParams));
        }
        else if (mode == "-o") // Remove operator status
        {
            if (modeParams.empty())
            {
                sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                return;
            }

            int targetSocket = findClientByNick(modeParams);
            if (targetSocket == -1)
            {
                sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, modeParams));
                return;
            }

            channel.removeOperator(targetSocket);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-o", modeParams));
        }
        else if (mode == "+l") // Set user limit
        {
            try
            {
                int limit = std::stoi(modeParams);
                if (limit <= 0)
                {
                    sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                    return;
                }
                channel.setUserLimit(limit);
                sendToClient(clientSocket, RPL_MODE(serverIp_, target, "+l", std::to_string(limit)));
            }
            catch (std::exception& e)
            {
                sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
            }
        }
        else if (mode == "-l") // Remove user limit
        {
            channel.setUserLimit(0);
            sendToClient(clientSocket, RPL_MODE(serverIp_, target, "-l", ""));
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