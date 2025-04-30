#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ircserv.hpp"

// User modes struct
struct UserModes {
    bool away;       // a - away
    bool invisible;  // i - invisible
    bool wallops;    // w - receive wallops
    bool restricted; // r - restricted user connection
    bool operator_;  // o - operator flag
    bool localOp;    // O - local operator flag
    bool notices;    // s - receive server notices
};

class Client {
private:
    int _fd;                           // Client socket file descriptor
    int _status;                       // Registration status
    std::string _nickname;             // Client nickname
    std::string _username;             // Client username
    std::string _realname;             // Client real name
    std::string _hostname;             // Client hostname or IP
    std::string _buffer;               // Message buffer for outgoing messages
    std::string _inBuffer;             // Message buffer for incoming messages
    struct sockaddr_in _addr;          // Client address info
    std::map<std::string, Channel*> _channels; // Channels the client is in
    UserModes _modes;                  // User modes
    bool _authenticated;               // Password authenticated
    bool _registered;                  // Fully registered client
    bool _isOper;                      // Server operator status
    time_t _lastActivity;              // Time of last activity
    bool _commandProcessed;            // Whether the current command has been processed

public:
    // Constructors & Destructor
    Client();
    Client(int fd, const std::string& hostname);
    Client(const Client& other);
    ~Client();
    Client& operator=(const Client& other);

    // Getters
    int getFd() const;
    int getStatus() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getHostname() const;
    std::string getPrefix() const;
    std::string getBuffer() const;
    std::string getInBuffer() const;
    std::map<std::string, Channel*> getChannels() const;
    struct sockaddr_in getAddr() const;
    bool isAuthenticated() const;
    bool isRegistered() const;
    bool isOperator() const;
    bool hasMode(char mode) const;
    time_t getLastActivity() const;
    bool isCommandProcessed() const;

    // Setters
    void setFd(int fd);
    void setStatus(int status);
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setHostname(const std::string& hostname);
    void setAddr(struct sockaddr_in addr);
    void setAuthenticated(bool authenticated);
    void setRegistered(bool registered);
    void setOperator(bool oper);
    void setMode(char mode, bool value);
    void setLastActivity(time_t time);
    void setCommandProcessed(bool processed);

    // Buffer operations
    void addToBuffer(const std::string& message);
    void addToInBuffer(const std::string& message);
    void clearBuffer();
    void clearInBuffer();

    // Channel operations
    void joinChannel(Channel* channel);
    void leaveChannel(const std::string& channelName);
    void leaveAllChannels();
    bool isInChannel(const std::string& channelName) const;
    
    // Message sending
    int sendMessage(const std::string& message);
    int receiveMessage();
    
    // Registration
    void incrementStatus();
    bool checkRegistration();
};

#endif // CLIENT_HPP 