#include "Server.hpp"

bool isPortInUse(int port)
{
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return true;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return result < 0;
}

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
        if (port < 1024 || port > 65535) // between 1 and 1024 req root
        {
            throw std::out_of_range("Port number must be between 1024 and 65535!");
        }
        if (isPortInUse(port))
        {
            throw std::runtime_error("Port is already in use!");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << RED "Error: " << RESET << e.what() << std::endl;
        return false;
    }
    pass = av[2];
    if (pass.empty() || pass.length() > 32)
    {
        std::cerr << "Password cannot be empty or too long (32 char max)\n";
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
    Server server(port, password);
    server.start();
    return 0;
}