#include "Server.hpp"

Server::Server(int port, std::string password) : port_(port), password_(password)
{
    serverSocket_ = -1; // init to -1 adn set up in setup()
    serverIp_ = "nop";
}

Server::~Server()
{
    if (serverSocket_ != -1)
        close(serverSocket_);
}

void Server::printErrorExit(std::string const& msg, bool exitP)
{
    std::cerr << RED "Error: " RESET << msg << std::endl;
    if (exitP)
        exit(1);
}

void Server::printInfo(messageType type, const std::string &msg)
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
        case INFO:
            typeStr = "INFO";
            color = BLUE;
            break;
        case WARNING:
            typeStr = "WARNING";
            color = YELLOW;
            break;
        case CONNECTION:
            typeStr = "CONNECTION";
            color = GREEN;
            break;
        case DISCONNECTION:
            typeStr = "DISCONNECTION";
            color = RED;
            break;
        case PING:
            typeStr = "PING";
            color = GREEN;
            break;
        case PONG:
            typeStr = "PONG";
            color = GREEN;
            break;
        case CLIENT:
            typeStr = "CLIENT";
            color = BLUE;
            break;
        case SERVER:
            typeStr = "SERVER";
            color = BLUE;
            break;
        case TIMEOUT:
            typeStr = "TIMEOUT";
            color = RED;
            break;
        case SUCCESS:
            typeStr = "SUCCESS";
            color = GREEN;
            break;
        default:
            typeStr = "UNKNOWN";
            color = RESET;
            break;
    }
    std::cout << color << "[" << typeStr << "]"  << GRAY << " [" << oss.str()<< "] "
        << RESET << msg << std::endl;
}

void Server::setup()
{
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0)
        printErrorExit("Establishing socket...", true);
    
    fcntl(serverSocket_, F_SETFL, O_NONBLOCK); //allow formula

    printInfo(INFO, "Socket server has been created.");

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    serverAddr_.sin_port = htons(port_);

    if (bind(serverSocket_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
        printErrorExit("Error binding socket.", true);
    
    printInfo(INFO, "Binding success...");

    if (listen(serverSocket_, 5) < 0)
        printErrorExit("Error listening...", true);
}


void Server::start() 
{
    std::cout << GREEN R"(░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀░░░░░░░░░░░░░        
░░░░░░░░░░░░░░░░░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀░░░░░░░░░░░░░       
░░░░░░░░░░░░░░░░░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░░░░░░░░░░░         
░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░█▀▀░█▀▄░░░
░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░▀▀█░░█░░█▀█░█▀▄░░█░░█▀▀░█░█░░░
░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░▀▀▀░▀▀░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░)" << std::endl;
    printInfo(INFO, "Server listening on ip:port " + std::to_string(port_));
    printInfo(INFO, "Waiting for conections...");
    while (true) // or sometign like bool isRunning = true; while (isRunning)
    {
        int ret = poll(pollFds_.data(), pollFds_.size(), -1);
        if (ret < 0)
        {
            printErrorExit("Poll failed!", false);
            continue;
        }
        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].revents & POLLIN)
            {
                if (pollFds_[i].fd == serverSocket_)
                {
                    acceptClient(pollFds_);
                }
                else
                {
                    handleClient(pollFds_[i].fd);
                }
            }
        }
    }
}

void Server::acceptClient(std::vector<pollfd>& pollFds_) 
{
    int clientSocket = accept(serverSocket_, NULL, NULL);
    if (clientSocket < 0)
    {
        printErrorExit("Accept failed!", false);
        return;
    }

    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    
    pollfd clientPollFd = {clientSocket, POLLIN, 0};
    pollFds_.push_back(clientPollFd);

    printInfo(CONNECTION, "New client connected.");

}

void Server::handleClient(int clientSocket) 
{
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0)
    {
        close(clientSocket);
        printInfo(INFO, "Client disconnected!");

        for (size_t i = 0; i < pollFds_.size(); i++)
        {
            if (pollFds_[i].fd == clientSocket)
            {
                pollFds_.erase(pollFds_.begin() + i);
                break;
            }
        }
        return;
    }

    // Basic message handling

    buffer[bytesRead] = '\0';
    std::string message(buffer);
    printInfo(CLIENT, std::string("Received: ") + message);

    // Parse message and if cmd execute it
    // so we need to check the msg and if comdn to execut it
    // i am thinkg if the comnd shave a prefix like / or ! or something 
    // then is easy, we pass the word to a function that will execute the command
}


void Server::sendToClient(int clientSocket, const std::string &message)
{
    std::string fullMessage = message + "\r\n";
    send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
}

bool Server::isCommand(const std::string &input)
{
    std::istringstream iss(input);
    std::string command;
    iss >> command;

    const std::set<std::string> validCommands =
    {
        "NICK", "USER", "JOIN", "PRIVMSG", "QUIT", "PING"
    };

    return validCommands.find(command) != validCommands.end();
}