#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

bool checkArgs(int ac, char*av[], int& port, std::string& pass)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return false;
    }
    try
    {
        port = std::stoi(av[1]);
        if (port < 1 || port > 65535)
        {
            throw std::out_of_range("Port number must be between 1 and 65535");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    pass = av[2];
    if (pass.empty())
    {
        std::cerr << "Password cannot be empty\n";
        return false;
    }
    return true;
}

int main(int ac, char *av[])
{
    int port;
    std::string password;
    if (!checkArgs(ac, av, port, password))
        return 1;
    std::cout << "Port: " << port << " Password: " << password << std::endl;
    try
    {
        Server server(port, password);
        server.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    
    return 0;
}