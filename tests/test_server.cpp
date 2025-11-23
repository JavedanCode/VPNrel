#include <iostream>
#include <vector>
#include <cstdint>
#include "core/network/socket_handler.hpp"

int main() {
    SocketHandler server;

    std::cout << "[SERVER] Starting server on port 5000..." << std::endl;
    if (!server.createServerSocket(5000)) {
        std::cerr << "[SERVER] Failed to create server socket." << std::endl;
        return 1;
    }

    std::cout << "[SERVER] Waiting for a client to connect..." << std::endl;
    SOCKET clientSock = server.acceptClient();
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "[SERVER] Failed to accept client." << std::endl;
        return 1;
    }

    std::cout << "[SERVER] Client connected!" << std::endl;

    // Temporary SocketHandler for the client connection
    SocketHandler clientConnection;
    clientConnection.setSocket(clientSock);

    // Receive data
    std::vector<uint8_t> buffer;
    int bytesReceived = clientConnection.receiveData(buffer);

    if (bytesReceived > 0) {
        std::string msg(buffer.begin(), buffer.end());
        std::cout << "[SERVER] Received message: " << msg << std::endl;
    }

    // Send response
    std::string response = "Hello from server!";
    std::vector<uint8_t> out(response.begin(), response.end());
    clientConnection.sendData(out);

    std::cout << "[SERVER] Response sent. Closing connection." << std::endl;

    return 0;
}
