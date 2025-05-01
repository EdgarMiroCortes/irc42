#include "../include/Channel.hpp"
#include "../include/Client.hpp"

/**
 * Default constructor
 */
Channel::Channel()
    : _inviteOnly(false), _topicRestricted(false), _noExternalMessages(true), 
      _moderated(false), _userLimit(0) { // Default user limit is 0 (unlimited)
    
    _creationTime = time(NULL);
}

/**
 * Constructor with channel name, creator, and optional password
 * 
 * @param name Channel name
 * @param creator Channel creator
 * @param password Optional channel password
 */
Channel::Channel(const std::string& name, Client* creator, const std::string& password)
    : _name(name), _password(password), _inviteOnly(false), _topicRestricted(false), 
      _noExternalMessages(true), _moderated(false), _userLimit(0) { // Default user limit is 0 (unlimited)
    
    _creationTime = time(NULL);
    
    // Add creator as first client and operator
    if (creator) {
        addClient(creator);
        addOperator(creator->getNickname());
    }
}

/**
 * Copy constructor
 * 
 * @param other Channel to copy
 */
Channel::Channel(const Channel& other) {
    *this = other;
}

/**
 * Destructor
 */
Channel::~Channel() {
    // Nothing to clean up as clients are managed by the server
}

/**
 * Assignment operator
 * 
 * @param other Channel to assign
 * @return Reference to this
 */
Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _topic = other._topic;
        _password = other._password;
        _clients = other._clients;
        _operators = other._operators;
        _voiced = other._voiced;
        _banned = other._banned;
        _invited = other._invited;
        _inviteOnly = other._inviteOnly;
        _topicRestricted = other._topicRestricted;
        _noExternalMessages = other._noExternalMessages;
        _moderated = other._moderated;
        _userLimit = other._userLimit;
        _creationTime = other._creationTime;
    }
    return *this;
}

/**
 * Gets the channel name
 * 
 * @return Channel name
 */
std::string Channel::getName() const {
    return _name;
}

/**
 * Gets the channel topic
 * 
 * @return Channel topic
 */
std::string Channel::getTopic() const {
    return _topic;
}

/**
 * Gets the channel password
 * 
 * @return Channel password
 */
std::string Channel::getPassword() const {
    return _password;
}

/**
 * Gets the clients in the channel
 * 
 * @return Map of clients (nickname -> client)
 */
std::map<std::string, Client*> Channel::getClients() const {
    return _clients;
}

/**
 * Gets the channel operators
 * 
 * @return Set of operator nicknames
 */
std::set<std::string> Channel::getOperators() const {
    return _operators;
}

/**
 * Gets the voiced users
 * 
 * @return Set of voiced user nicknames
 */
std::set<std::string> Channel::getVoiced() const {
    return _voiced;
}

/**
 * Gets the banned users
 * 
 * @return Set of banned user masks
 */
std::set<std::string> Channel::getBanned() const {
    return _banned;
}

/**
 * Gets the invited users
 * 
 * @return Set of invited user nicknames
 */
std::set<std::string> Channel::getInvited() const {
    return _invited;
}

/**
 * Checks if the channel is invite-only
 * 
 * @return True if the channel is invite-only, false otherwise
 */
bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

/**
 * Checks if the channel topic is restricted
 * 
 * @return True if only operators can change the topic, false otherwise
 */
bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

/**
 * Checks if the channel blocks external messages
 * 
 * @return True if external messages are blocked, false otherwise
 */
bool Channel::hasNoExternalMessages() const {
    return _noExternalMessages;
}

/**
 * Checks if the channel is moderated
 * 
 * @return True if the channel is moderated, false otherwise
 */
bool Channel::isModerated() const {
    return _moderated;
}

/**
 * Gets the channel user limit
 * 
 * @return Channel user limit, or 0 if no limit
 */
unsigned int Channel::getUserLimit() const {
    return _userLimit;
}

/**
 * Gets the channel creation time
 * 
 * @return Channel creation time
 */
time_t Channel::getCreationTime() const {
    return _creationTime;
}

/**
 * Gets the channel modes
 * 
 * @return String of channel modes
 */
std::string Channel::getModes() const {
    std::string modes;
    
    if (_inviteOnly) {
        modes += "i";
    }
    if (_topicRestricted) {
        modes += "t";
    }
    if (_noExternalMessages) {
        modes += "n";
    }
    if (_moderated) {
        modes += "m";
    }
    if (!_password.empty()) {
        modes += "k";
    }
    if (_userLimit > 0) {
        modes += "l";
    }
    
    return modes;
}

/**
 * Gets the channel mode string
 * 
 * @return Mode string with parameters
 */
std::string Channel::getModeString() const {
    std::string modeString = "+" + getModes();
    std::string params;
    
    if (!_password.empty()) {
        params += " " + _password;
    }
    if (_userLimit > 0) {
        std::stringstream ss;
        ss << _userLimit;
        params += " " + ss.str();
    }
    
    return modeString + params;
}

/**
 * Sets the channel topic
 * 
 * @param topic New channel topic
 */
void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

/**
 * Sets the channel password
 * 
 * @param password New channel password
 */
void Channel::setPassword(const std::string& password) {
    _password = password;
}

/**
 * Sets the invite-only mode
 * 
 * @param inviteOnly New invite-only mode
 */
void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

/**
 * Sets the topic restricted mode
 * 
 * @param topicRestricted New topic restricted mode
 */
void Channel::setTopicRestricted(bool topicRestricted) {
    _topicRestricted = topicRestricted;
}

/**
 * Sets the no external messages mode
 * 
 * @param noExternalMessages New no external messages mode
 */
void Channel::setNoExternalMessages(bool noExternalMessages) {
    _noExternalMessages = noExternalMessages;
}

/**
 * Sets the moderated mode
 * 
 * @param moderated New moderated mode
 */
void Channel::setModerated(bool moderated) {
    _moderated = moderated;
}

/**
 * Sets the user limit
 * 
 * @param userLimit New user limit
 */
void Channel::setUserLimit(unsigned int userLimit) {
    // Update the limit
    _userLimit = userLimit;
    
    // Debug log
    if (userLimit > 0) {
        std::cout << "[DEBUG] Channel " << _name << " user limit set to " << userLimit << std::endl;
    } else {
        std::cout << "[DEBUG] Channel " << _name << " user limit removed" << std::endl;
    }
}

/**
 * Adds a client to the channel
 * 
 * @param client Client to add
 * @return True if the client was added, false otherwise
 */
bool Channel::addClient(Client* client) {
    if (!client) {
        return false;
    }
    
    // Check if client is already in the channel
    if (isClient(client)) {
        std::cout << "[DEBUG] Client " << client->getNickname() << " is already in the channel." << std::endl;
        return false;
    }
    
    // Check if the client is banned
    if (isBanned(client)) {
        std::cout << "[DEBUG] Client " << client->getNickname() << " is banned." << std::endl;
        return false;
    }
    
    // Note: User limit check is now done in the JOIN handler
    
    // Add client to channel
    _clients[client->getNickname()] = client;
    
    // Add channel to client's channels
    client->joinChannel(this);
    
    std::cout << "[DEBUG] " << client->getNickname() << " joined " << _name 
              << ". Members: " << _clients.size() << std::endl;

    return true;
}

/**
 * Removes a client from the channel
 * 
 * @param client Client to remove
 * @return True if the client was removed, false otherwise
 */
bool Channel::removeClient(Client* client) {
    if (!client || !isClient(client)) {
        return false;
    }
    
    // // Remove client from channel
    // _clients.erase(client->getNickname());
    
    // // Remove client from operators and voiced users if applicable
    // _operators.erase(client->getNickname());
    // _voiced.erase(client->getNickname());
    
    // // Remove channel from client's channels
    // client->leaveChannel(_name);
    
    // return true;
    
    std::string nick = client->getNickname();
    std::string partMsg = ":" + nick + " PART " + _name;

    // Notificar al cliente y al canal
    client->sendMessage(partMsg);
    broadcastMessage(partMsg, client);

    // Eliminar de listas internas
    _clients.erase(nick);
    _operators.erase(nick);
    _voiced.erase(nick);
    client->leaveChannel(_name);

    std::cout << "[DEBUG] " << nick << " dejó el canal " << _name << std::endl;
    return true;
}

/**
 * Checks if a client is in the channel by nickname
 * 
 * @param nickname Client nickname
 * @return True if the client is in the channel, false otherwise
 */
bool Channel::isClient(const std::string& nickname) const {
    return _clients.find(nickname) != _clients.end();
}

/**
 * Checks if a client is in the channel
 * 
 * @param client Client to check
 * @return True if the client is in the channel, false otherwise
 */
bool Channel::isClient(Client* client) const {
    if (!client) {
        return false;
    }
    return isClient(client->getNickname());
}

/**
 * Adds an operator to the channel
 * 
 * @param nickname Operator nickname
 * @return True if the operator was added, false otherwise
 */
bool Channel::addOperator(const std::string& nickname) {
    // Check if the client is in the channel
    if (!isClient(nickname)) {
        return false;
    }
    
    // Add operator
    _operators.insert(nickname);
    
    return true;
}

/**
 * Removes an operator from the channel
 * 
 * @param nickname Operator nickname
 * @return True if the operator was removed, false otherwise
 */
bool Channel::removeOperator(const std::string& nickname) {
    return _operators.erase(nickname) > 0;
}

/**
 * Checks if a client is an operator by nickname
 * 
 * @param nickname Client nickname
 * @return True if the client is an operator, false otherwise
 */
bool Channel::isOperator(const std::string& nickname) const {
    return _operators.find(nickname) != _operators.end();
}

/**
 * Checks if a client is an operator
 * 
 * @param client Client to check
 * @return True if the client is an operator, false otherwise
 */
bool Channel::isOperator(Client* client) const {
    if (!client) {
        return false;
    }
    return isOperator(client->getNickname());
}

/**
 * Adds a voice to a client
 * 
 * @param nickname Client nickname
 * @return True if the voice was added, false otherwise
 */
bool Channel::addVoice(const std::string& nickname) {
    // Check if the client is in the channel
    if (!isClient(nickname)) {
        return false;
    }
    
    // Add voice
    _voiced.insert(nickname);
    
    return true;
}

/**
 * Removes a voice from a client
 * 
 * @param nickname Client nickname
 * @return True if the voice was removed, false otherwise
 */
bool Channel::removeVoice(const std::string& nickname) {
    return _voiced.erase(nickname) > 0;
}

/**
 * Checks if a client has voice by nickname
 * 
 * @param nickname Client nickname
 * @return True if the client has voice, false otherwise
 */
bool Channel::hasVoice(const std::string& nickname) const {
    return _voiced.find(nickname) != _voiced.end();
}

/**
 * Checks if a client has voice
 * 
 * @param client Client to check
 * @return True if the client has voice, false otherwise
 */
bool Channel::hasVoice(Client* client) const {
    if (!client) {
        return false;
    }
    return hasVoice(client->getNickname());
}

/**
 * Adds a ban mask
 * 
 * @param mask Ban mask
 * @return True if the ban was added, false otherwise
 */
bool Channel::addBan(const std::string& mask) {
    _banned.insert(mask);
    return true;
}

/**
 * Removes a ban mask
 * 
 * @param mask Ban mask
 * @return True if the ban was removed, false otherwise
 */
bool Channel::removeBan(const std::string& mask) {
    return _banned.erase(mask) > 0;
}

/**
 * Checks if a client is banned by nickname
 * 
 * @param nickname Client nickname
 * @return True if the client is banned, false otherwise
 */
bool Channel::isBanned(const std::string& nickname) const {
    // Simple implementation - just check if the nickname is in the banned set
    // A more sophisticated implementation would check masks (nick!user@host)
    return _banned.find(nickname) != _banned.end();
}

/**
 * Checks if a client is banned
 * 
 * @param client Client to check
 * @return True if the client is banned, false otherwise
 */
bool Channel::isBanned(Client* client) const {
    if (!client) {
        return false;
    }
    
    // Check nickname
    if (isBanned(client->getNickname())) {
        return true;
    }
    
    // Check full prefix (nick!user@host)
    if (!client->getPrefix().empty() && _banned.find(client->getPrefix()) != _banned.end()) {
        return true;
    }
    
    // TODO: Implement wildcard matching for ban masks
    
    return false;
}

/**
 * Adds an invite
 * 
 * @param nickname Client nickname
 * @return True if the invite was added, false otherwise
 */
bool Channel::addInvite(const std::string& nickname) {
    _invited.insert(nickname);
    return true;
}

/**
 * Removes an invite
 * 
 * @param nickname Client nickname
 * @return True if the invite was removed, false otherwise
 */
bool Channel::removeInvite(const std::string& nickname) {
    return _invited.erase(nickname) > 0;
}

/**
 * Checks if a client is invited by nickname
 * 
 * @param nickname Client nickname
 * @return True if the client is invited, false otherwise
 */
bool Channel::isInvited(const std::string& nickname) const {
    return _invited.find(nickname) != _invited.end();
}

/**
 * Checks if a client is invited
 * 
 * @param client Client to check
 * @return True if the client is invited, false otherwise
 */
bool Channel::isInvited(Client* client) const {
    if (!client) {
        return false;
    }
    return isInvited(client->getNickname());
}

/**
 * Sets a channel mode
 * 
 * @param mode Mode to set
 * @param add True to add the mode, false to remove it
 * @param param Mode parameter
 * @return True if the mode was set, false otherwise
 */
bool Channel::setMode(char mode, bool add, const std::string& param) {
    switch (mode) {
        case 'i': // Invite-only
            setInviteOnly(add);
            return true;
        
        case 't': // Topic restricted
            setTopicRestricted(add);
            return true;
        
        case 'n': // No external messages
            setNoExternalMessages(add);
            return true;
        
        case 'm': // Moderated
            setModerated(add);
            return true;
        
        case 'k': // Key (password)
            if (add) {
                if (param.empty()) {
                    return false;
                }
                setPassword(param);
            } else {
                setPassword("");
            }
            return true;
        
        case 'l': // User limit
            if (add) {
                if (param.empty()) {
                    return false;
                }
                try {
                    // Check if the parameter is a valid number
                    for (size_t i = 0; i < param.length(); i++) {
                        if (!isdigit(param[i])) {
                            return false;
                        }
                    }
                    
                    unsigned int limit = static_cast<unsigned int>(atoi(param.c_str()));
                    // Validate that limit is reasonable (>0 and not too large)
                    if (limit == 0 || limit > 1000) {
                        return false;
                    }
                    setUserLimit(limit);
                } catch (const std::exception& e) {
                    return false;
                }
            } else {
                setUserLimit(0); // Remove the limit
            }
            return true;
        
        case 'o': // Operator
            if (param.empty()) {
                return false;
            }
            if (add) {
                return addOperator(param);
            } else {
                return removeOperator(param);
            }
        
        case 'v': // Voice
            if (param.empty()) {
                return false;
            }
            if (add) {
                return addVoice(param);
            } else {
                return removeVoice(param);
            }
        
        case 'b': // Ban
            if (param.empty()) {
                return false;
            }
            if (add) {
                return addBan(param);
            } else {
                return removeBan(param);
            }
        
        default:
            return false;
    }
}

/**
 * Broadcasts a message to all clients in the channel
 * 
 * @param message Message to broadcast
 * @param exclude Client to exclude from the broadcast
 */
void Channel::broadcastMessage(const std::string& message, Client* exclude) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second && it->second != exclude) {
            it->second->sendMessage(message);
        }
    }
}

/**
 * Gets a list of clients in the channel
 * 
 * @return Space-separated list of clients with prefixes
 */
std::string Channel::getClientsList() const {
    std::string list;
    
    for (std::map<std::string, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it != _clients.begin()) {
            list += " ";
        }
        
        // Add prefix symbols
        if (isOperator(it->first)) {
            list += "@";
        } else if (hasVoice(it->first)) {
            list += "+";
        }
        
        list += it->first;
    }
    
    return list;
}

/**
 * Gets the number of clients in the channel
 * 
 * @return Number of clients
 */
size_t Channel::getClientCount() const {
    return _clients.size();
}

/**
 * Gets the old mode parameter for a mode
 * 
 * @param mode Mode
 * @return Mode parameter
 */
std::string Channel::getOldModeParams(char mode) const {
    switch (mode) {
        case 'k':
            return _password;
        
        case 'l':
            if (_userLimit > 0) {
                std::stringstream ss;
                ss << _userLimit;
                return ss.str();
            }
            return "";
        
        default:
            return "";
    }
} 