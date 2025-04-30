#include "../include/Message.hpp"
#include "../include/Client.hpp"

/**
 * Default constructor
 */
Message::Message() : _prefix(""), _command(""), _sender(NULL) {}

/**
 * Constructor with raw message and sender
 * 
 * @param rawMessage The raw IRC message
 * @param sender The client who sent the message
 */
Message::Message(const std::string& rawMessage, Client* sender) : _rawMessage(rawMessage), _sender(sender) {
    parse(rawMessage);
}

/**
 * Copy constructor
 * 
 * @param other Message to copy
 */
Message::Message(const Message& other) {
    *this = other;
}

/**
 * Destructor
 */
Message::~Message() {}

/**
 * Assignment operator
 * 
 * @param other Message to assign
 * @return Reference to this
 */
Message& Message::operator=(const Message& other) {
    if (this != &other) {
        _prefix = other._prefix;
        _command = other._command;
        _params = other._params;
        _rawMessage = other._rawMessage;
        _sender = other._sender;
    }
    return *this;
}

/**
 * Gets the message prefix
 * 
 * @return Message prefix
 */
std::string Message::getPrefix() const {
    return _prefix;
}

/**
 * Gets the command
 * 
 * @return Command
 */
std::string Message::getCommand() const {
    return _command;
}

/**
 * Gets all parameters
 * 
 * @return Vector of parameters
 */
std::vector<std::string> Message::getParams() const {
    return _params;
}

/**
 * Gets a parameter by index
 * 
 * @param index Index of parameter
 * @return Parameter at index, or empty string if index is out of range
 */
std::string Message::getParam(size_t index) const {
    if (index < _params.size()) {
        return _params[index];
    }
    return "";
}

/**
 * Gets the trailing parameter (the part after the colon)
 * 
 * @return Trailing parameter, or empty string if no trailing parameter
 */
std::string Message::getTrailing() const {
    if (hasTrailing() && !_params.empty()) {
        return _params.back();
    }
    return "";
}

/**
 * Gets the raw message
 * 
 * @return Raw message
 */
std::string Message::getRawMessage() const {
    return _rawMessage;
}

/**
 * Gets the sender client
 * 
 * @return Pointer to sender client
 */
Client* Message::getSender() const {
    return _sender;
}

/**
 * Sets the prefix
 * 
 * @param prefix Message prefix
 */
void Message::setPrefix(const std::string& prefix) {
    _prefix = prefix;
}

/**
 * Sets the command
 * 
 * @param command Command
 */
void Message::setCommand(const std::string& command) {
    _command = command;
}

/**
 * Sets all parameters
 * 
 * @param params Vector of parameters
 */
void Message::setParams(const std::vector<std::string>& params) {
    _params = params;
}

/**
 * Adds a parameter
 * 
 * @param param Parameter to add
 */
void Message::addParam(const std::string& param) {
    _params.push_back(param);
}

/**
 * Sets the raw message
 * 
 * @param rawMessage Raw message
 */
void Message::setRawMessage(const std::string& rawMessage) {
    _rawMessage = rawMessage;
}

/**
 * Sets the sender client
 * 
 * @param sender Pointer to sender client
 */
void Message::setSender(Client* sender) {
    _sender = sender;
}

/**
 * Parses a raw message into prefix, command, and parameters
 * 
 * @param rawMessage Raw message to parse
 * @return True if parsing was successful, false otherwise
 */
bool Message::parse(const std::string& rawMessage) {
    _rawMessage = rawMessage;
    _prefix = "";
    _command = "";
    _params.clear();
    
    std::string message = rawMessage;
    
    // Remove CR and LF from the end of the message
    if (!message.empty() && message[message.length() - 1] == '\n') {
        message.erase(message.length() - 1);
    }
    if (!message.empty() && message[message.length() - 1] == '\r') {
        message.erase(message.length() - 1);
    }
    
    // Check if message is empty
    if (message.empty()) {
        return false;
    }
    
    // Parse prefix if present
    size_t start = 0;
    if (message[0] == ':') {
        size_t prefixEnd = message.find(' ');
        if (prefixEnd == std::string::npos) {
            return false;
        }
        _prefix = message.substr(1, prefixEnd - 1);
        start = prefixEnd + 1;
        
        // Skip any additional spaces
        while (start < message.length() && message[start] == ' ') {
            start++;
        }
        
        if (start >= message.length()) {
            return false;
        }
    }
    
    // Parse command
    size_t commandEnd = message.find(' ', start);
    if (commandEnd == std::string::npos) {
        _command = message.substr(start);
        std::transform(_command.begin(), _command.end(), _command.begin(), ::toupper);
        return true;
    }
    
    _command = message.substr(start, commandEnd - start);
    std::transform(_command.begin(), _command.end(), _command.begin(), ::toupper);
    
    // Skip any additional spaces
    start = commandEnd + 1;
    while (start < message.length() && message[start] == ' ') {
        start++;
    }
    
    if (start >= message.length()) {
        return true;
    }
    
    // Parse parameters
    size_t trailing = message.find(" :", start);
    if (trailing != std::string::npos) {
        // Parse middle parameters
        std::string middle = message.substr(start, trailing - start);
        if (!middle.empty()) {
            std::istringstream iss(middle);
            std::string param;
            while (iss >> param) {
                _params.push_back(param);
            }
        }
        
        // Parse trailing parameter
        std::string trailing_param = message.substr(trailing + 2);
        _params.push_back(trailing_param);
    } else {
        // Parse all parameters as middle parameters
        std::istringstream iss(message.substr(start));
        std::string param;
        while (iss >> param) {
            if (param[0] == ':') {
                // If a parameter starts with :, it's the trailing parameter
                param = param.substr(1);
                std::string trailing_param = param;
                
                // Get the rest of the message as part of the trailing parameter
                std::string rest;
                std::getline(iss, rest);
                if (!rest.empty()) {
                    trailing_param += rest;
                }
                
                _params.push_back(trailing_param);
                break;
            }
            _params.push_back(param);
        }
    }
    
    return true;
}

/**
 * Checks if the message is valid
 * 
 * @return True if the message is valid, false otherwise
 */
bool Message::isValid() const {
    return !_command.empty();
}

/**
 * Checks if the message has a prefix
 * 
 * @return True if the message has a prefix, false otherwise
 */
bool Message::hasPrefix() const {
    return !_prefix.empty();
}

/**
 * Checks if the message has a trailing parameter
 * 
 * @return True if the message has a trailing parameter, false otherwise
 */
bool Message::hasTrailing() const {
    if (_params.empty()) {
        return false;
    }
    
    // In our implementation, a trailing parameter is one that was preceded by a colon
    // Since we've already parsed the message, we can't tell directly if a parameter
    // was a trailing parameter. Instead, we'll check if the last parameter contains spaces.
    return _params.back().find(' ') != std::string::npos;
}

/**
 * Gets the number of parameters
 * 
 * @return Number of parameters
 */
size_t Message::paramsCount() const {
    return _params.size();
}

/**
 * Converts the message to a string
 * 
 * @return String representation of the message
 */
std::string Message::toString() const {
    std::string result;
    
    // Add prefix if present
    if (hasPrefix()) {
        result += ":" + _prefix + " ";
    }
    
    // Add command
    result += _command;
    
    // Add middle parameters
    for (size_t i = 0; i < _params.size() - (hasTrailing() ? 1 : 0); i++) {
        result += " " + _params[i];
    }
    
    // Add trailing parameter if present
    if (hasTrailing()) {
        result += " :" + _params.back();
    }
    
    return result;
} 