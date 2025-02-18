# IRC Server

This is a simple IRC server implementation that allows users to connect and communicate in channels. This README provides instructions on how to set up and connect to the server, as well as explanations of key concepts like sockets and ports.

## resource
https://datatracker.ietf.org/doc/html/rfc1459
https://datatracker.ietf.org/doc/html/rfc2812
http://chi.cs.uchicago.edu/chirc/
https://irssi.org/documentation/manual/

What is a Socket and What is a Port?
A socket is an endpoint for sending or receiving data across a computer network. It is a combination of an IP address and a port number, which together uniquely identify a connection to a specific service on a specific device. Think of a socket as the full address of an apartment, which includes both the street name and the apartment number.

What is a port?
A port is a numerical identifier in the range of 0 to 65535 that is used to specify a particular service or application on a device. It acts as a communication endpoint for the socket. In our analogy, the port is like the street name where the apartment is located. It helps direct the data to the correct service running on that device.


## Features

- Supports multiple clients
- Channel management
- Ping-pong mechanism for connection stability

## Requirements

- C++ compiler (e.g., g++)
- Make (for building the project)

## Installation
git clone .....
cd it
make
./ircserv <port> <pass>



## no client test
nc -c localhost <port> [enter]
PASS password [enter]
NICK test [enter]
USER testuser hostname servername :Real Name [enter] 
[you shuld see a confirmation adn be able to use commands]


#using irssi
brew install irssi
cp config ~/.issi/config
irssi -c <host> -p <port> -w <pass>

/mode #channel +i/-i ....