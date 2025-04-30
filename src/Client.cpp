#include "../include/Client.hpp"
#include "../include/Channel.hpp"

/**
 * Default constructor
 */
Client::Client() 
    : _fd(-1), _status(CONNECTED), _authenticated(false), _registered(false), 
      _isOper(false), _commandProcessed(false) {
    
    // Initialize all modes to false
    _modes.away = false;
    _modes.invisible = false;
    _modes.wallops = false;
    _modes.restricted = false;
    _modes.operator_ = false;
    _modes.localOp = false;
    _modes.notices = false;
    
    // Set last activity to current time
    _lastActivity = time(NULL);
}

/**
 * Constructor with file descriptor and hostname
 * 
 * @param fd Client socket file descriptor
 * @param hostname Client hostname or IP address
 */
Client::Client(int fd, const std::string& hostname) 
    : _fd(fd), _status(CONNECTED), _hostname(hostname), _authenticated(false), 
      _registered(false), _isOper(false), _commandProcessed(false) {
    
    // Initialize all modes to false
    _modes.away = false;
    _modes.invisible = false;
    _modes.wallops = false;
    _modes.restricted = false;
    _modes.operator_ = false;
    _modes.localOp = false;
    _modes.notices = false;
    
    // Set last activity to current time
    _lastActivity = time(NULL);
}

/**
 * Copy constructor
 * 
 * @param other Client to copy
 */
Client::Client(const Client& other) {
    *this = other;
}

/**
 * Destructor
 */
Client::~Client() {
    // Nothing to clean up as channels are managed by the server
}

/**
 * Assignment operator
 * 
 * @param other Client to assign
 * @return Reference to this
 */
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _fd = other._fd;
        _status = other._status;
        _nickname = other._nickname;
        _username = other._username;
        _realname = other._realname;
        _hostname = other._hostname;
        _buffer = other._buffer;
        _inBuffer = other._inBuffer;
        _addr = other._addr;
        _channels = other._channels;
        _modes = other._modes;
        _authenticated = other._authenticated;
        _registered = other._registered;
        _isOper = other._isOper;
        _lastActivity = other._lastActivity;
        _commandProcessed = other._commandProcessed;
    }
    return *this;
}

/**
 * Gets the file descriptor
 * 
 * @return File descriptor
 */
int Client::getFd() const {
    return _fd;
}

/**
 * Gets the status
 * 
 * @return Status
 */
int Client::getStatus() const {
    return _status;
}

/**
 * Gets the nickname
 * 
 * @return Nickname
 */
std::string Client::getNickname() const {
    return _nickname;
}

/**
 * Gets the username
 * 
 * @return Username
 */
std::string Client::getUsername() const {
    return _username;
}

/**
 * Gets the real name
 * 
 * @return Real name
 */
std::string Client::getRealname() const {
    return _realname;
}

/**
 * Gets the hostname
 * 
 * @return Hostname
 */
std::string Client::getHostname() const {
    return _hostname;
}

/**
 * Gets the client prefix (nickname!username@hostname)
 * 
 * @return Client prefix
 */
std::string Client::getPrefix() const {
    if (_nickname.empty() || _username.empty() || _hostname.empty()) {
        return "";
    }
    return _nickname + "!" + _username + "@" + _hostname;
}

/**
 * Gets the output buffer
 * 
 * @return Output buffer
 */
std::string Client::getBuffer() const {
    return _buffer;
}

/**
 * Gets the input buffer
 * 
 * @return Input buffer
 */
std::string Client::getInBuffer() const {
    return _inBuffer;
}

/**
 * Gets the channels the client is in
 * 
 * @return Map of channels (name -> channel)
 */
std::map<std::string, Channel*> Client::getChannels() const {
    return _channels;
}

/**
 * Gets the client address
 * 
 * @return Client address
 */
struct sockaddr_in Client::getAddr() const {
    return _addr;
}

/**
 * Checks if the client is authenticated
 * 
 * @return True if authenticated, false otherwise
 */
bool Client::isAuthenticated() const {
    return _authenticated;
}

/**
 * Checks if the client is registered
 * 
 * @return True if registered, false otherwise
 */
bool Client::isRegistered() const {
    return _registered;
}

/**
 * Checks if the client is an operator
 * 
 * @return True if operator, false otherwise
 */
bool Client::isOperator() const {
    return _isOper;
}

/**
 * Checks if the client has a specific mode
 * 
 * @param mode Mode to check
 * @return True if the client has the mode, false otherwise
 */
bool Client::hasMode(char mode) const {
    switch (mode) {
        case 'a': return _modes.away;
        case 'i': return _modes.invisible;
        case 'w': return _modes.wallops;
        case 'r': return _modes.restricted;
        case 'o': return _modes.operator_;
        case 'O': return _modes.localOp;
        case 's': return _modes.notices;
        default: return false;
    }
}

/**
 * Gets the last activity time
 * 
 * @return Last activity time
 */
time_t Client::getLastActivity() const {
    return _lastActivity;
}

/**
 * Checks if the command has been processed
 * 
 * @return True if the command has been processed, false otherwise
 */
bool Client::isCommandProcessed() const {
    return _commandProcessed;
}

/**
 * Sets the file descriptor
 * 
 * @param fd File descriptor
 */
void Client::setFd(int fd) {
    _fd = fd;
}

/**
 * Sets the status
 * 
 * @param status New status
 */
void Client::setStatus(int status) {
    _status = status;
}

/**
 * Sets the nickname
 * 
 * @param nickname New nickname
 */
void Client::setNickname(const std::string& nickname) {
    _nickname = nickname;
}

/**
 * Sets the username
 * 
 * @param username New username
 */
void Client::setUsername(const std::string& username) {
    _username = username;
}

/**
 * Sets the real name
 * 
 * @param realname New real name
 */
void Client::setRealname(const std::string& realname) {
    _realname = realname;
}

/**
 * Sets the hostname
 * 
 * @param hostname New hostname
 */
void Client::setHostname(const std::string& hostname) {
    _hostname = hostname;
}

/**
 * Sets the client address
 * 
 * @param addr New client address
 */
void Client::setAddr(struct sockaddr_in addr) {
    _addr = addr;
}

/**
 * Sets the authentication status
 * 
 * @param authenticated New authentication status
 */
void Client::setAuthenticated(bool authenticated) {
    _authenticated = authenticated;
}

/**
 * Sets the registration status
 * 
 * @param registered New registration status
 */
void Client::setRegistered(bool registered) {
    _registered = registered;
}

/**
 * Sets the operator status
 * 
 * @param oper New operator status
 */
void Client::setOperator(bool oper) {
    _isOper = oper;
    // Also set the operator mode
    _modes.operator_ = oper;
}

/**
 * Sets a mode
 * 
 * @param mode Mode to set
 * @param value Value to set the mode to
 */
void Client::setMode(char mode, bool value) {
    switch (mode) {
        case 'a': _modes.away = value; break;
        case 'i': _modes.invisible = value; break;
        case 'w': _modes.wallops = value; break;
        case 'r': _modes.restricted = value; break;
        case 'o': _modes.operator_ = value; _isOper = value; break;
        case 'O': _modes.localOp = value; break;
        case 's': _modes.notices = value; break;
    }
}

/**
 * Sets the last activity time
 * 
 * @param time New last activity time
 */
void Client::setLastActivity(time_t time) {
    _lastActivity = time;
}

/**
 * Sets the command processed status
 * 
 * @param processed New command processed status
 */
void Client::setCommandProcessed(bool processed) {
    _commandProcessed = processed;
}

/**
 * Adds a message to the output buffer
 * 
 * @param message Message to add
 */
void Client::addToBuffer(const std::string& message) {
    _buffer += message;
    if (message.length() >= 2 && message.substr(message.length() - 2) != "\r\n") {
        _buffer += "\r\n";
    }
}

/**
 * Adds a message to the input buffer
 * 
 * @param message Message to add
 */
void Client::addToInBuffer(const std::string& message) {
    _inBuffer += message;
}

/**
 * Clears the output buffer
 */
void Client::clearBuffer() {
    _buffer.clear();
}

/**
 * Clears the input buffer
 */
void Client::clearInBuffer() {
    _inBuffer.clear();
}

/**
 * Joins a channel
 * 
 * @param channel Channel to join
 */
void Client::joinChannel(Channel* channel) {
    if (channel) {
        _channels[channel->getName()] = channel;
    }
}

/**
 * Leaves a channel
 * 
 * @param channelName Name of the channel to leave
 */
void Client::leaveChannel(const std::string& channelName) {
    _channels.erase(channelName);
}

/**
 * Leaves all channels
 */
void Client::leaveAllChannels() {
    // Notify all channels that the client is leaving
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (it->second) {
            it->second->removeClient(this);
        }
    }
    
    // Clear the channels map
    _channels.clear();
}

/**
 * Checks if the client is in a channel
 * 
 * @param channelName Name of the channel to check
 * @return True if the client is in the channel, false otherwise
 */
bool Client::isInChannel(const std::string& channelName) const {
    return _channels.find(channelName) != _channels.end();
}

/**
 * Sends a message to the client
 * 
 * @param message Message to send
 * @return Number of bytes sent, or -1 on error
 */
int Client::sendMessage(const std::string& message) {
    // Update last activity time
    _lastActivity = time(NULL);
    
    std::string msg = message;
    // Ensure message ends with CRLF
    if (msg.length() < 2 || msg.substr(msg.length() - 2) != "\r\n") {
        msg += "\r\n";
    }
    
    return send(_fd, msg.c_str(), msg.length(), 0);
}

/**
 * Receives a message from the client
 * 
 * @return Number of bytes received, or -1 on error
 */
int Client::receiveMessage() {
    char buffer[BUFFER_SIZE];
    int bytes = recv(_fd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        addToInBuffer(buffer);
        
        // Update last activity time
        _lastActivity = time(NULL);
    }
    
    return bytes;
}

/**
 * Increments the registration status
 */
void Client::incrementStatus() {
    if (_status < REGISTERED) {
        _status++;
    }
}

/**
 * Checks if the client is fully registered
 * 
 * @return True if the client is fully registered, false otherwise
 */
bool Client::checkRegistration() {
    // A client is fully registered when they:
    // 1. Have provided a valid password (if required)
    // 2. Have set a nickname
    // 3. Have set a username and real name
    if (_authenticated && !_nickname.empty() && !_username.empty() && !_realname.empty()) {
        _registered = true;
        _status = REGISTERED;
        return true;
    }
    return false;
} 