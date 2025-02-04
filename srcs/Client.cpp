#include "Client.hpp"
#include <cstring> // For memset

using namespace std;

Client::Client(string ip, int clientSocket) : ip(ip), portNum(123), isExit(false), buffSize(1024) 
{
    buffer = new char[buffSize];
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) 
    {
        cout << ">> Error establishing socket..." << endl;
        exit(1);
    }
    cout << ">> Socket client has been created..." << endl;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNum);
    memset(&serverAddr.sin_addr, 0, sizeof(serverAddr.sin_addr)); // Initialize server address
}

Client::~Client() 
{
    //delete[] buffer;
    close(clientSocket);
}