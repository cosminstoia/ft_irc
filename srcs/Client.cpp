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
    delete[] buffer;
    close(clientSocket);
}

bool Client::connectToServer() 
{
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) 
    {
        cout << ">> Connection to the server port number: " << portNum << endl;
        return true;
    }
    return false;
}

void Client::communicate()
{
    cout << ">> Waiting for the confirmation from the server..." << endl;
    recv(clientSocket, buffer, buffSize, 0);
    cout << ">> Connection confirmed!" << endl;
    cout << ">> Enter # to end the connection\n" << endl;

    do 
    {
        cout << "Client: ";
        do 
        {
            cin >> buffer;
            send(clientSocket, buffer, buffSize, 0);
            if (*buffer == '#') 
            {
                send(clientSocket, buffer, buffSize, 0);
                *buffer = '*';
                isExit = true;
            }
        } 
        while (*buffer != 42);

        cout << "Server: ";
        do 
        {
            recv(clientSocket, buffer, buffSize, 0);
            cout << buffer << " ";
            if (*buffer == '#') 
            {
                *buffer = '*';
                isExit = true;
            }
        } 
        while (*buffer != 42);
        cout << endl;
    } 
    while (!isExit);

    cout << ">> Connection terminated!" << endl;
}