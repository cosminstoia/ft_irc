#include "Bot.hpp"
#include <ctime>

Bot::Bot() 
{
    botCommand_["!help"] = &Bot::helpCommand;
    botCommand_["!motivate"] = &Bot::motivationCommand;
}

Bot::~Bot() {}

void Bot::helpCommand(Server& server, int clientSocket, const std::string& recipient) 
{
    std::string helpMessage =
        "---------HELP----------\n"
        "HELP - Lists all commands\n"
        "KICK - Eject a client from the channel\n"
        "INVITE - Invite a client to a channel\n"
        "TOPIC - Change or view the channel topic\n"
        "MODE - Change the channel’s mode:\n"
        "  · i: Set/remove Invite-only channel\n"
        "  · t: Set/remove the restrictions of the TOPIC command to channel operators\n"
        "  · k: Set/remove the channel key (password)\n"
        "  · o: Give/take channel operator privilege\n"
        "--------END-HELP-------";
    std::istringstream iss(helpMessage);
    std::string line;
    while (std::getline(iss, line)) 
        server.sendMsgToChannel(clientSocket, recipient, line);
}

void Bot::motivationCommand(Server& server, int clientSocket, const std::string& recipient) 
{
    std::vector<std::string> quotes = {
        "Believe you can and you're halfway there.",
        "The only way to do great work is to love what you do.",
        "You are never too old to set another goal or to dream a new dream.",
        "Success is not the key to happiness. Happiness is the key to success.",
        "The future belongs to those who believe in the beauty of their dreams."
    };
    if (!quotes.empty())
        server.sendMsgToChannel(clientSocket, recipient, quotes.at(rand() % quotes.size()));
}

void Bot::executeCommand(Server& server, int clientSocket, const std::string& recipient, const std::string& command) 
{
    auto it = botCommand_.find(command);
    if (it != botCommand_.end()) 
    {
        BotCommandFunction commandFunction = it->second;
        (this->*commandFunction)(server, clientSocket, recipient);
    }
    else
       server.sendMsgToChannel(clientSocket, recipient, "Unknown command: " + command);
}