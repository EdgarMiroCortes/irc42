#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "ircserv.hpp"

class Message {
private:
    std::string _prefix;           // Message prefix (optional)
    std::string _command;          // Message command
    std::vector<std::string> _params; // Message parameters
    std::string _rawMessage;       // Raw message string
    Client* _sender;               // Sender client
    
public:
    // Constructors & Destructor
    Message();
    Message(const std::string& rawMessage, Client* sender = NULL);
    Message(const Message& other);
    ~Message();
    Message& operator=(const Message& other);
    
    // Getters
    std::string getPrefix() const;
    std::string getCommand() const;
    std::vector<std::string> getParams() const;
    std::string getParam(size_t index) const;
    std::string getTrailing() const; // Get the last parameter (after :)
    std::string getRawMessage() const;
    Client* getSender() const;
    
    // Setters
    void setPrefix(const std::string& prefix);
    void setCommand(const std::string& command);
    void setParams(const std::vector<std::string>& params);
    void addParam(const std::string& param);
    void setRawMessage(const std::string& rawMessage);
    void setSender(Client* sender);
    
    // Parsing
    bool parse(const std::string& rawMessage);
    
    // Validation
    bool isValid() const;
    bool hasPrefix() const;
    bool hasTrailing() const;
    size_t paramsCount() const;
    
    // Conversion
    std::string toString() const;
};

#endif // MESSAGE_HPP 