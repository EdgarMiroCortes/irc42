#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Message.hpp"

// Static member initialization
bool Server::_running = false;

// Signal handler
void Server::_signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        _running = false;
    }
}

/**
 * Default constructor
 */
Server::Server()
    : _name("ft_irc"), _version("1.0"), _port(6667), _password(""), 
      _serverSocket(-1), _totalConnections(0) {
    
    _startTime = time(NULL);
    _creationTime = currentDateTime();
    _running = false;
}

/**
 * Constructor with port and password
 * 
 * @param port Server port
 * @param password Server password
 */
Server::Server(int port, const std::string& password)
    : _name("ft_irc"), _version("1.0"), _port(port), _password(password), 
      _serverSocket(-1), _totalConnections(0) {
    
    // Initialize statistics
    _startTime = time(NULL);
    _creationTime = currentDateTime();
    
    // Initialize command handlers
    _initializeCommandHandlers();
    
    // Set default MOTD
    _motd = "Welcome to ft_irc server. Have a nice chat!";
    
    // Initialize signal handlers
    _running = true;
    signal(SIGINT, _signalHandler);
}

/**
 * Copy constructor
 * 
 * @param other Server to copy
 */
Server::Server(const Server& other) {
    *this = other;
}

/**
 * Destructor
 */
Server::~Server() {
    _cleanup();
}

/**
 * Assignment operator
 * 
 * @param other Server to assign
 * @return Reference to this
 */
Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _name = other._name;
        _version = other._version;
        _host = other._host;
        _port = other._port;
        _password = other._password;
        _creationTime = other._creationTime;
        _motd = other._motd;
        _clients = other._clients;
        _channels = other._channels;
        _operators = other._operators;
        _totalConnections = other._totalConnections;
        _startTime = other._startTime;
        
        // Don't copy socket FD or pollfd vector
    }
    return *this;
}

/**
 * Initialize the server
 * 
 * @return True if initialization was successful, false otherwise
 */
bool Server::initialize() {
    // Set up signal handlers
    signal(SIGINT, _signalHandler);
    signal(SIGTERM, _signalHandler);
    
    // Set up server socket
    _setupServerSocket();
    
    return (_serverSocket != -1);
}

/**
 * Run the server
 */
void Server::run() {
    _running = true;
    
    std::cout << "Server is running on port " << _port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    while (_running) {
        // Wait for events on the poll fds
        int numEvents = poll(&_pollfds[0], _pollfds.size(), 1000); // 1 second timeout
        
        if (numEvents == -1) {
            if (errno == EINTR) {
                // Interrupted by signal, check if we're still running
                continue;
            }
            
            std::cerr << "Error in poll(): " << strerror(errno) << std::endl;
            break;
        }
        
        // Check for events
        for (size_t i = 0; i < _pollfds.size() && numEvents > 0; i++) {
            if (_pollfds[i].revents == 0) {
                continue;
            }
            
            numEvents--;
            
            if (_pollfds[i].fd == _serverSocket) {
                // New connection on the server socket
                if (_pollfds[i].revents & POLLIN) {
                    _acceptNewConnection();
                }
            } else {
                // Activity on a client socket
                if (_pollfds[i].revents & POLLIN) {
                    _handleClientInput(_pollfds[i].fd);
                }
                if (_pollfds[i].revents & POLLOUT) {
                    _handleClientOutput(_pollfds[i].fd);
                }
                if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                    _removeClient(_pollfds[i].fd);
                    i--; // Adjust index since we removed an element
                }
            }
        }
    }
    
    // Clean up before exiting
    _cleanup();
}

/**
 * Shutdown the server
 */
void Server::shutdown() {
    _running = false;
}

/**
 * Gets the server name
 * 
 * @return Server name
 */
std::string Server::getName() const {
    return _name;
}

/**
 * Gets the server version
 * 
 * @return Server version
 */
std::string Server::getVersion() const {
    return _version;
}

/**
 * Gets the server hostname
 * 
 * @return Server hostname
 */
std::string Server::getHost() const {
    return _host;
}

/**
 * Gets the server port
 * 
 * @return Server port
 */
int Server::getPort() const {
    return _port;
}

/**
 * Gets the server password
 * 
 * @return Server password
 */
std::string Server::getPassword() const {
    return _password;
}

/**
 * Gets the server creation time
 * 
 * @return Server creation time
 */
std::string Server::getCreationTime() const {
    return _creationTime;
}

/**
 * Gets the server message of the day
 * 
 * @return Server message of the day
 */
std::string Server::getMotd() const {
    return _motd;
}

/**
 * Gets the clients connected to the server
 * 
 * @return Map of clients (fd -> client)
 */
ClientMap Server::getClients() const {
    return _clients;
}

/**
 * Gets the channels on the server
 * 
 * @return Map of channels (name -> channel)
 */
ChannelMap Server::getChannels() const {
    return _channels;
}

/**
 * Sets the server name
 * 
 * @param name New server name
 */
void Server::setName(const std::string& name) {
    _name = name;
}

/**
 * Sets the server version
 * 
 * @param version New server version
 */
void Server::setVersion(const std::string& version) {
    _version = version;
}

/**
 * Sets the server hostname
 * 
 * @param host New server hostname
 */
void Server::setHost(const std::string& host) {
    _host = host;
}

/**
 * Sets the server port
 * 
 * @param port New server port
 */
void Server::setPort(int port) {
    _port = port;
}

/**
 * Sets the server password
 * 
 * @param password New server password
 */
void Server::setPassword(const std::string& password) {
    _password = password;
}

/**
 * Sets the server message of the day
 * 
 * @param motd New server message of the day
 */
void Server::setMotd(const std::string& motd) {
    _motd = motd;
}

/**
 * Broadcasts a message to all clients
 * 
 * @param message Message to broadcast
 */
void Server::broadcastToAll(const std::string& message) {
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second && it->second->isRegistered()) {
            it->second->addToBuffer(message);
        }
    }
}

/**
 * Sends a message to a client
 * 
 * @param client Client to send to
 * @param message Message to send
 */
void Server::sendToClient(Client* client, const std::string& message) {
    if (client) {
        client->addToBuffer(message);
    }
}

/**
 * Gets the local time
 * 
 * @return Local time string
 */
std::string Server::getLocalTime() const {
    return currentDateTime();
}

/**
 * Gets the server uptime
 * 
 * @return Server uptime string
 */
std::string Server::getUptime() const {
    time_t now = time(NULL);
    time_t uptime = now - _startTime;
    
    int days = uptime / 86400;
    uptime %= 86400;
    int hours = uptime / 3600;
    uptime %= 3600;
    int minutes = uptime / 60;
    int seconds = uptime % 60;
    
    std::stringstream ss;
    ss << days << " days, " << hours << " hours, " << minutes << " minutes, " << seconds << " seconds";
    
    return ss.str();
}

/**
 * Initialize command handlers
 */
void Server::_initializeCommandHandlers() {
    _commandHandlers["PASS"] = &Server::_handlePass;
    _commandHandlers["NICK"] = &Server::_handleNick;
    _commandHandlers["USER"] = &Server::_handleUser;
    _commandHandlers["OPER"] = &Server::_handleOper;
    _commandHandlers["QUIT"] = &Server::_handleQuit;
    _commandHandlers["JOIN"] = &Server::_handleJoin;
    _commandHandlers["PART"] = &Server::_handlePart;
    _commandHandlers["MODE"] = &Server::_handleMode;
    _commandHandlers["TOPIC"] = &Server::_handleTopic;
    _commandHandlers["NAMES"] = &Server::_handleNames;
    _commandHandlers["LIST"] = &Server::_handleList;
    _commandHandlers["INVITE"] = &Server::_handleInvite;
    _commandHandlers["KICK"] = &Server::_handleKick;
    _commandHandlers["PRIVMSG"] = &Server::_handlePrivmsg;
    _commandHandlers["NOTICE"] = &Server::_handleNotice;
    _commandHandlers["PING"] = &Server::_handlePing;
    _commandHandlers["PONG"] = &Server::_handlePong;
    _commandHandlers["WHO"] = &Server::_handleWho;
    _commandHandlers["WHOIS"] = &Server::_handleWhois;
    _commandHandlers["MOTD"] = &Server::_handleMotd;
    _commandHandlers["TIME"] = &Server::_handleTime;
}

/**
 * Set up the server socket
 */
void Server::_setupServerSocket() {
    // Create socket
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
        close(_serverSocket);
        _serverSocket = -1;
        return;
    }
    
    // Set non-blocking mode
    int flags = fcntl(_serverSocket, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
        close(_serverSocket);
        _serverSocket = -1;
        return;
    }
    
    if (fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error setting non-blocking mode: " << strerror(errno) << std::endl;
        close(_serverSocket);
        _serverSocket = -1;
        return;
    }
    
    // Bind socket
    memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
    _serverAddr.sin_port = htons(_port);
    
    if (bind(_serverSocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(_serverSocket);
        _serverSocket = -1;
        return;
    }
    
    // Listen for connections
    if (listen(_serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
        close(_serverSocket);
        _serverSocket = -1;
        return;
    }
    
    // Add server socket to poll fds
    struct pollfd pfd;
    pfd.fd = _serverSocket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    
    // Get hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        _host = hostname;
    } else {
        _host = "localhost";
    }
}

/**
 * Accept new connection
 */
void Server::_acceptNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
        }
        return;
    }
    
    // Set non-blocking mode
    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Error getting client socket flags: " << strerror(errno) << std::endl;
        close(clientFd);
        return;
    }
    
    if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error setting client socket to non-blocking: " << strerror(errno) << std::endl;
        close(clientFd);
        return;
    }
    
    // Add client to poll fds
    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN | POLLOUT;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    
    // Get client hostname
    char hostname[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), hostname, INET_ADDRSTRLEN);
    
    // Create new client
    Client* client = new Client(clientFd, hostname);
    client->setAddr(clientAddr);
    
    // Add client to clients map
    _clients[clientFd] = client;
    
    // Increment connection count
    _totalConnections++;
    
    std::cout << "New connection from " << hostname << " on socket " << clientFd << std::endl;
}

/**
 * Handle client input
 * 
 * @param fd Client file descriptor
 */
void Server::_handleClientInput(int fd) {
    ClientMap::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }
    
    Client* client = it->second;
    int bytes = client->receiveMessage();
    
    if (bytes <= 0) {
        if (bytes == 0 || (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
            _removeClient(fd);
        }
        return;
    }
    
    // Process messages in the input buffer
    std::string inBuffer = client->getInBuffer();
    std::string line;
    std::istringstream iss(inBuffer);
    
    std::string newBuffer;
    while (std::getline(iss, line)) {
        // Check if the line ends with \r
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }
        
        // Process the message
        if (!line.empty()) {
            _processMessage(client, line);
        }
    }
    
    client->clearInBuffer();
}

/**
 * Handle client output
 * 
 * @param fd Client file descriptor
 */
void Server::_handleClientOutput(int fd) {
    ClientMap::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }
    
    Client* client = it->second;
    std::string buffer = client->getBuffer();
    
    if (!buffer.empty()) {
        int bytes = client->sendMessage(buffer);
        
        if (bytes <= 0) {
            if (bytes == 0 || (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                _removeClient(fd);
            }
            return;
        }
        
        client->clearBuffer();
    }
}

/**
 * Process a client message
 * 
 * @param client Client that sent the message
 * @param message Message to process
 */
void Server::_processMessage(Client* client, const std::string& message) {
    if (!client) {
        return;
    }
    
    // Parse the message
    Message msg(message, client);
    
    if (!msg.isValid()) {
        return;
    }
    
    // Log message
    std::cout << "Received from " << client->getFd() << ": " << message << std::endl;
    
    // Handle the command
    std::string command = msg.getCommand();
    
    // Find command handler
    std::map<std::string, CommandHandler>::iterator it = _commandHandlers.find(command);
    if (it != _commandHandlers.end()) {
        // Call the command handler
        (this->*(it->second))(msg);
    } else {
        // Unknown command
        _handleUnknown(msg);
    }
}

/**
 * Register a client when all required information is provided
 * 
 * @param client Client to register
 */
void Server::_registerClient(Client* client) {
    if (!client) {
        return;
    }
    
    // Check if the client has all required information
    if (client->checkRegistration()) {
        // Send welcome messages
        std::string welcomeMsg = _buildReply(RPL_WELCOME, client, "Welcome to the Internet Relay Network " + client->getPrefix());
        std::string hostMsg = _buildReply(RPL_YOURHOST, client, "Your host is " + _name + ", running version " + _version);
        std::string createdMsg = _buildReply(RPL_CREATED, client, "This server was created " + _creationTime);
        std::string myinfoMsg = _buildReply(RPL_MYINFO, client, _name + " " + _version + " iowsraOk ntlmb");
        
        client->addToBuffer(welcomeMsg);
        client->addToBuffer(hostMsg);
        client->addToBuffer(createdMsg);
        client->addToBuffer(myinfoMsg);
        
        // Send MOTD
        if (!_motd.empty()) {
            std::string motdStartMsg = _buildReply(RPL_MOTDSTART, client, "- " + _name + " Message of the Day - ");
            std::string motdMsg = _buildReply(RPL_MOTD, client, "- " + _motd);
            std::string motdEndMsg = _buildReply(RPL_ENDOFMOTD, client, "End of MOTD command");
            
            client->addToBuffer(motdStartMsg);
            client->addToBuffer(motdMsg);
            client->addToBuffer(motdEndMsg);
        } else {
            std::string noMotdMsg = _buildReply(ERR_NOMOTD, client, "MOTD File is missing");
            client->addToBuffer(noMotdMsg);
        }
        
        std::cout << "Client " << client->getFd() << " (" << client->getNickname() << ") registered" << std::endl;
    }
}

/**
 * Remove a client from the server
 * 
 * @param fd Client file descriptor
 */
void Server::_removeClient(int fd) {
    ClientMap::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }
    
    Client* client = it->second;
    
    // Remove client from channels
    client->leaveAllChannels();
    
    // Remove client from clients map
    _clients.erase(it);
    
    // Remove client from poll fds
    for (std::vector<struct pollfd>::iterator pfdIt = _pollfds.begin(); pfdIt != _pollfds.end(); ++pfdIt) {
        if (pfdIt->fd == fd) {
            _pollfds.erase(pfdIt);
            break;
        }
    }
    
    // Close socket
    close(fd);
    
    // Log
    std::cout << "Client " << fd << " disconnected" << std::endl;
    
    // Delete client
    delete client;
}

/**
 * Clean up server resources
 */
void Server::_cleanup() {
    // Close all client connections
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    _clients.clear();
    
    // Delete all channels
    for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
    _channels.clear();
    
    // Close server socket
    if (_serverSocket != -1) {
        close(_serverSocket);
        _serverSocket = -1;
    }
    
    // Clear poll fds
    _pollfds.clear();
}

/**
 * Authenticate a client
 * 
 * @param client Client to authenticate
 * @param password Password to check
 * @return True if the client is authenticated, false otherwise
 */
bool Server::_authenticateClient(Client* client, const std::string& password) {
    if (!client) {
        return false;
    }
    
    // Check if password is required
    if (_password.empty()) {
        client->setAuthenticated(true);
        return true;
    }
    
    // Check password
    if (password == _password) {
        client->setAuthenticated(true);
        return true;
    }
    
    return false;
}

/**
 * Get a client by nickname
 * 
 * @param nickname Nickname to look for
 * @return Pointer to the client, or NULL if not found
 */
Client* Server::_getClientByNickname(const std::string& nickname) {
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second && it->second->getNickname() == nickname) {
            return it->second;
        }
    }
    
    return NULL;
}

/**
 * Get a channel by name
 * 
 * @param name Channel name
 * @return Pointer to the channel, or NULL if not found
 */
Channel* Server::_getChannel(const std::string& name) {
    ChannelMap::iterator it = _channels.find(toLower(name));
    if (it != _channels.end()) {
        return it->second;
    }
    
    return NULL;
}

/**
 * Create a new channel
 * 
 * @param name Channel name
 * @param creator Channel creator
 * @param password Channel password
 * @return Pointer to the new channel
 */
Channel* Server::_createChannel(const std::string& name, Client* creator, const std::string& password) {
    Channel* channel = new Channel(name, creator, password);
    
    // If a password was provided, set the channel mode to +k
    if (!password.empty()) {
        channel->setMode('k', true, password);
    }
    
    _channels[toLower(name)] = channel;
    
    return channel;
}

/**
 * Remove a channel
 * 
 * @param name Channel name
 */
void Server::_removeChannel(const std::string& name) {
    ChannelMap::iterator it = _channels.find(toLower(name));
    if (it != _channels.end()) {
        delete it->second;
        _channels.erase(it);
    }
}

/**
 * Build a reply message
 * 
 * @param code Numeric reply code
 * @param client Client to send to
 * @param message Message content
 * @return Formatted reply message
 */
std::string Server::_buildReply(const std::string& code, Client* client, const std::string& message) {
    if (!client) {
        return "";
    }
    
    std::string reply = ":" + _name + " " + code + " " + client->getNickname() + " " + message;
    
    return reply;
}

/**
 * Build a message
 * 
 * @param command Command
 * @param target Target
 * @param message Message content
 * @return Formatted message
 */
std::string Server::_buildMessage(const std::string& command, const std::string& target, const std::string& message) {
    std::string msg = ":" + _name + " " + command + " " + target + " :" + message;
    
    return msg;
} 