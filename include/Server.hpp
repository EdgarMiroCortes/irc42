#ifndef SERVER_HPP
#define SERVER_HPP

#include "ircserv.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"

// Operator structure for server operators
struct ServerOperator {
    std::string username;
    std::string hostname;
    std::string password;
};

// Command handler function pointer type
typedef void (Server::*CommandHandler)(Message& message);

class Server {
private:
    // Server configuration
    std::string _name;                      // Server name
    std::string _version;                   // Server version
    std::string _host;                      // Server hostname
    int _port;                              // Server port
    std::string _password;                  // Server password
    std::string _creationTime;              // Server creation time
    std::string _motd;                      // Message of the day
    
    // Server socket
    int _serverSocket;                      // Server socket descriptor
    struct sockaddr_in _serverAddr;         // Server address
    
    // Connected clients and channels
    ClientMap _clients;                     // Connected clients (fd -> client)
    ChannelMap _channels;                   // Active channels (name -> channel)
    std::vector<struct pollfd> _pollfds;    // Poll file descriptors
    std::vector<ServerOperator> _operators; // Server operators
    
    // Command handlers
    std::map<std::string, CommandHandler> _commandHandlers;
    
    // Statistics
    unsigned int _totalConnections;         // Total number of connections
    time_t _startTime;                      // Server start time
    
    // Private methods
    void _initializeCommandHandlers();      // Initialize command handlers
    void _setupServerSocket();              // Setup server socket
    void _acceptNewConnection();            // Accept new client connection
    void _handleClientInput(int fd);        // Handle client input
    void _handleClientOutput(int fd);       // Handle client output
    void _processMessage(Client* client, const std::string& message); // Process client message
    void _registerClient(Client* client);   // Register client when all info provided
    void _removeClient(int fd);             // Remove client from server
    void _cleanup();                        // Clean up resources
    
    // Authentication
    bool _authenticateClient(Client* client, const std::string& password);
    
    // Client lookup
    Client* _getClientByNickname(const std::string& nickname);
    
    // Channel operations
    Channel* _getChannel(const std::string& name);
    Channel* _createChannel(const std::string& name, Client* creator, const std::string& password = "");
    void _removeChannel(const std::string& name);
    
    // Message building
    std::string _buildReply(const std::string& code, Client* client, const std::string& message);
    std::string _buildMessage(const std::string& command, const std::string& target, const std::string& message);
    
    // Signal handling
    static bool _running;
    static void _signalHandler(int signal);
    
    // Command handlers
    void _handlePass(Message& message);
    void _handleNick(Message& message);
    void _handleUser(Message& message);
    void _handleOper(Message& message);
    void _handleQuit(Message& message);
    void _handleJoin(Message& message);
    void _handlePart(Message& message);
    void _handleMode(Message& message);
    void _handleTopic(Message& message);
    void _handleNames(Message& message);
    void _handleList(Message& message);
    void _handleInvite(Message& message);
    void _handleKick(Message& message);
    void _handlePrivmsg(Message& message);
    void _handleNotice(Message& message);
    void _handlePing(Message& message);
    void _handlePong(Message& message);
    void _handleWho(Message& message);
    void _handleWhois(Message& message);
    void _handleMotd(Message& message);
    void _handleTime(Message& message);
    void _handleUnknown(Message& message);
    
    // Mode handling helpers
    void _handleChannelMode(Message& message);
    void _handleUserMode(Message& message);
    
public:
    // Constructors & Destructor
    Server();
    Server(int port, const std::string& password);
    Server(const Server& other);
    ~Server();
    Server& operator=(const Server& other);
    
    // Server operations
    bool initialize();
    void run();
    void shutdown();
    
    // Getters
    std::string getName() const;
    std::string getVersion() const;
    std::string getHost() const;
    int getPort() const;
    std::string getPassword() const;
    std::string getCreationTime() const;
    std::string getMotd() const;
    ClientMap getClients() const;
    ChannelMap getChannels() const;
    
    // Setters
    void setName(const std::string& name);
    void setVersion(const std::string& version);
    void setHost(const std::string& host);
    void setPort(int port);
    void setPassword(const std::string& password);
    void setMotd(const std::string& motd);
    
    // Client operations
    void broadcastToAll(const std::string& message);
    void sendToClient(Client* client, const std::string& message);
    
    // Utility methods
    std::string getLocalTime() const;
    std::string getUptime() const;
};

#endif // SERVER_HPP 