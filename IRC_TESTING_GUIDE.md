# IRC Server Testing Guide with irssi

This guide provides step-by-step instructions for testing the IRC server using irssi, a powerful terminal-based IRC client.

## Getting Started

### Connecting to the Server

1. Install irssi if you don't have it:
   ```
   # macOS with Homebrew
   brew install irssi
   
   # Debian/Ubuntu
   sudo apt-get install irssi
   ```

2. Start the IRC server:
   ```
   ./ircserv 6667 password
   ```
   
   Alternatively, you can use the Makefile to run the server:
   ```
   make run
   ```
   This will start the server on port 6667 with "password" as the password.

3. Launch irssi and connect to the server:
   ```
   irssi -c localhost -p 6667 -n nickname -w password
   ```
   - `-c`: Server address
   - `-p`: Port number
   - `-n`: Your nickname
   - `-w`: Server password
   
   For convenience, you can also use the provided Makefile command:
   ```
   make irssi
   ```
   This will automatically connect to the server with default settings.

### Using Netcat for Testing

If you prefer to use netcat (nc) for more direct protocol testing:

1. Connect using the provided Makefile command:
   ```
   make nc
   ```
   This will connect to the server and display the required registration commands.

2. Once connected with nc, you need to manually send these commands in order:
   ```
   PASS password
   NICK testuser
   USER testuser 0 * :Test User
   ```

### Basic Commands

Once connected, you'll see a status window. Here are essential commands:

- `/help`: Display irssi commands
- `/quit`: Disconnect from server
- `/exit`: Exit irssi completely

## Testing IRC Features

### Registration and Basic Connectivity

1. Connect and verify successful connection:
   ```
   irssi -c localhost -p 6667 -n user1 -w password
   ```

2. If connection fails or no welcome message appears, check:
   - Server is running
   - Password is correct
   - Port is available

### Multiple Client Testing

For thorough testing, open multiple terminal windows with different clients:

```
# Terminal 1
irssi -c localhost -p 6667 -n user1 -w password

# Terminal 2
irssi -c localhost -p 6667 -n user2 -w password

# Terminal 3
irssi -c localhost -p 6667 -n user3 -w password
```

You can also combine irssi and netcat clients:
```
# Terminal 1
make nc    # connects with netcat

# Terminal 2
make irssi # connects with irssi as testuser
```

### Channels Management

#### Creating and Joining Channels

1. Create a channel by joining it:
   ```
   /join #test
   ```

2. Join a channel with password:
   ```
   /join #secret password
   ```

#### Channel Modes

Set various channel modes:

1. Make channel invite-only:
   ```
   /mode #test +i
   ```

2. Set channel password:
   ```
   /mode #test +k secretpass
   ```

3. Restrict topic changes to operators:
   ```
   /mode #test +t
   ```

4. Set user limit (NOTE: Currently being implemented/fixed):
   ```
   /mode #test +l 5
   ```

5. Make channel moderated:
   ```
   /mode #test +m
   ```

6. View current channel modes:
   ```
   /mode #test
   ```

#### Channel Operations

1. Set channel topic:
   ```
   /topic #test New channel topic
   ```

2. Give operator status to another user:
   ```
   /mode #test +o user2
   ```

3. Give voice privileges:
   ```
   /mode #test +v user3
   ```

4. Kick a user:
   ```
   /kick #test user3 Reason for kick
   ```

5. Ban a user (by nickname):
   ```
   /mode #test +b user3
   ```

6. Invite someone to an invite-only channel:
   ```
   /invite user3 #test
   ```

### Direct Messaging

1. Send a private message:
   ```
   /msg user2 Hello, how are you?
   ```

2. Start a private conversation window:
   ```
   /query user2
   ```
   Type message and press Enter to send.

3. Close a query window:
   ```
   /window close
   ```

### Server Operator Features

1. Become server operator (if configured):
   ```
   /oper admin adminpass
   ```

2. Send notice to all users (operators only):
   ```
   /notice #test Important server announcement
   ```

## Testing Sequence for All Functionality

Here's a recommended testing sequence to verify all features:

### Basic Connectivity

1. Connect with user1
2. Connect with user2
3. Connect with user3

### Channel Operations

1. Create a channel:
   ```
   /join #test
   ```

2. Check you're an operator:
   ```
   /mode #test
   ```
   (Should show you as @user1)

3. Invite other users:
   ```
   /invite user2 #test
   /invite user3 #test
   ```

4. Verify members can join:
   ```
   # From user2's client
   /join #test
   ```

### Channel Modes Testing

1. Set channel password:
   ```
   /mode #test +k secretpass
   ```

2. Test password protection:
   ```
   # From a new client
   /join #test           # Should fail
   /join #test secretpass # Should succeed
   ```

3. Set user limit (NOTE: Feature currently being implemented):
   ```
   /mode #test +l 3
   ```

4. Test limit with a new client (Should be implemented soon)

5. Make channel invite-only:
   ```
   /mode #test +i
   ```

6. Test invite-only:
   ```
   # From a new client
   /join #test secretpass # Should fail
   
   # From an existing channel user
   /invite user4 #test
   
   # From user4's client
   /join #test secretpass # Should now work
   ```

7. Set and test topic restriction:
   ```
   /mode #test +t
   
   # From operator
   /topic #test New topic by operator
   
   # From non-operator
   /topic #test New topic by regular user # Should fail
   ```

### User Modes

1. Test operator status:
   ```
   /oper admin adminpass
   ```

2. Give other users privileges:
   ```
   /mode #test +o user2
   ```

### Message Exchange

1. Test channel messaging from different users
2. Test private messaging:
   ```
   /msg user2 Private message test
   ```
3. Test notices:
   ```
   /notice user2 Notice message test
   ```

### Edge Cases

1. Test behavior when a user is kicked and tries to rejoin
2. Test behavior when channel creator leaves
3. Test simultaneous commands from multiple users
4. Test server response to invalid commands

## Comparison with nc (netcat) Testing

For those familiar with netcat (nc), here's a comparison between irssi and nc commands:

| Function | nc Command | irssi Command |
|----------|------------|---------------|
| Connect to server | `nc -C 127.0.0.1 6667` then `PASS password` `NICK user1` `USER u1 0 * :usuario1` | `irssi -c localhost -p 6667 -n user1 -w password` |
| Join channel | `JOIN #test` | `/join #test` |
| Private message | `PRIVMSG user2 :Hello!` | `/msg user2 Hello!` |
| Channel message | `PRIVMSG #test :Hello channel!` | `/msg #test Hello channel!` |
| Change topic | `TOPIC #test New topic` | `/topic #test New topic` |
| Add channel mode | `MODE #test +i` | `/mode #test +i` |
| Make user operator | `MODE #test +o user1` | `/mode #test +o user1` |
| Invite user | `INVITE user1 #test` | `/invite user1 #test` |
| Quit | `QUIT :Goodbye!` | `/quit Goodbye!` |

## Useful irssi Tips

- `/window list`: Show all open windows
- `/window #`: Switch to window number #
- Alt+1, Alt+2, etc.: Switch between windows
- `/names`: Show users in current channel
- `/whois nickname`: Get info about a user
- `/away [message]`: Mark yourself as away
- `/back`: Remove away status

## Testing on Your Local Network

If you want to test the server across your local network:

1. Find your machine's IP address (use `hostname` or `ifconfig` on macOS/Linux)
2. Start the server binding to that IP:
   ```
   ./ircserv 6667 password
   ```
3. Connect from another machine:
   ```
   irssi -c YOUR_MACHINE_IP -p 6667 -n nickname -w password
   ```

## Troubleshooting

- If you can't connect, check server logs for errors
- If commands fail, verify syntax and permissions
- For connection issues, ensure no firewall is blocking port 6667
- If channel operations fail, verify you have operator status

## Known Issues

- The user limit mode (`+l`) is currently being implemented and may not work properly yet.
- Default channel limits are not currently set.

## Complete Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `/join` | Join a channel | `/join #test [password]` |
| `/part` | Leave a channel | `/part #test [message]` |
| `/quit` | Disconnect from server | `/quit [message]` |
| `/msg` | Send private message | `/msg nickname message` |
| `/notice` | Send notice | `/notice #channel message` |
| `/nick` | Change nickname | `/nick newnick` |
| `/topic` | View/change channel topic | `/topic #chan New topic` |
| `/mode` | Set/view modes | `/mode #chan +k pass` |
| `/kick` | Remove user from channel | `/kick #chan user [reason]` |
| `/invite` | Invite user to channel | `/invite user #channel` |
| `/who` | List users in channel | `/who #channel` |
| `/whois` | Get info about user | `/whois nickname` |
| `/oper` | Become server operator | `/oper username password` | 