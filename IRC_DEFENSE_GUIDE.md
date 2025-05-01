# ft_irc Defense Guide

This defense guide will help you explain the IRC server project to evaluators who may not be familiar with IRC protocols or the implementation details.

## Project Overview

The ft_irc project is an Internet Relay Chat (IRC) server implementation that follows the RFC 2812 specification. It allows multiple clients to connect, exchange messages, manage channels, and use various IRC commands.

### Key Technologies

1. **Socket Programming**: Used for network communication between clients and the server
2. **Non-blocking I/O**: Implemented with `fcntl` to handle multiple client connections efficiently
3. **Poll Multiplexing**: Used `poll()` to monitor multiple file descriptors simultaneously
4. **C++98 Standard**: The entire project follows the C++98 standard as required

## Core Components Explanation

### Server Architecture

The server follows a client-server architecture where:
- The server listens on a specified port
- Clients connect to the server using IRC clients (irssi, netcat, etc.)
- Messages are parsed and routed between clients and channels
- All I/O operations are non-blocking to prevent any single client from freezing the server

### Non-blocking I/O and Poll

Explain why non-blocking I/O with `poll()` is essential:

1. **Single-threaded efficiency**: Using `poll()` allows the server to handle hundreds of client connections with a single thread
2. **Resource efficiency**: Avoids creating one thread per client, which would consume excessive system resources
3. **Responsiveness**: The server remains responsive even if some clients are slow or unresponsive

Example code snippets to explain during defense:
```cpp
// Setting socket to non-blocking mode
int flags = fcntl(socketFD, F_GETFL, 0);
fcntl(socketFD, F_SETFL, flags | O_NONBLOCK);

// Using poll() to monitor multiple file descriptors
pollfd fds[MAX_CLIENTS + 1];
// Set up poll structure...
int ready = poll(fds, numFds, TIMEOUT);
```

### Client Management

Explain how the server handles client connections:
- Authentication process with PASS, NICK, and USER commands
- Client state tracking (registered, channels joined, etc.)
- Buffer management for partial messages

### Channel Management

Explain channel operations:
- Creating and joining channels
- Channel modes (invite-only, key, topic protection, etc.)
- User privileges within channels (operators, voice, etc.)

### Message Parsing

Explain the IRC protocol message parsing:
- IRC messages format: `[:prefix] command parameters[:trailing]`
- Command handlers implementation
- Error handling

## Requirements Fulfilled

### Mandatory Requirements

1. ✅ **Communication between clients and server using TCP/IP**
   - The server exclusively uses TCP/IP sockets for client communication

2. ✅ **No forking, all I/O operations are non-blocking**
   - Using `fcntl()` to set non-blocking mode and `poll()` for I/O multiplexing

3. ✅ **Can handle multiple clients simultaneously without hanging**
   - The server can handle hundreds of client connections using efficient I/O

4. ✅ **Support for authentication, nickname setting, channel creation/deletion**
   - All required commands (PASS, NICK, USER, JOIN, PART) are implemented

5. ✅ **Public and private messaging**
   - PRIVMSG command allows messaging to channels or directly to users

6. ✅ **Channel operators and regular users**
   - Implemented channel modes that designate operators with special privileges

7. ✅ **All appropriate IRC command implementations**
   - All required commands following RFC 2812 are implemented

### Channel Modes Implemented

- `+i`: Invite-only channel
- `+t`: Only operators can change topic
- `+k`: Channel password
- `+o`: Channel operator
- `+l`: User limit (currently being implemented)

## Performance and Robustness

The server has been tested for:
- **High connection loads**: Multiple clients connecting simultaneously
- **Error handling**: Server properly handles malformed messages
- **Edge cases**: Testing unusual sequences of commands and user behaviors

## Testing Tools

During development and testing, we used:
1. **Netcat**: For direct protocol testing with raw IRC commands
2. **irssi**: For real-world IRC client testing
3. **Custom tests**: To verify correct behavior in edge cases

We created convenient Makefile commands to facilitate testing:
- `make run`: Start the IRC server
- `make nc`: Connect with netcat
- `make irssi`: Connect with irssi client

## Current Limitations and Future Improvements

- Channel user limit mode (`+l`) is currently being implemented
- Some additional channel modes could be added in the future
- Server configuration could be expanded for more customization options

## Conclusion

The ft_irc project is a robust IRC server implementation that complies with the RFC 2812 specification. It efficiently handles multiple client connections using non-blocking I/O and the poll mechanism, demonstrating a solid understanding of network programming concepts and the IRC protocol.

## Common Questions and Answers

### How does non-blocking I/O work?

Non-blocking I/O allows the server to check if a socket is ready for reading or writing without waiting. If not ready, it returns immediately with an error code (EWOULDBLOCK or EAGAIN). This allows the server to handle other clients without being blocked by any single client.

### Why not use threads for each client?

Using threads for each client would consume more system resources and create synchronization complexities. Our approach using `poll()` is more efficient and follows the project requirements.

### How do you handle partial messages?

Each client has a buffer where we store incoming data. We process complete messages from this buffer and keep partial messages until more data arrives.

### How does the server handle client disconnections?

When a client disconnects, we detect it through `poll()` or read/write errors, clean up the client's resources, inform other relevant clients, and remove the client from any channels it was in. 