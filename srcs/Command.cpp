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
    commandMap_["MODE"] = std::bind(&Server::cmdMode, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["INVITE"] = std::bind(&Server::cmdInvite, this, std::placeholders::_1, std::placeholders::_2);
    commandMap_["PART"] = std::bind(&Server::cmdPart, this, std::placeholders::_1, std::placeholders::_2);
}

void Server::validateParams(int clientSocket, const std::string& params, const std::string& command)
{
    if (params.empty())
        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, command));
}

void Server::cmdNick(int clientSocket, std::string const& params)
{
    if (params.empty())
        return sendToClient(clientSocket, ERR_NONICKNAMEGIVEN(serverIp_));
    
    if (params.find_first_of(" ,*?!@.") != std::string::npos)
        return sendToClient(clientSocket, ERR_ERRONEUSNICKNAME(serverIp_, params));

    // check if the nickname is already in use
    for (const auto& pair : clients_)
        if (pair.second.getNickName() == params && pair.first != clientSocket)
            return sendToClient(clientSocket, ERR_NICKNAMEINUSE(serverIp_, params));

    Client& client = clients_[clientSocket];
    std::string oldNick = client.getNickName();
    
    if (oldNick != params)
    {
        std::string nickMsg;
        if (client.isLoggedIn())
            // full format hostmask
            nickMsg = ":" + oldNick + "!" + client.getUserName() +
                                "@" + serverIp_ + " NICK :" + params;
        else
            nickMsg = ":" + serverIp_ + " NICK :" + params;

        // Update the nickname in server data
        client.setNickName(params);

        // Notify all clients in the same channel
        std::set<int> notifiedClients;
        for (const auto& channel : channels_)
        {
            if (channel.second.isMember(clientSocket))
            {
                for (int memberSocket : channel.second.getMembers())
                {
                    if (notifiedClients.find(memberSocket) == notifiedClients.end())
                    {
                        sendToClient(memberSocket, nickMsg);
                        notifiedClients.insert(memberSocket);
                    }
                }
            }
        }
        // Also send to the client itself if not already sent
        if (notifiedClients.find(clientSocket) == notifiedClients.end())
            sendToClient(clientSocket, nickMsg);
    }
}

void Server::cmdUser(int clientSocket, std::string const& params)
{
    // Split params into required parts
    std::istringstream iss(params);
    std::string username, hostname, servername, realname;
    
    // Get first 3 parameters
    if (!(iss >> username >> hostname >> servername))
        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "USER"));

    // Get realname (everything after the :)
    size_t colonPos = params.find(':');
    if (colonPos == std::string::npos)
        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "USER"));

    Client& client = clients_[clientSocket];

    // Check if user is already registered
    if (client.isLoggedIn())
        return sendToClient(clientSocket, ERR_ALREADYREGISTRED(serverIp_));

    client.setUserName(username);
    client.setRealName(realname);

    // If we have both USER and NICK, welcome the client
    if (!client.getNickName().empty() && !client.isLoggedIn())
        welcomeClient(clientSocket);
}

void Server::cmdJoin(int clientSocket, std::string const& params)
{
    if (params == ":" || params.empty())
        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "JOIN"));
    std::string channelName = params;

    if (channelName[0] != '#')
        channelName = "#" + channelName;

    // std::cout << "[debug]Channel name: " << channelName << std::endl;
    if (channels_.find(channelName) == channels_.end())
    {
        channels_.emplace(channelName, Channel(channelName)); // Explicitly construct the channel
        printInfoToServer(CHANNEL, "Channel " + channelName + " created by " + clients_[clientSocket].getNickName(), false);
    }
    Channel& channel = channels_[channelName];
    bool isFirstMember = channel.getMembers().empty();

    if(!channel.getPassword().empty() && params != channel.getPassword())
        return sendToClient(clientSocket, ERR_BADCHANNELKEY(serverIp_, channelName));

    // check restrictions
    if (channel.isInviteOnly() && !channel.isInvited(clientSocket))
        return sendToClient(clientSocket, ERR_INVITEONLYCHAN(serverIp_, channelName));
    if (channel.isFull())
        return sendToClient(clientSocket, ERR_CHANNELISFULL(serverIp_, channelName));

    // add client to channel
    channel.addMember(clientSocket);
    // notify other clients in the channel
    std::string joinMsg = RPL_JOIN(serverIp_, clients_[clientSocket].getNickName(), channelName);
    for (int memberSocket : channel.getMembers())
        sendToClient(memberSocket, joinMsg);
    if (isFirstMember)
    {
        channel.addOperator(clientSocket);
        std::string modeMsg = ":" + serverIp_ + " MODE " + channelName + " +o " + 
                             clients_[clientSocket].getNickName();
        sendToClient(clientSocket, modeMsg);
        sendToClient(clientSocket, "You're now channel operator");
    }
    if (channel.getTopic().empty())
        sendToClient(clientSocket, RPL_NOTOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName));
    else
        sendToClient(clientSocket, RPL_TOPIC(serverIp_, clients_[clientSocket].getNickName(), channelName, channel.getTopic()));
    printInfoToServer(CHANNEL, "Client " + clients_[clientSocket].getNickName() + " joined channel " + channelName, false);
}

void Server::cmdPrivmsg(int clientSocket, std::string const& params)
{    
    validateParams(clientSocket, params, "PRIVMSG");

    size_t sp = params.find(' ');
    if (sp == std::string::npos || sp == 0)
        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "PRIVMSG"));

    std::string recipient = params.substr(0, sp); // "#chanel" or "user"
    std::string message = params.substr(sp + 1); // ":message"

    if (recipient[0] == '#')
    {
        if (channels_.find(recipient) == channels_.end())
            return sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, recipient));

        Channel& channel = channels_[recipient];
        if (!channel.isMember(clientSocket))
            return sendToClient(clientSocket, ERR_CANNOTSENDTOCHAN(serverIp_, recipient));
        if (!message.empty() && message[1] == '!')
        {
            printInfoToServer(INFO, "BOT called by " + clients_[clientSocket].getNickName(), false);
            bot_->executeCommand(*this, clientSocket, recipient, message.substr(1));
        }
        else
        {
            //remove channel message
            std::string privmsg = RPL_PRIVMSG(serverIp_, clients_[clientSocket].getNickName(), recipient, message);

            for (int memberSocket : channel.getMembers())
            {
                if (memberSocket != clientSocket)
                    sendToClient(memberSocket, privmsg);
            }

            // Log the action
            std::ostringstream oss;
            oss << "Message sent on channel [" << recipient << "] Users in channel: [";
            bool first = true;
            for (int memberSocket : channel.getMembers())
            {
                if (!first) oss << ", ";
                oss << clients_[memberSocket].getNickName();
                first = false;
            }
            oss << "]";
            printInfoToServer(CHANNEL, oss.str(), false);
        }
    }
    else
    {
        int recipientSocket = findClientByNick(recipient);
        if (recipientSocket == -1)
        {
            sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, recipient));
            //if its not a channel and not a user or not bot then we can asuem its to a
            return;
        }
        std::string senderNick = clients_[clientSocket].getNickName();
        std::string privmsg = ":" + senderNick + " PRIVMSG " + recipient + " :" + message;
        sendToClient(recipientSocket, privmsg);
        printInfoToServer(PRIVMSG, "PRIVMSG sent to " + recipient + " from " + senderNick, false);
    }
}

void Server::cmdQuit(int clientSocket, std::string const& params)
{
    (void)params;
    std::string nickName = clients_[clientSocket].getNickName();
    std::string userName = clients_[clientSocket].getUserName();
    std::string host = clients_[clientSocket].getIpAddr();
    // Notify other clients in channels
    for (auto & pair : channels_)
    {
        Channel & channel = pair.second;
        if (channel.isMember(clientSocket))
        {
            std::string quitMsg = RPL_QUIT(nickName, userName, host);
            for (int memberSocket : channel.getMembers())
            {
                if (memberSocket != clientSocket)
                    sendToClient(memberSocket, quitMsg);
            }
            channel.removeClient(clientSocket);
        }
    }
    removeClient(clientSocket);
}

void Server::cmdPing(int clientSocket, std::string const& params)
{
    (void)params;
    std::string response = "PONG :" + serverIp_;
    sendToClient(clientSocket, response);
}

void Server::cmdPong(int clientSocket, std::string const& params)
{
    (void)params;
    std::string lastKey;
    for (const auto& pair : channels_) 
    {
        const std::string& key = pair.first;
        lastKey = key;
    }
    bot_->botPeriodicBroadcast(*this, clientSocket, lastKey);
    auto it = clients_.find(clientSocket);
    if (it != clients_.end())
    {
        it->second.updatePongReceived();
        printInfoToServer(PONG, "Received from client on socket " + std::to_string(clientSocket), false);
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
    if (!channel.isMember(clientSocket))
        return sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, channelName));
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
        return sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));

    Channel& channel = channels_[channelName];
    if (!channel.isOperator(clientSocket))
        return sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, channelName));

    int targetSocket = findClientByNick(targetNick);

    if (targetSocket == -1 || !channel.isMember(targetSocket))
        return sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));

    // Notify all clients in the channel
    std::string kickMsg = ":" + clients_[clientSocket].getNickName() + "!" + 
                         clients_[clientSocket].getUserName() + "@" + serverIp_ + 
                         " KICK " + channelName + " " + targetNick + " :" + reason; // mawyb make all defined
    for (int memberSocket : channel.getMembers())
        sendToClient(memberSocket, kickMsg);
    
    channel.removeClient(targetSocket);
    sendToClient(clientSocket, "You have kicked " + targetNick + " from " + channelName);
    printInfoToServer(WARNING, clients_[clientSocket].getNickName() + " kicked " + targetNick + " from " + channelName, false);
}

void Server::cmdInvite(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "INVITE");
    std::istringstream iss(params);
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;

    if (channels_.find(channelName) == channels_.end())
        return sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));

    Channel& channel = channels_[channelName];
    if (!channel.isMember(clientSocket))
        return sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, channelName));

    int targetSocket = findClientByNick(targetNick);
    if (targetSocket == -1)
        return sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, targetNick));

    channel.addInvite(targetSocket);
    sendToClient(targetSocket, RPL_INVITE(serverIp_, clients_[clientSocket].getNickName(), targetNick, channelName));
    sendToClient(clientSocket, ":" + clients_[clientSocket].getNickName() + "!" + 
                           clients_[clientSocket].getUserName() + "@" + serverIp_ + 
                           " INVITE " + targetNick + " :" + channelName);
    printInfoToServer(INFO, clients_[clientSocket].getNickName() + " invited " + targetNick + " to " + channelName, false);
}

void Server::cmdMode(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "MODE");
    std::istringstream iss(params);
    std::string target, mode, modeParams;
    iss >> target >> mode;
    std::getline(iss, modeParams); // Better way to get remaining params with spaces
    modeParams.erase(0, modeParams.find_first_not_of(' '));

    // Add # prefix if missing
    if (target[0] != '#')
        target = "#" + target;

    if (channels_.find(target) == channels_.end())
        return sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, target));

    Channel& channel = channels_[target];
    if (!channel.isMember(clientSocket))
        return sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, target));
    if (!channel.isOperator(clientSocket))
        return sendToClient(clientSocket, ERR_CHANOPRIVSNEEDED(serverIp_, target));
    if (mode.length() != 2)
        return sendToClient(clientSocket, ERR_UNKNOWNMODE(serverIp_, mode));

    bool adding = (mode[0] == '+');
    char modeChar = mode[1];

    std::string userHost = clients_[clientSocket].getNickName() + "!" + 
                          clients_[clientSocket].getUserName() + "@" + serverIp_;
    std::string modeMsg;

    switch(modeChar)
    {
        case 'i':   // Invite-only
            channel.setInviteOnly(adding);
            modeMsg = ":" + userHost + " MODE " + target + " " + mode;
            break;

        case 't':   // Topic restriction
            channel.topicRestricted(adding);
            modeMsg = ":" + userHost + " MODE " + target + " " + mode;
            break;

        case 'k':   // Key/Password
            if (adding && modeParams.empty())
                return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
            channel.setPassword(adding ? modeParams : "");
            modeMsg = ":" + userHost + " MODE " + target + " " + mode + 
                     (adding ? " " + modeParams : "");
            break;

        case 'o':   // Operator status
        {
            if (modeParams.empty())
                return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
            
            // Trim any extra spaces from nickname
            modeParams = modeParams.substr(0, modeParams.find(' '));
            
            int targetSocket = findClientByNick(modeParams);
            if (targetSocket == -1)
                return sendToClient(clientSocket, ERR_NOSUCHNICK(serverIp_, modeParams));
            
            if (!channel.isMember(targetSocket))
                return sendToClient(clientSocket, ERR_USERNOTINCHANNEL(serverIp_, modeParams, target));
            
            if (adding)
                channel.addOperator(targetSocket);
            else
                channel.removeOperator(targetSocket);
            
            modeMsg = ":" + userHost + " MODE " + target + " " + mode + " " + modeParams;
            break;
        }
        case 'l':   // User limit
            if (adding)
            {
                try
                {
                    int limit = std::stoi(modeParams);
                    if (limit <= 0)
                        return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                    channel.setUserLimit(limit);
                    modeMsg = ":" + userHost + " MODE " + target + " " + mode + " " + modeParams;
                }
                catch (std::exception& e)
                {
                    return sendToClient(clientSocket, ERR_NEEDMOREPARAMS(serverIp_, "MODE"));
                }
            }
            else
            {
                channel.setUserLimit(0);
                modeMsg = ":" + userHost + " MODE " + target + " " + mode;
            }
            break;
            
        default:
            return sendToClient(clientSocket, ERR_UNKNOWNMODE(serverIp_, mode));
    }
    // Broadcast to all channel members
    for (int memberSocket : channel.getMembers())
        sendToClient(memberSocket, modeMsg);
}

void Server::cmdPart(int clientSocket, std::string const& params)
{
    validateParams(clientSocket, params, "PART");
    std::string channelName = params;

    if (channels_.find(channelName) == channels_.end())
        return sendToClient(clientSocket, ERR_NOSUCHCHANNEL(serverIp_, channelName));

    Channel& channel = channels_[channelName];
    if (!channel.isMember(clientSocket))
        return sendToClient(clientSocket, ERR_NOTONCHANNEL(serverIp_, channelName));

    // construct msg
    std::string partMsg = ":" + clients_[clientSocket].getNickName() + "!" +
        clients_[clientSocket].getUserName() + "@" + clients_[clientSocket].getIpAddr() +
        " PART " + channelName;
    for (int memberSocket : channel.getMembers())
        sendToClient(memberSocket, partMsg);
    channel.removeMember(clientSocket);
    clients_[clientSocket].leaveChannel(channelName);
    printInfoToServer(CHANNEL, "Client " + clients_[clientSocket].getNickName() +
                      " has left channel " + channelName, false);
}
