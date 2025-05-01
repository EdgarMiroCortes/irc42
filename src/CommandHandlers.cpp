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
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 2) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "OPER :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string name = message.getParam(0);
    std::string password = message.getParam(1);
    
    // In a real server, this would check against a configuration file of operators
    // For now, we'll just hardcode a single operator name and password
    if (name == "admin" && password == "adminpass") {
        // Set the client as an operator
        client->setOperator(true);
        
        // Send RPL_YOUREOPER
        std::string operMsg = _buildReply(RPL_YOUREOPER, client, ":You are now an IRC operator");
        client->addToBuffer(operMsg);
        
        // Send the operator MODE message
        std::string modeMsg = ":" + client->getPrefix() + " MODE " + client->getNickname() + " :+o";
        client->addToBuffer(modeMsg);
    } else {
        // Send ERR_PASSWDMISMATCH
        std::string errMsg = _buildReply(ERR_PASSWDMISMATCH, client, ":Password incorrect");
        client->addToBuffer(errMsg);
    }
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
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "JOIN :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string channelName = message.getParam(0);
    std::string password = (message.paramsCount() > 1) ? message.getParam(1) : "";

    // Validar nombre del canal
    if (!isValidChannelName(channelName)) {
        std::string errMsg = _buildReply(ERR_BADCHANMASK, client, channelName + " :Invalid channel name");
        client->addToBuffer(errMsg);
        return;
    }

    // Buscar o crear el canal
    Channel* channel = _getChannel(channelName);
    if (!channel) {
        channel = _createChannel(channelName, client, password);
        
        // Notificar JOIN al cliente
        std::string joinMsg = ":" + client->getPrefix() + " JOIN " + channelName;
        client->addToBuffer(joinMsg);
        
        // Enviar lista de miembros (RPL_NAMREPLY)
        std::string namesMsg = _buildReply(RPL_NAMREPLY, client, "= " + channelName + " :" + channel->getClientsList());
        client->addToBuffer(namesMsg);
        std::string endNamesMsg = _buildReply(RPL_ENDOFNAMES, client, channelName + " :End of NAMES list");
        client->addToBuffer(endNamesMsg);
        
        return;
    }
    
    // Check if channel has a password
    if (!channel->getPassword().empty() && channel->getPassword() != password) {
        std::string errMsg = _buildReply(ERR_BADCHANNELKEY, client, channelName + " :Cannot join channel (+k)");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if channel is invite-only
    if (channel->isInviteOnly() && !channel->isInvited(client)) {
        std::string errMsg = _buildReply(ERR_INVITEONLYCHAN, client, channelName + " :Cannot join channel (+i)");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if channel is full
    if (channel->getUserLimit() > 0 && channel->getClientCount() >= channel->getUserLimit()) {
        std::string errMsg = _buildReply(ERR_CHANNELISFULL, client, channelName + " :Cannot join channel (+l)");
        client->addToBuffer(errMsg);
        return;
    }

    // Unir al cliente al canal (don't check for user limit here, we already checked)
    if (!channel->addClient(client)) {
        // Client might be banned
        if (channel->isBanned(client)) {
            std::string errMsg = _buildReply(ERR_BANNEDFROMCHAN, client, channelName + " :Cannot join channel (+b)");
            client->addToBuffer(errMsg);
        }
        return;
    }

    // Notificar JOIN al cliente
    std::string joinMsg = ":" + client->getPrefix() + " JOIN " + channelName;
    client->addToBuffer(joinMsg);

    // Notificar a otros miembros del canal
    channel->broadcastMessage(joinMsg, client);

    // Enviar lista de miembros (RPL_NAMREPLY)
    std::string namesMsg = _buildReply(RPL_NAMREPLY, client, "= " + channelName + " :" + channel->getClientsList());
    client->addToBuffer(namesMsg);
    std::string endNamesMsg = _buildReply(RPL_ENDOFNAMES, client, channelName + " :End of NAMES list");
    client->addToBuffer(endNamesMsg);
}

/**
 * Handle PART command
 * 
 * @param message Message to handle
 */
void Server::_handlePart(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "PART :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string channelsParam = message.getParam(0);
    std::string partMessage = (message.paramsCount() > 1) ? message.getParam(1) : client->getNickname();
    
    // Split the channel list by comma
    std::vector<std::string> channelNames = split(channelsParam, ',');
    
    for (size_t i = 0; i < channelNames.size(); i++) {
        std::string channelName = channelNames[i];
        
        // Check if the channel exists
        Channel* channel = _getChannel(channelName);
        if (!channel) {
            std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, channelName + " :No such channel");
            client->addToBuffer(errMsg);
            continue;
        }
        
        // Check if the client is in the channel
        if (!channel->isClient(client)) {
            std::string errMsg = _buildReply(ERR_NOTONCHANNEL, client, channelName + " :You're not on that channel");
            client->addToBuffer(errMsg);
            continue;
        }
        
        // Build the PART message
        std::string partMsg = ":" + client->getPrefix() + " PART " + channelName;
        if (message.paramsCount() > 1) {
            partMsg += " :" + partMessage;
        }
        
        // Send the PART message to the channel
        channel->broadcastMessage(partMsg);
        
        // Remove the client from the channel
        channel->removeClient(client);
        
        // If the channel is empty, remove it
        if (channel->getClientCount() == 0) {
            _removeChannel(channelName);
        }
    }
}

/**
 * Handle MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleMode(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "MODE :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string target = message.getParam(0);
    
    // Check if the target is a channel or a user
    if (target[0] == '#' || target[0] == '&') {
        // Handle channel mode
        _handleChannelMode(message);
    } else {
        // Handle user mode
        _handleUserMode(message);
    }
}

/**
 * Handle channel MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleChannelMode(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }

    std::string channelName = message.getParam(0);
    Channel* channel = _getChannel(channelName);
    
    if (!channel) {
        std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, channelName + " :No such channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // If no modes are provided, just return the current modes
    if (message.paramsCount() == 1) {
        std::string currentModes = channel->getModes();
        std::string modeReply = _buildReply(RPL_CHANNELMODEIS, client, channelName + " " + currentModes);
        client->addToBuffer(modeReply);
        return;
    }
    
    // Check if the client has permission to change modes
    if (!channel->isOperator(client->getNickname()) && !client->isOperator()) {
        std::string errMsg = _buildReply(ERR_CHANOPRIVSNEEDED, client, channelName + " :You're not channel operator");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Parse the mode string
    std::string modeString = message.getParam(1);
    bool adding = true;
    size_t currentParam = 2;
    std::string modeChanges = "";
    std::string paramChanges = "";
    bool topicModeChanged = false;
    
    for (size_t i = 0; i < modeString.length(); i++) {
        char c = modeString[i];
        
        if (c == '+') {
            adding = true;
            continue;
        } else if (c == '-') {
            adding = false;
            continue;
        }
        
        // Get the parameter if needed
        std::string param = "";
        if (c == 'k' || c == 'l' || c == 'o' || c == 'v' || c == 'b') {
            if (currentParam < message.paramsCount()) {
                param = message.getParam(currentParam);
                currentParam++;
            } else if (adding) {
                // If we're adding a mode that requires a parameter and none is provided, skip it
                continue;
            }
        }
        
        // Apply the mode change
        if (channel->setMode(c, adding, param)) {
            // Mode change successful
            modeChanges += (adding ? "+" : "-");
            modeChanges += c;
            
            if (!param.empty()) {
                if (!paramChanges.empty()) {
                    paramChanges += " ";
                }
                paramChanges += param;
            }
            
            // Check if topic mode was changed
            if (c == 't') {
                topicModeChanged = true;
            }
        }
    }
    
    // If there were mode changes, notify all clients in the channel
    if (!modeChanges.empty()) {
        std::string modeMsg = ":" + client->getPrefix() + " MODE " + channelName + " " + modeChanges;
        if (!paramChanges.empty()) {
            modeMsg += " " + paramChanges;
        }
        channel->broadcastMessage(modeMsg);
        
        // If topic mode was changed, send additional info about current state
        if (topicModeChanged) {
            std::string topicMsg;
            if (channel->isTopicRestricted()) {
                topicMsg = _buildReply("NOTICE", client, channelName + " :Topic is now restricted to channel operators");
            } else {
                topicMsg = _buildReply("NOTICE", client, channelName + " :Topic can now be changed by all channel members");
            }
            channel->broadcastMessage(topicMsg);
        }
    }
}

/**
 * Handle user MODE command
 * 
 * @param message Message to handle
 */
void Server::_handleUserMode(Message& message) {
    Client* client = message.getSender();
    if (!client) {
        return;
    }

    std::string nickname = message.getParam(0);
    
    // Check if the target user exists
    Client* targetClient = _getClientByNickname(nickname);
    if (!targetClient) {
        std::string errMsg = _buildReply(ERR_NOSUCHNICK, client, nickname + " :No such nickname");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Only the user themselves can change their modes (except IRC operators)
    if (client->getNickname() != targetClient->getNickname() && !client->isOperator()) {
        std::string errMsg = _buildReply(ERR_USERSDONTMATCH, client, ":Cannot change mode for other users");
        client->addToBuffer(errMsg);
        return;
    }
    
    // If no modes are provided, just return the current modes
    if (message.paramsCount() == 1) {
        // Just show the current modes
        std::string modeReply = _buildReply(RPL_UMODEIS, client, "+");
        if (targetClient->isOperator()) {
            modeReply += "o";
        }
        // Add other modes as needed
        client->addToBuffer(modeReply);
        return;
    }
    
    // Parse the mode string
    std::string modeString = message.getParam(1);
    bool adding = true;
    std::string modeChanges = "";
    
    for (size_t i = 0; i < modeString.length(); i++) {
        char c = modeString[i];
        
        if (c == '+') {
            adding = true;
            continue;
        } else if (c == '-') {
            adding = false;
            continue;
        }
        
        bool modeChanged = false;
        
        // Handle different user modes
        switch (c) {
            case 'i': // Invisible
                // We don't have setVisible, so for now we'll ignore this
                modeChanged = true;
                break;
                
            case 'o': // Operator
                // Only allow demotion, not promotion
                if (!adding && (client->isOperator() || client == targetClient)) {
                    targetClient->setOperator(false);
                    modeChanged = true;
                } else if (adding && client->isOperator() && client != targetClient) {
                    // Only IRC operators can promote others to operators
                    targetClient->setOperator(true);
                    modeChanged = true;
                }
                break;
                
            case 'w': // Receive wallops
                // We don't have setReceiveWallops, so for now we'll ignore this
                modeChanged = true;
                break;
                
            case 'a': // Away
                // This should normally be set by the AWAY command, not MODE
                // Just ignore it for now
                break;
                
            default:
                // Unknown mode, ignore
                break;
        }
        
        if (modeChanged) {
            modeChanges += (adding ? "+" : "-");
            modeChanges += c;
        }
    }
    
    // If there were mode changes, notify the client
    if (!modeChanges.empty()) {
        std::string modeMsg = ":" + client->getPrefix() + " MODE " + targetClient->getNickname() + " " + modeChanges;
        targetClient->addToBuffer(modeMsg);
        
        // If the client is not the target, also notify the client
        if (client != targetClient) {
            client->addToBuffer(modeMsg);
        }
    }
}

/**
 * Handle TOPIC command
 * 
 * @param message Message to handle
 */
void Server::_handleTopic(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "TOPIC :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string channelName = message.getParam(0);
    
    // Check if the channel exists
    Channel* channel = _getChannel(channelName);
    if (!channel) {
        std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, channelName + " :No such channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the client is in the channel
    if (!channel->isClient(client)) {
        std::string errMsg = _buildReply(ERR_NOTONCHANNEL, client, channelName + " :You're not on that channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // If no second parameter, show the topic
    if (message.paramsCount() == 1) {
        std::string topic = channel->getTopic();
        if (topic.empty()) {
            std::string noTopicMsg = _buildReply(RPL_NOTOPIC, client, channelName + " :No topic is set");
            client->addToBuffer(noTopicMsg);
        } else {
            std::string topicMsg = _buildReply(RPL_TOPIC, client, channelName + " :" + topic);
            client->addToBuffer(topicMsg);
        }
        return;
    }
    
    // Try to set the topic
    std::string newTopic = message.getParam(1);
    
    // Check if the channel is +t (topic protection) and the client is not an operator
    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        std::string errMsg = _buildReply(ERR_CHANOPRIVSNEEDED, client, channelName + " :You're not channel operator");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Set the topic and broadcast the change
    channel->setTopic(newTopic);
    
    // Notify all clients in the channel
    std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic;
    channel->broadcastMessage(topicMsg);
}

/**
 * Handle NAMES command
 * 
 * @param message Message to handle
 */
void Server::_handleNames(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() == 0) {
        // List all channels and their users
        for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it) {
            Channel* channel = it->second;
            if (channel) {
                std::string namesMsg = _buildReply(RPL_NAMREPLY, client, "= " + channel->getName() + " :" + channel->getClientsList());
                client->addToBuffer(namesMsg);
                std::string endNamesMsg = _buildReply(RPL_ENDOFNAMES, client, channel->getName() + " :End of NAMES list");
                client->addToBuffer(endNamesMsg);
            }
        }
        return;
    }

    // List users for specific channels
    std::string channelsParam = message.getParam(0);
    std::vector<std::string> channelNames = split(channelsParam, ',');
    
    for (size_t i = 0; i < channelNames.size(); i++) {
        std::string channelName = channelNames[i];
        Channel* channel = _getChannel(channelName);
        
        if (!channel) {
            // Channel doesn't exist, just send an end of names message
            std::string endNamesMsg = _buildReply(RPL_ENDOFNAMES, client, channelName + " :End of NAMES list");
            client->addToBuffer(endNamesMsg);
            continue;
        }
        
        // Send names list
        std::string namesMsg = _buildReply(RPL_NAMREPLY, client, "= " + channelName + " :" + channel->getClientsList());
        client->addToBuffer(namesMsg);
        std::string endNamesMsg = _buildReply(RPL_ENDOFNAMES, client, channelName + " :End of NAMES list");
        client->addToBuffer(endNamesMsg);
    }
}

/**
 * Handle LIST command
 * 
 * @param message Message to handle
 */
void Server::_handleList(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    // Start the list response
    std::string listStartMsg = _buildReply(RPL_LISTSTART, client, "Channel :Users  Name");
    client->addToBuffer(listStartMsg);

    if (message.paramsCount() == 0) {
        // List all channels
        for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it) {
            Channel* channel = it->second;
            if (channel) {
                std::stringstream userCount;
                userCount << channel->getClientCount();
                std::string topic = channel->getTopic();
                if (topic.empty()) {
                    topic = "No topic set";
                }
                
                std::string listMsg = _buildReply(RPL_LIST, client, 
                                                 channel->getName() + " " + 
                                                 userCount.str() + " :" + topic);
                client->addToBuffer(listMsg);
            }
        }
    } else {
        // List specific channels
        std::string channelsParam = message.getParam(0);
        std::vector<std::string> channelNames = split(channelsParam, ',');
        
        for (size_t i = 0; i < channelNames.size(); i++) {
            std::string channelName = channelNames[i];
            Channel* channel = _getChannel(channelName);
            
            if (channel) {
                std::stringstream userCount;
                userCount << channel->getClientCount();
                std::string topic = channel->getTopic();
                if (topic.empty()) {
                    topic = "No topic set";
                }
                
                std::string listMsg = _buildReply(RPL_LIST, client, 
                                                 channel->getName() + " " + 
                                                 userCount.str() + " :" + topic);
                client->addToBuffer(listMsg);
            }
        }
    }
    
    // End the list response
    std::string listEndMsg = _buildReply(RPL_LISTEND, client, ":End of LIST");
    client->addToBuffer(listEndMsg);
}

/**
 * Handle INVITE command
 * 
 * @param message Message to handle
 */
void Server::_handleInvite(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 2) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "INVITE :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string nickname = message.getParam(0);
    std::string channelName = message.getParam(1);
    
    // Check if the target user exists
    Client* targetClient = _getClientByNickname(nickname);
    if (!targetClient) {
        std::string errMsg = _buildReply(ERR_NOSUCHNICK, client, nickname + " :No such nickname");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the channel exists
    Channel* channel = _getChannel(channelName);
    if (!channel) {
        std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, channelName + " :No such channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the client is in the channel
    if (!channel->isClient(client)) {
        std::string errMsg = _buildReply(ERR_NOTONCHANNEL, client, channelName + " :You're not on that channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the target is already in the channel
    if (channel->isClient(nickname)) {
        std::string errMsg = _buildReply(ERR_USERONCHANNEL, client, nickname + " " + channelName + " :is already on channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // If the channel is invite-only, check if the client has permission to invite
    if (channel->isInviteOnly() && !channel->isOperator(client->getNickname()) && !client->isOperator()) {
        std::string errMsg = _buildReply(ERR_CHANOPRIVSNEEDED, client, channelName + " :You're not channel operator");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Add the target client to the channel's invite list
    channel->addInvite(nickname);
    
    // Send RPL_INVITING to the inviter
    std::string invitingMsg = _buildReply(RPL_INVITING, client, nickname + " " + channelName);
    client->addToBuffer(invitingMsg);
    
    // Send the INVITE message to the invitee
    std::string inviteMsg = ":" + client->getPrefix() + " INVITE " + nickname + " " + channelName;
    targetClient->addToBuffer(inviteMsg);
    
    // If the target client is away, also notify the inviter
    /* We don't have isAway method, so we'll skip this for now
    if (targetClient->isAway()) {
        std::string awayMsg = _buildReply(RPL_AWAY, client, nickname + " :" + targetClient->getAwayMessage());
        client->addToBuffer(awayMsg);
    }
    */
}

/**
 * Handle KICK command
 * 
 * @param message Message to handle
 */
void Server::_handleKick(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 2) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "KICK :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string channelName = message.getParam(0);
    std::string targetNick = message.getParam(1);
    std::string kickReason = (message.paramsCount() > 2) ? message.getParam(2) : client->getNickname();
    
    // Check if the channel exists
    Channel* channel = _getChannel(channelName);
    if (!channel) {
        std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, channelName + " :No such channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the client is in the channel
    if (!channel->isClient(client)) {
        std::string errMsg = _buildReply(ERR_NOTONCHANNEL, client, channelName + " :You're not on that channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the client is an operator in the channel
    if (!channel->isOperator(client)) {
        std::string errMsg = _buildReply(ERR_CHANOPRIVSNEEDED, client, channelName + " :You're not channel operator");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Check if the target is in the channel
    if (!channel->isClient(targetNick)) {
        std::string errMsg = _buildReply(ERR_USERNOTINCHANNEL, client, targetNick + " " + channelName + " :They aren't on that channel");
        client->addToBuffer(errMsg);
        return;
    }
    
    // Get the target client
    Client* targetClient = _getClientByNickname(targetNick);
    if (!targetClient) {
        return; // Should not happen if the client is in the channel
    }
    
    // Build the KICK message
    std::string kickMsg = ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + kickReason;
    
    // Send the KICK message to all clients in the channel
    channel->broadcastMessage(kickMsg);
    
    // Remove the target client from the channel
    channel->removeClient(targetClient);
    
    // If the channel is empty, remove it
    if (channel->getClientCount() == 0) {
        _removeChannel(channelName);
    }
}

/**
 * Handle PRIVMSG command
 * 
 * @param message Message to handle
 */
// void Server::_handlePrivmsg(Message& message) {
//     (void)message; // Unused parameter
//     // Stub implementation
// }
void Server::_handlePrivmsg(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    // Verificar parámetros mínimos: PRIVMSG <target> <message>
    if (message.paramsCount() < 2) {
        std::string errMsg = _buildReply(ERR_NEEDMOREPARAMS, client, "PRIVMSG :Not enough parameters");
        client->addToBuffer(errMsg);
        return;
    }

    std::string target = message.getParam(0);
    std::string text = message.getParam(1); // El mensaje (trailing)

    // Enviar a un canal
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = _getChannel(target);
        if (!channel) {
            std::string errMsg = _buildReply(ERR_NOSUCHCHANNEL, client, target + " :No such channel");
            client->addToBuffer(errMsg);
            return;
        }

        // Verificar si el cliente está en el canal
        if (!channel->isClient(client)) {
            std::string errMsg = _buildReply(ERR_NOTONCHANNEL, client, target + " :You're not on that channel");
            client->addToBuffer(errMsg);
            return;
        }

        // Verificar modos del canal (+n, +m, bans)
        if (channel->hasNoExternalMessages() && !channel->isClient(client)) {
            std::string errMsg = _buildReply(ERR_CANNOTSENDTOCHAN, client, target + " :Cannot send to channel");
            client->addToBuffer(errMsg);
            return;
        }

        // Broadcast del mensaje
        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        channel->broadcastMessage(msg, client);
    }
    // Enviar a un usuario
    else {
        Client* targetClient = _getClientByNickname(target);
        if (!targetClient) {
            std::string errMsg = _buildReply(ERR_NOSUCHNICK, client, target + " :No such nick");
            client->addToBuffer(errMsg);
            return;
        }

        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        targetClient->addToBuffer(msg);
    }
}

/**
 * Handle NOTICE command
 * 
 * @param message Message to handle
 */
void Server::_handleNotice(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered() || message.paramsCount() < 2) {
        return; // NOTICE doesn't send error replies
    }
    
    std::string target = message.getParam(0);
    std::string msgText = message.getParam(1);
    
    if (target.empty() || msgText.empty()) {
        return; // NOTICE doesn't send error replies
    }
    
    // Create the message string
    std::string msg = ":" + client->getPrefix() + " NOTICE " + target + " :" + msgText;
    
    // Check if the target is a channel
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = _getChannel(target);
        if (!channel) {
            return; // NOTICE doesn't send error replies
        }
        
        // Check if the client is in the channel
        if (!channel->isClient(client->getNickname())) {
            return; // NOTICE doesn't send error replies
        }
        
        // Check if the channel is moderated and the client doesn't have voice
        if (channel->isModerated() && !channel->hasVoice(client->getNickname()) && !channel->isOperator(client->getNickname())) {
            return; // NOTICE doesn't send error replies
        }
        
        // Broadcast the message to all clients in the channel except the sender
        channel->broadcastMessage(msg, client);
    } else {
        // The target is a client
        Client* targetClient = _getClientByNickname(target);
        if (!targetClient) {
            return; // NOTICE doesn't send error replies
        }
        
        // Send the message to the client
        targetClient->addToBuffer(msg);
    }
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
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        // WHO with no parameters should list all visible users
        // For simplicity, we'll just return an end of WHO list
        std::string endWhoMsg = _buildReply(RPL_ENDOFWHO, client, "* :End of WHO list");
        client->addToBuffer(endWhoMsg);
        return;
    }

    std::string mask = message.getParam(0);
    bool operOnly = (message.paramsCount() > 1 && message.getParam(1) == "o");
    
    // Check if the mask is a channel name
    if (mask[0] == '#' || mask[0] == '&') {
        Channel* channel = _getChannel(mask);
        if (channel) {
            // Get all clients in the channel
            std::map<std::string, Client*> clients = channel->getClients();
            
            for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
                Client* targetClient = it->second;
                
                if (operOnly && !targetClient->isOperator()) {
                    continue;
                }
                
                // Build the WHO reply
                std::string whoReply = _buildReply(RPL_WHOREPLY, client,
                    channel->getName() + " " +
                    targetClient->getUsername() + " " +
                    targetClient->getHostname() + " " +
                    _name + " " +
                    targetClient->getNickname() + " " +
                    (false ? "G" : "H") +
                    (targetClient->isOperator() ? "*" : "") +
                    (channel->isOperator(targetClient->getNickname()) ? "@" : 
                     (channel->hasVoice(targetClient->getNickname()) ? "+" : "")) +
                    " :0 " + targetClient->getRealname());
                
                client->addToBuffer(whoReply);
            }
        }
        
        // End of WHO list
        std::string endWhoMsg = _buildReply(RPL_ENDOFWHO, client, mask + " :End of WHO list");
        client->addToBuffer(endWhoMsg);
        return;
    }
    
    // The mask is a user mask
    for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        Client* targetClient = it->second;
        
        if (!targetClient || !targetClient->isRegistered()) {
            continue;
        }
        
        if (operOnly && !targetClient->isOperator()) {
            continue;
        }
        
        // Check if the client matches the mask
        // For simplicity, we'll just check if the nickname contains the mask
        if (targetClient->getNickname().find(mask) != std::string::npos) {
            // Get a common channel with the target client, if any
            std::string commonChannel = "";
            std::map<std::string, Channel*> clientChannels = client->getChannels();
            std::map<std::string, Channel*> targetChannels = targetClient->getChannels();
            
            for (std::map<std::string, Channel*>::iterator chanIt = clientChannels.begin(); chanIt != clientChannels.end(); ++chanIt) {
                if (targetChannels.find(chanIt->first) != targetChannels.end()) {
                    commonChannel = chanIt->first;
                    break;
                }
            }
            
            // If no common channel, skip the client if they're not visible
            // We don't have isVisible method, so we'll assume all clients are visible for now
            /*
            if (commonChannel.empty() && !targetClient->isVisible()) {
                continue;
            }
            */
            
            // Build the WHO reply
            std::string whoReply = _buildReply(RPL_WHOREPLY, client,
                commonChannel + " " +
                targetClient->getUsername() + " " +
                targetClient->getHostname() + " " +
                _name + " " +
                targetClient->getNickname() + " " +
                (false ? "G" : "H") +
                (targetClient->isOperator() ? "*" : "") +
                (commonChannel.empty() ? "" : 
                 (targetChannels[commonChannel]->isOperator(targetClient->getNickname()) ? "@" : 
                  (targetChannels[commonChannel]->hasVoice(targetClient->getNickname()) ? "+" : ""))) +
                " :0 " + targetClient->getRealname());
            
            client->addToBuffer(whoReply);
        }
    }
    
    // End of WHO list
    std::string endWhoMsg = _buildReply(RPL_ENDOFWHO, client, mask + " :End of WHO list");
    client->addToBuffer(endWhoMsg);
}

/**
 * Handle WHOIS command
 * 
 * @param message Message to handle
 */
void Server::_handleWhois(Message& message) {
    Client* client = message.getSender();
    if (!client || !client->isRegistered()) {
        return;
    }

    if (message.paramsCount() < 1) {
        std::string errMsg = _buildReply(ERR_NONICKNAMEGIVEN, client, ":No nickname given");
        client->addToBuffer(errMsg);
        return;
    }

    // Get target or targets
    std::string targets = message.getParam(0);
    
    // Optional second parameter is ignored (server)
    // For simplicity, we'll just use the first parameter as a comma-separated list of nicknames
    std::vector<std::string> nicknames = split(targets, ',');
    
    for (size_t i = 0; i < nicknames.size(); i++) {
        std::string nickname = nicknames[i];
        Client* targetClient = _getClientByNickname(nickname);
        
        if (!targetClient) {
            std::string errMsg = _buildReply(ERR_NOSUCHNICK, client, nickname + " :No such nickname");
            client->addToBuffer(errMsg);
            continue;
        }
        
        // User info
        std::string userInfoMsg = _buildReply(RPL_WHOISUSER, client, 
            targetClient->getNickname() + " " + 
            targetClient->getUsername() + " " + 
            targetClient->getHostname() + " * :" + 
            targetClient->getRealname());
        client->addToBuffer(userInfoMsg);
        
        // Server info
        std::string serverInfoMsg = _buildReply(RPL_WHOISSERVER, client, 
            targetClient->getNickname() + " " + 
            _name + " :This server");
        client->addToBuffer(serverInfoMsg);
        
        // Is operator?
        if (targetClient->isOperator()) {
            std::string operMsg = _buildReply(RPL_WHOISOPERATOR, client, 
                targetClient->getNickname() + " :is an IRC operator");
            client->addToBuffer(operMsg);
        }
        
        // Idle time
        time_t now = time(NULL);
        time_t idleTime = now - targetClient->getLastActivity();
        std::stringstream ss;
        ss << idleTime;
        std::string idleMsg = _buildReply(RPL_WHOISIDLE, client, 
            targetClient->getNickname() + " " + 
            ss.str() + " :seconds idle");
        client->addToBuffer(idleMsg);
        
        // Channel list
        std::map<std::string, Channel*> channels = targetClient->getChannels();
        if (!channels.empty()) {
            std::string channelList = "";
            
            for (std::map<std::string, Channel*>::iterator chanIt = channels.begin(); chanIt != channels.end(); ++chanIt) {
                if (!channelList.empty()) {
                    channelList += " ";
                }
                
                Channel* channel = chanIt->second;
                if (channel) {
                    if (channel->isOperator(targetClient->getNickname())) {
                        channelList += "@";
                    } else if (channel->hasVoice(targetClient->getNickname())) {
                        channelList += "+";
                    }
                    channelList += channel->getName();
                }
            }
            
            std::string channelsMsg = _buildReply(RPL_WHOISCHANNELS, client, 
                targetClient->getNickname() + " :" + channelList);
            client->addToBuffer(channelsMsg);
        }
        
        // End of WHOIS
        std::string endMsg = _buildReply(RPL_ENDOFWHOIS, client, 
            targetClient->getNickname() + " :End of WHOIS list");
        client->addToBuffer(endMsg);
    }
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