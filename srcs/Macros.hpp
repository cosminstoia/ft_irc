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

// WELCOME MESSAGES
#define RPL_WELCOME(nick) ("001 " + (nick) + ": Welcome to the Internet Relay Network by mrusu and cstoia\r\n")
#define RPL_YOURHOST "002 : Your host is ircserv\r\n"
#define RPL_CREATED "003 : This server was created recently\r\n"
#define RPL_MYINFO "004 : Server info\r\n"

// WHOIS
#define RPL_WHOISUSER(nick, username, host, realname) ("311 " + (nick) + " " + (username) + " " + (host) + " * :" + (realname) + "\r\n")
#define RPL_WHOISSERVER(nick, server, info) ("312 " + (nick) + " " + (server) + " :" + (info) + "\r\n")
#define RPL_ENDOFWHOIS(nick) ("318 " + (nick) + " :End of WHOIS list\r\n")

// ERROR MESSAGES
#define ERR_NICKNAMEINUSE(nick) ("433 " + (nick) + " : Nickname is already in use\r\n")
#define ERR_NEEDMOREPARAMS(cmd) ("461 " + (cmd) + " : Not enough parameters\r\n")
#define ERR_NOSUCHCHANNEL(channel) ("403 " + (channel) + " : No such channel\r\n")
#define ERR_CHANOPRIVSNEEDED(channel) ("482 " + (channel) + " : You're not channel operator\r\n")
#define ERR_NOTREGISTERED "451 : You have not registered\r\n"
#define ERR_PASSWDMISMATCH "464 : Password incorrect\r\n"