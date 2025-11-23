#include <iostream>
#include <vector>
#include <cstdint>
#include "core/network/socket_handler.hpp"

int main() {
    SocketHandler client;

    std::cout << "[CLIENT] Connecting to server..." << std::endl;
    if (!client.connectToServer("127.0.0.1", 5000)) {
        std::cerr << "[CLIENT] Failed to connect to server." << std::endl;
        return 1;
    }

    std::cout << "[CLIENT] Connected!" << std::endl;

    // Send a message to the server
    std::string message = "Hello from client!";
    std::vector<uint8_t> data(message.begin(), message.end());
    client.sendData(data);

    // Receive response
    std::vector<uint8_t> buffer;
    int bytesReceived = client.receiveData(buffer);

    if (bytesReceived > 0) {
        std::string msg(buffer.begin(), buffer.end());
        std::cout << "[CLIENT] Received response: " << msg << std::endl;
    }

    return 0;
}
