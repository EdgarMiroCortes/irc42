#include "../include/ircserv.hpp"

/**
 * Returns the current date and time as a formatted string
 * 
 * @return Current date and time as a string
 */
std::string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

/**
 * Splits a string into a vector of strings based on a delimiter
 * 
 * @param s String to split
 * @param delimiter Character to split on
 * @return Vector of split strings
 */
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

/**
 * Converts a string to lowercase
 * 
 * @param s String to convert
 * @return Lowercase string
 */
std::string toLower(const std::string& s) {
    std::string result = s;
    
    // In C++98, we can't use lambda functions, so we use a for loop
    for (size_t i = 0; i < result.length(); i++) {
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    }
    
    return result;
}

/**
 * Checks if a nickname is valid according to IRC standards
 * 
 * @param nickname Nickname to check
 * @return True if the nickname is valid, false otherwise
 */
bool isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }
    
    // First character must be a letter or special character
    if (!isalpha(nickname[0]) && nickname[0] != '_' && nickname[0] != '[' && 
        nickname[0] != ']' && nickname[0] != '\\' && nickname[0] != '`' && 
        nickname[0] != '^' && nickname[0] != '{' && nickname[0] != '}' && 
        nickname[0] != '|') {
        return false;
    }
    
    // Other characters can be letters, digits, or special characters
    for (size_t i = 1; i < nickname.length(); i++) {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && 
            c != '\\' && c != '`' && c != '^' && c != '{' && c != '}' && 
            c != '|') {
            return false;
        }
    }
    
    return true;
}

/**
 * Checks if a channel name is valid according to IRC standards
 * 
 * @param channelName Channel name to check
 * @return True if the channel name is valid, false otherwise
 */
bool isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }
    
    // Channel name must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&') {
        return false;
    }
    
    // Channel name cannot contain spaces, commas, or non-printable characters
    for (size_t i = 1; i < channelName.length(); i++) {
        char c = channelName[i];
        if (c == ' ' || c == ',' || c == '\a' || c == '\0' || c == '\r' || 
            c == '\n' || c == 7) {
            return false;
        }
    }
    
    return true;
} 