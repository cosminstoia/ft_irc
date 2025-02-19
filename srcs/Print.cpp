#include "Server.hpp"

void printInfoToServer(messageType type, std::string const& msg, bool exitP)
{
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm *tm_now = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%H:%M:%S");

    std::string typeStr;
    std::string color;
    switch (type)
    {
        case INFO:  typeStr = "INFO";   color = BLUE;   break;
        case WARNING:   typeStr = "WARNING";    color = YELLOW; break;
        case CONNECTION:typeStr = "CONNECTION"; color = GREEN;  break;
        case DISCONNECTION: typeStr = "DISCONNECTION"; color = RED; break;
        case ERROR:  typeStr = "ERROR";   color = RED;   break;
        case PING: typeStr = "PING";  color = CYAN;   break;
        case PONG: typeStr = "PONG";  color = CYAN;   break;
        case CHANNEL: typeStr = "CHANNEL"; color = MAGENTA; break;
        case PRIVMSG: typeStr = "CLIENT"; color = LIGHT_MAGENTA; break;
        default:        typeStr = "UNKNOWN";    color = RESET; break;
    }
    std::cout << color << "[" << typeStr << "]" << GRAY " [" << oss.str() << "] "
                << RESET << msg << std::endl;
    if(typeStr == "ERROR" && exitP)
        exit(1);
}
