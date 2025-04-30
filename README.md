# ft_irc - IRC Server Implementation

This project is an implementation of an IRC (Internet Relay Chat) server in C++98, following the RFC 2812 specification. The server can handle multiple clients simultaneously, manages channels, and supports various IRC commands and channel modes.

## Features

- Non-blocking I/O with socket programming
- User registration and authentication
- Channel creation and management
- Channel modes (invite-only, topic protection, etc.)
- Private messaging
- Operator privileges
- Command handling for standard IRC commands
- Compliance with RFC 2812 specifications

## Requirements

- C++98 compatible compiler (g++ or clang++)
- POSIX compliant operating system (Linux or macOS)

## Compilation

To compile the project, simply run:

```bash
make
```

This will create an executable called `ircserv` in the root directory.

## Usage

To start the IRC server:

```bash
./ircserv <port> <password>
```

- `<port>`: The port number the server will listen on (1024-65535)
- `<password>`: The password clients need to connect to the server

## Connecting to the Server

### Using netcat

You can use netcat to test the server:

```bash
nc -C localhost <port>
```

Once connected, you need to register by sending these commands:

```
PASS <password>
NICK <nickname>
USER <username> 0 * :<real name>
```

### Using irssi

You can also connect with the irssi IRC client:

```bash
irssi -c localhost -p <port> -n <nickname> -w <password>
```

## Supported Commands

The server supports the following IRC commands:

- `PASS`: Set the connection password
- `NICK`: Set or change your nickname
- `USER`: Set your username and real name
- `JOIN`: Join a channel
- `PART`: Leave a channel
- `PRIVMSG`: Send a message to a user or channel
- `NOTICE`: Send a notice to a user or channel
- `MODE`: Change channel or user modes
- `TOPIC`: Get or set a channel's topic
- `LIST`: List all channels
- `NAMES`: List users in a channel
- `INVITE`: Invite a user to a channel
- `KICK`: Remove a user from a channel
- `PING`/`PONG`: Server ping/pong
- `QUIT`: Disconnect from the server
- `WHO`: Get information about users
- `WHOIS`: Get detailed information about a user

## Channel Modes

The following channel modes are supported:

- `i`: Invite-only channel
- `t`: Only operators can change the topic
- `k`: Channel password
- `o`: Channel operator
- `l`: User limit
- `n`: No messages from outside the channel
- `m`: Moderated channel
- `v`: Voice (ability to speak in moderated channels)
- `b`: Ban mask

## User Modes

The following user modes are supported:

- `i`: Invisible
- `o`: Operator
- `w`: Receive wallops
- `a`: Away
- `r`: Restricted
- `O`: Local operator
- `s`: Receive server notices

## Testing

You can test the server using the provided test command:

```bash
make test
```

This will display instructions for connecting with netcat and irssi.

## Debugging

To compile with debug symbols for debugging with gdb or lldb:

```bash
make debug
```

## License

This project is part of the 42 School curriculum, created for educational purposes. 