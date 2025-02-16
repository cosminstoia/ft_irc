// color macros
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"
#define CYAN "\033[36m" 

// server macros
#define MAX_CHARS 1024
#define MAX_CLIENTS 100
#define MAX_Q_CLIENTS 5
#define PING_INTERVAL 60
#define PING_TIMEOUT 120
#define USER_LIMIT_CHANNEL 10


// Format: :<server> <numeric_code> <target> :<message>
// Full format as per IRC protocol (RFC 2812): :<nick>!<user>@<host>
// Example: ":127.0.0.1 001 <nick> :Welcome to the IRC server!\r\n"
// !<user>@<host> is optional metadata. If needed, you can add it after <nick>.
// If metadata is needed: Add "!<user>@<host>" after (nick) and at the end of the message.

// WELCOME MESSAGES
#define RPL_WELCOME(server, nick) (":" + std::string(server) + " 001 " + nick + " :Welcome to the IRC server! by mrusu and cstoia\r\n")
#define RPL_NOTOPIC(server, nick, channel) (":" + std::string(server) + " 331 " + nick + " " + channel + " :No topic is set\r\n")
#define RPL_TOPIC(server, nick, channel, topic) (":" + std::string(server) + " 332 " + nick + " " + channel + " :" + topic + "\r\n")
#define RPL_INVITE(server, nick, target, channel) (":" + std::string(server) + " 341 " + nick + " " + target + " " + channel + "\r\n")
#define RPL_MODE(server, channel, mode, params) (":" + std::string(server) + " 324 " + channel + " " + mode + " " + params + "\r\n")

// ERROR MESSAGES
#define ERR_NOSUCHNICK(server, nick) (":" + std::string(server) + " 401 " + nick + " :No such nick/channel\r\n")
#define ERR_NOSUCHCHANNEL(server, channel) (":" + std::string(server) + " 403 " + channel + " :No such channel\r\n")
#define ERR_CANNOTSENDTOCHAN(server, channel) (":" + std::string(server) + " 404 " + channel + " :Cannot send to channel\r\n")
#define ERR_NICKNAMEINUSE(server, nick) (":" + std::string(server) + " 433 * " + nick + " :Nickname is already in use\r\n")
#define ERR_NEEDMOREPARAMS(server, cmd) (":" + std::string(server) + " 461 * " + cmd + " :Not enough parameters\r\n")
#define ERR_PASSWDMISMATCH(server) (":" + std::string(server) + " 464 * :Password incorrect\r\n")
#define ERR_NOTONCHANNEL(server, channel) (":" + std::string(server) + " 442 * " + channel + " :You're not on that channel\r\n")
#define ERR_CHANOPRIVSNEEDED(server, channel) (":" + std::string(server) + " 482 * " + channel + " :You're not channel operator\r\n")
#define ERR_UNKNOWNMODE(server, mode) (":" + std::string(server) + " 472 * " + mode + " :is unknown mode char\r\n")
#define ERR_NOTREGISTERED(server) (":" + std::string(server) + " 451 * :You have not registered\r\n")
#define ERR_UNKNOWNCOMMAND(server, cmd) (":" + std::string(server) + " 421 * " + cmd + " :Unknown command\r\n")
#define ERR_UMODEUNKNOWNFLAG(server, mode) (":" + std::string(server) + " 501 * :Unknown MODE flag " + mode + "\r\n")
#define ERR_INVITEONLYCHAN(server, channel) (":" + std::string(server) + " 473 * " + channel + " :Cannot join channel (+i)\r\n")
#define ERR_CHANNELISFULL(server, channel) (":" + std::string(server) + " 471 * " + channel + " :Cannot join channel (+l)\r\n")
#define ERR_USERNOTINCHANNEL(server, nick, channel) (":" + std::string(server) + " 441 * " + nick + " " + channel + " :They aren't on that channel\r\n")