# IRC Server

A lightweight IRC (Internet Relay Chat) server implementation supporting multiple clients, channel management, and connection stability through ping-pong mechanisms.

## Table of Contents
- [Core Concepts](#core-concepts)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Testing](#testing)
- [Commands](#commands)
- [Resources](#resources)

## Core Concepts

### What is a Socket?
```
IP Address + Port = Socket (Connection Endpoint)
Example: 192.168.1.1:6667
```
A socket is an endpoint for network communication, combining an IP address and port number to uniquely identify a connection.

### What is a Port?
- Numerical identifier (0-65535)
- Specifies service/application on a device
- Think of it as an apartment number in a building

## Features
- Multiple client support with non-blocking I/O
- Channel management (creation, joining, leaving)
- Operator privileges and channel modes
- Ping-pong mechanism for connection stability
- Command parsing and execution
- Real-time message broadcasting

## Requirements
- C++ compiler (g++)
- Make
- Unix-like operating system

## Installation
```bash
# Clone repository
git clone [repository-url]
cd [repository-name]

# Build project
make

# Run server
./ircserv <port> <password>
```

## Testing

### Using Netcat
```bash
nc -c localhost <port>
PASS password
NICK test
USER testuser hostname servername :Real Name
```

### Using IRSSI
```bash
# Install IRSSI
brew install irssi

# Configure IRSSI
cp config ~/.irssi/config

# Connect to server
irssi -c <host> -p <port> -w <password>
```

## Commands

### Channel Operator Commands
```
Command                         Description
-------                        -----------
MODE #channel +o <user>        Give operator status
MODE #channel +i              Set invite-only
MODE #channel +t              Set topic restriction
MODE #channel +k <pass>       Set channel password
MODE #channel +l <limit>      Set user limit
KICK #channel <user>         Kick user
INVITE <user> #channel       Invite user
TOPIC #channel :<topic>      Change topic
```

## Resources
- [IRC Protocol RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459)
- [IRC Protocol RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812)
- [chirc Documentation](http://chi.cs.uchicago.edu/chirc/)
- [IRSSI Manual](https://irssi.org/documentation/manual/)

## Architecture
```
Server Structure:
+---------------+
| Poll Array    |
|  - Server FD  |
|  - Client FDs |
+---------------+
       │
       ▼
+---------------+
| Client Map    |
|  Socket → Data|
+---------------+
       │
       ▼
+---------------+
| Channels Map  |
| Name → Channel|
+---------------+
```

Last updated: 2025-02-19