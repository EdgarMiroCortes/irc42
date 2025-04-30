#include "../include/ircserv.hpp"
#include "../include/Server.hpp"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <port> <password>" << std::endl;
    std::cerr << "  port     - Port number on which the IRC server will listen (1024-65535)" << std::endl;
    std::cerr << "  password - Password required for clients to connect to the server" << std::endl;
}

bool validatePort(const std::string& portStr) {
    for (size_t i = 0; i < portStr.length(); i++) {
        if (!isdigit(portStr[i])) {
            return false;
        }
    }
    
    int port = atoi(portStr.c_str());
    return (port > 1023 && port < 65536);
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Validate port
    if (!validatePort(argv[1])) {
        std::cerr << "Error: Invalid port number. Must be between 1024 and 65535." << std::endl;
        return 1;
    }
    
    // Convert port to integer
    int port = atoi(argv[1]);
    
    // Get password
    std::string password = argv[2];
    
    try {
        // Create and initialize server
        Server server(port, password);
        
        // Initialize the server
        if (!server.initialize()) {
            std::cerr << "Failed to initialize server" << std::endl;
            return 1;
        }
        
        // Start the server
        std::cout << "IRC Server started on port " << port << std::endl;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 