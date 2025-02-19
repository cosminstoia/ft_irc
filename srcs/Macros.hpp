// color macros
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"
#define CYAN "\033[36m" 
#define MAGENTA "\033[35m"
#define LIGHT_MAGENTA "\033[35;1m"

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
#define RPL_WELCOME(server, nick) (":" + server + " 001 " + nick + " :Welcome to the IRC server! by mrusu and cstoia")
#define RPL_NOTOPIC(server, nick, channel) (":" + server + " 331 " + nick + " " + channel + " :No topic is set")
#define RPL_TOPIC(server, nick, channel, topic) (":" + server + " 332 " + nick + " " + channel + " :" + topic)
#define RPL_INVITE(server, nick, target, channel) (":" + server + " 341 " + nick + " " + target + " " + channel)

#define RPL_PART(nick, user, host, channel) (":" + nick + "!" + user + "@" + host + " PART " + channel + " :has left the channel")
#define RPL_JOIN(server, nick, channel) (":" + nick + "!" + nick + "@" + server + " JOIN " + channel)
#define RPL_PRIVMSG(server, nick, recipient, message) (":" + nick + "!" + nick + "@" + server + " PRIVMSG " + recipient + " " + message)
#define RPL_QUIT(nick, user, host, message) (":" + nick + "!" + user + "@" + host + " QUIT :Client " + message)
#define RPL_SERVERMODE(server, channel, mode, nick) (":" + server + " MODE " + channel + " " + mode + " " + nick)
#define RPL_YOUROP "You're now channel operator"
#define RPL_USERHOST(nick, username, server) (":" + nick + "!" + username + "@" + server)
#define RPL_PRIVMSGFORMAT(sender, target, message) (":" + sender + " PRIVMSG " + target + " :" + message)
#define RPL_KICKFORMAT(userhost, channel, target, reason) (userhost + " KICK " + channel + " " + target + " :" + reason)
#define RPL_INVITEFORMAT(userhost, target, channel) (userhost + " INVITE " + target + " :" + channel)
#define RPL_MODEFORMAT(userhost, target, mode) (userhost + " MODE " + target + " " + mode)
#define RPL_MODEFORMAT_PARAM(userhost, target, mode, param) (userhost + " MODE " + target + " " + mode + " " + param)
#define INFO_PRIVMSG_SENT(recipient, sender) ("PRIVMSG sent to " + recipient + " from " + sender)
#define INFO_INVITE_SENT(sender, target, channel) (sender + " invited " + target + " to " + channel)

// ERROR MESSAGES
#define ERR_NOSUCHNICK(server, nick) (":" + server + " 401 " + nick + " :No such nick/channel")
#define ERR_NOSUCHCHANNEL(server, channel) (":" + server + " 403 " + channel + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(server, channel) (":" + server + " 404 " + channel + " :Cannot send to channel")
#define ERR_NICKNAMEINUSE(server, nick) (":" + server + " 433 * " + nick + " :Nickname is already in use")
#define ERR_NONICKNAMEGIVEN(server) (":" + server + " 431 * :Nickname not given")
#define ERR_NEEDMOREPARAMS(server, cmd) (":" + server + " 461 * " + cmd + " :Not enough parameters")
#define ERR_PASSWDMISMATCH(server) (":" + server + " 464 * :Password incorrect")
#define ERR_NOTONCHANNEL(server, channel) (":" + server + " 442 * " + channel + " :You're not on that channel")
#define ERR_CHANOPRIVSNEEDED(server, channel) (":" + server + " 482 * " + channel + " :You're not channel operator")
#define ERR_UNKNOWNMODE(server, mode) (":" + server + " 472 * " + mode + " :is unknown mode char")
#define ERR_NOTREGISTERED(server) (":" + server + " 451 * :You have not registered")
#define ERR_UNKNOWNCOMMAND(server, cmd) (":" + server + " 421 * " + cmd + " :Unknown command")
#define ERR_UMODEUNKNOWNFLAG(server, mode) (":" + server + " 501 * :Unknown MODE flag " + mode)
#define ERR_INVITEONLYCHAN(server, channel) (":" + server + " 473 * " + channel + " :Cannot join channel (+i)")
#define ERR_CHANNELISFULL(server, channel) (":" + server + " 471 * " + channel + " :Cannot join channel (+l)")
#define ERR_USERNOTINCHANNEL(server, nick, channel) (":" + server + " 441 * " + nick + " " + channel + " :They aren't on that channel")
#define ERR_ALREADYREGISTRED(server) (":" + server + " 462 * :You may not reregister")
#define ERR_ERRONEUSNICKNAME(server, nick) (":" + server + " 432 * " + nick + " :Erroneus nickname")
#define ERR_BADCHANNELKEY(server, channel) (":" + server + " 475 * " + channel + " :Cannot join channel (+k)")