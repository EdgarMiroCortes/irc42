#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "ircserv.hpp"

class Channel {
private:
    std::string _name;                       // Channel name
    std::string _topic;                      // Channel topic
    std::string _password;                   // Channel key/password
    std::map<std::string, Client*> _clients; // Users in channel (nick -> client)
    std::set<std::string> _operators;        // Channel operators
    std::set<std::string> _voiced;           // Users with voice privileges
    std::set<std::string> _banned;           // Banned users/masks
    std::set<std::string> _invited;          // Invited users for invite-only channels
    
    // Channel modes
    bool _inviteOnly;                        // Mode +i - invite only
    bool _topicRestricted;                   // Mode +t - only ops can change topic
    bool _noExternalMessages;                // Mode +n - no external messages
    bool _moderated;                         // Mode +m - moderated channel
    unsigned int _userLimit;                 // Mode +l - user limit
    time_t _creationTime;                    // Channel creation time
    
public:
    // Constructors & Destructor
    Channel();
    Channel(const std::string& name, Client* creator, const std::string& password = "");
    Channel(const Channel& other);
    ~Channel();
    Channel& operator=(const Channel& other);
    
    // Getters
    std::string getName() const;
    std::string getTopic() const;
    std::string getPassword() const;
    std::map<std::string, Client*> getClients() const;
    std::set<std::string> getOperators() const;
    std::set<std::string> getVoiced() const;
    std::set<std::string> getBanned() const;
    std::set<std::string> getInvited() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    bool hasNoExternalMessages() const;
    bool isModerated() const;
    unsigned int getUserLimit() const;
    time_t getCreationTime() const;
    std::string getModes() const;
    std::string getModeString() const;
    
    // Setters
    void setTopic(const std::string& topic);
    void setPassword(const std::string& password);
    void setInviteOnly(bool inviteOnly);
    void setTopicRestricted(bool topicRestricted);
    void setNoExternalMessages(bool noExternalMessages);
    void setModerated(bool moderated);
    void setUserLimit(unsigned int userLimit);
    
    // Client operations
    bool addClient(Client* client);
    bool removeClient(Client* client);
    bool isClient(const std::string& nickname) const;
    bool isClient(Client* client) const;
    
    // Operator operations
    bool addOperator(const std::string& nickname);
    bool removeOperator(const std::string& nickname);
    bool isOperator(const std::string& nickname) const;
    bool isOperator(Client* client) const;
    
    // Voice operations
    bool addVoice(const std::string& nickname);
    bool removeVoice(const std::string& nickname);
    bool hasVoice(const std::string& nickname) const;
    bool hasVoice(Client* client) const;
    
    // Ban operations
    bool addBan(const std::string& mask);
    bool removeBan(const std::string& mask);
    bool isBanned(const std::string& nickname) const;
    bool isBanned(Client* client) const;
    
    // Invite operations
    bool addInvite(const std::string& nickname);
    bool removeInvite(const std::string& nickname);
    bool isInvited(const std::string& nickname) const;
    bool isInvited(Client* client) const;
    
    // Mode operations
    bool setMode(char mode, bool add, const std::string& param = "");
    
    // Message sending
    void broadcastMessage(const std::string& message, Client* exclude = NULL);
    
    // Channel info
    std::string getClientsList() const;
    size_t getClientCount() const;
    
    // Mode parameters
    std::string getOldModeParams(char mode) const;
};

#endif // CHANNEL_HPP 