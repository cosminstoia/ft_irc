#include "Server.hpp"

void printInfoToServer(messageType type, std::string const& msg)
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
        case BOT:  typeStr = "BOT";   color = GRAY;   break;
        case ERROR:  typeStr = "ERROR";   color = RED;   break;
        case PING: typeStr = "PING";  color = CYAN;   break;
        case PONG: typeStr = "PONG";  color = CYAN;   break;
        default:        typeStr = "UNKNOWN";    color = RESET; break;
    }

    std::cout << color << "[" << typeStr << "]" << GRAY " [" << oss.str() << "] "
                << RESET << msg << std::endl;
}

void printErrorExit(std::string const& msg, bool exitP)
{
    std::cerr << RED "Error: " RESET << msg << std::endl;
    if (exitP)
        exit(1);
}