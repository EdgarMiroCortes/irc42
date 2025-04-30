#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Message.hpp"

/**
 * Handle PASS command
 * 
 * @param message Message to handle
 */
void Server::_handlePass(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if client is already registered
    if (client->isRegistered()) {
        std::string errorMsg = _buildReply(ERR_ALREADYREGISTERED, client, "You may not reregister");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Check if the message has a password parameter
    if (message.paramsCount() < 1) {
        std::string errorMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "PASS :Not enough parameters");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Get the password
    std::string password = message.getParam(0);
    
    // Authenticate the client
    if (_authenticateClient(client, password)) {
        // Check if the client can be registered
        _registerClient(client);
    } else {
        std::string errorMsg = _buildReply(ERR_PASSWDMISMATCH, client, "Password incorrect");
        client->addToBuffer(errorMsg);
    }
}

/**
 * Handle NICK command
 * 
 * @param message Message to handle
 */
void Server::_handleNick(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if the message has a nickname parameter
    if (message.paramsCount() < 1) {
        std::string errorMsg = _buildReply(ERR_NONICKNAMEGIVEN, client, "No nickname given");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Get the nickname
    std::string nickname = message.getParam(0);
    
    // Check if the nickname is valid
    if (!isValidNickname(nickname)) {
        std::string errorMsg = _buildReply(ERR_ERRONEUSNICKNAME, client, nickname + " :Erroneous nickname");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Check if the nickname is already in use
    Client* existingClient = _getClientByNickname(nickname);
    if (existingClient && existingClient != client) {
        std::string errorMsg = _buildReply(ERR_NICKNAMEINUSE, client, nickname + " :Nickname is already in use");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Set the nickname
    client->setNickname(nickname);
    
    // Check if the client can be registered
    _registerClient(client);
}

/**
 * Handle USER command
 * 
 * @param message Message to handle
 */
void Server::_handleUser(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if client is already registered
    if (client->isRegistered()) {
        std::string errorMsg = _buildReply(ERR_ALREADYREGISTERED, client, "You may not reregister");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Check if the message has enough parameters
    if (message.paramsCount() < 4) {
        std::string errorMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "USER :Not enough parameters");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Get the username and realname
    std::string username = message.getParam(0);
    std::string realname = message.getParam(3);
    
    // Set the username and realname
    client->setUsername(username);
    client->setRealname(realname);
    
    // Check if the client can be registered
    _registerClient(client);
}

/**
 * Handle OPER command
 * 
 * @param message Message to handle
 */
void Server::_handleOper(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle QUIT command
 * 
 * @param message Message to handle
 */
void Server::_handleQuit(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Get the quit message
    std::string quitMsg = "Quit";
    if (message.paramsCount() > 0) {
        quitMsg = message.getParam(0);
    }
    
    // Notify all channels the client is in
    std::map<std::string, Channel*> channels = client->getChannels();
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->second) {
            std::string channelMsg = ":" + client->getPrefix() + " QUIT :" + quitMsg;
            it->second->broadcastMessage(channelMsg, client);
        }
    }
    
    // Remove the client
    _removeClient(client->getFd());
}

/**
 * Handle JOIN command
 * 
 * @param message Message to handle
 */
void Server::_handleJoin(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle PART command
 * 
 * @param message Message to handle
 */
void Server::_handlePart(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleMode(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle channel MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleChannelMode(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle user MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleUserMode(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle TOPIC command
 * 
 * @param message Message to handle
 */
void Server::_handleTopic(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle NAMES command
 * 
 * @param message Message to handle
 */
void Server::_handleNames(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle LIST command
 * 
 * @param message Message to handle
 */
void Server::_handleList(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle INVITE command
 * 
 * @param message Message to handle
 */
void Server::_handleInvite(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle KICK command
 * 
 * @param message Message to handle
 */
void Server::_handleKick(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle PRIVMSG command
 * 
 * @param message Message to handle
 */
void Server::_handlePrivmsg(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle NOTICE command
 * 
 * @param message Message to handle
 */
void Server::_handleNotice(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle PING command
 * 
 * @param message Message to handle
 */
void Server::_handlePing(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if the message has a parameter
    if (message.paramsCount() < 1) {
        std::string errorMsg = _buildReply(ERR_NOORIGIN, client, "No origin specified");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Get the ping parameter
    std::string param = message.getParam(0);
    
    // Send a PONG reply
    std::string pongMsg = ":" + _name + " PONG " + _name + " :" + param;
    client->addToBuffer(pongMsg);
}

/**
 * Handle PONG command
 * 
 * @param message Message to handle
 */
void Server::_handlePong(Message& message) {
    (void)message; // Unused parameter
    // Just ignore PONG messages for now
}

/**
 * Handle WHO command
 * 
 * @param message Message to handle
 */
void Server::_handleWho(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle WHOIS command
 * 
 * @param message Message to handle
 */
void Server::_handleWhois(Message& message) {
    (void)message; // Unused parameter
    // Stub implementation
}

/**
 * Handle MOTD command
 * 
 * @param message Message to handle
 */
void Server::_handleMotd(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if client is registered
    if (!client->isRegistered()) {
        std::string errorMsg = _buildReply(ERR_NOTREGISTERED, client, "You have not registered");
        client->addToBuffer(errorMsg);
        return;
    }
    
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
}

/**
 * Handle TIME command
 * 
 * @param message Message to handle
 */
void Server::_handleTime(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Check if client is registered
    if (!client->isRegistered()) {
        std::string errorMsg = _buildReply(ERR_NOTREGISTERED, client, "You have not registered");
        client->addToBuffer(errorMsg);
        return;
    }
    
    // Send the server time
    std::string timeMsg = _buildReply(RPL_TIME, client, _name + " :" + getLocalTime());
    client->addToBuffer(timeMsg);
}

/**
 * Handle unknown command
 * 
 * @param message Message to handle
 */
void Server::_handleUnknown(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }
    
    // Get the command
    std::string command = message.getCommand();
    
    // Send an error message
    std::string errorMsg = _buildReply(ERR_UNKNOWNCOMMAND, client, command + " :Unknown command");
    client->addToBuffer(errorMsg);
} 