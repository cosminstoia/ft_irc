#include "Server.hpp"

// static member initialization
Server* Server::instance = nullptr;

void Server::signalHandler(int signum)
{
    if (signum == SIGINT && instance)    
        instance->isRunning_ = false;
}

void Server::sSignalHandler(int signum)
{
    if (instance)
        instance->signalHandler(signum);
}

void Server::setupSignalHandler()
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = Server::sSignalHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        printInfoToServer(ERROR, "Signal handler failed!", true);
}