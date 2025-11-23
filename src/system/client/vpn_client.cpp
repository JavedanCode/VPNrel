// src/system/client/vpn_client.cpp
#include "system/client/vpn_client.hpp"
#include <iostream>
#include <thread>
#include <atomic>

// Connect to server (already implemented in hpp/cpp, keep as-is)
bool VPNClient::connectToServer(const std::string& ip, int port) {
    std::cout << "[CLIENT] Connecting to " << ip << ":" << port << "...\n";

    if (!socket.connectToServer(ip, port)) {
        std::cerr << "[CLIENT] Failed to connect to server.\n";
        return false;
    }

    std::cout << "[CLIENT] Connected!\n";
    return true;
}

// Convert and send a single message (thread-safe wrt single sender)
void VPNClient::sendMessage(const std::string& message) {
    if (!active) {
        std::cerr << "[CLIENT] Not active. Cannot send message.\n";
        return;
    }

    // Convert to binary payload
    std::vector<uint8_t> payload(message.begin(), message.end());

    // Encrypt payload in-place
    EncryptionModule::encrypt(payload, sessionKey);

    // Use clientID 0 for now (server assigns/knows client). You can change if needed.
    uint16_t clientID = 0;
    std::vector<uint8_t> packet = packetHandler.encapsulate(clientID, payload);

    int bytesSent = socket.sendData(packet);
    if (bytesSent <= 0) {
        std::cerr << "[CLIENT] Failed to send packet. Sent bytes: " << bytesSent << "\n";
        // Optionally decide to deactivate
    } else {
        std::cout << "[CLIENT] Sent " << bytesSent << " bytes (encrypted packet).\n";
    }
}

// Runs in background thread to receive and process incoming packets
void VPNClient::receiveLoop() {
    std::cout << "[CLIENT] Receive loop started.\n";

    while (true) {
        std::vector<uint8_t> packetBytes;
        int bytesReceived = socket.receiveData(packetBytes);

        if (bytesReceived <= 0) {
            std::cout << "[CLIENT] Receive loop terminating: socket closed or read error.\n";
            break;
        }

        if (packetBytes.size() < 7) {
            std::cerr << "[CLIENT] Received malformed packet (too small). Ignoring.\n";
            continue;
        }

        uint16_t fromClientID = 0;
        std::vector<uint8_t> payload;
        if (!packetHandler.decapsulate(packetBytes, fromClientID, payload)) {
            std::cerr << "[CLIENT] Failed to decapsulate incoming packet. Ignoring.\n";
            continue;
        }

        EncryptionModule::decrypt(payload, sessionKey);
        std::string msg(payload.begin(), payload.end());

        std::cout << "[CLIENT] Received from server (clientID field=" << fromClientID << "): "
                  << msg << "\n";
    }

    std::cout << "[CLIENT] Receive loop terminated.\n";
}


// Main run loop: connect, handshake, start receiver, send messages from stdin
void VPNClient::run() {
    std::string ip = "127.0.0.1";
    int port = 5000;

    if (!connectToServer(ip, port)) {
        return;
    }

    // Receive session key from server handshake
    if (!KeyManager::receiveSessionKey(socket, sessionKey)) {
        std::cerr << "[CLIENT] Failed to receive session key. \n";
        return;
    }

    std::cout << "[CLIENT] Session key received: " << (int)sessionKey << "\n";

    // Activate tunnel state on client side (optional metadata)
    tunnel.initializeTunnel(ip);
    active = true;

    // Start background receive thread
    std::thread receiver(&VPNClient::receiveLoop, this);

    // Main send loop (user input). Type "quit" to exit.
    std::cout << "Enter messages to send to the VPN server. Type /quit to exit.\n";
    while (active) {

        std::string line;

        if (!std::getline(std::cin, line)) {
            std::cout << "[CLIENT] Warning: stdin not ready. Retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue; // DO NOT EXIT
        }

        if (line.empty()) continue;


        if (line == "/quit" || line == "quit") {
            std::cout << "[CLIENT] Quit requested. Shutting down.\n";
            active = false;
            break;
        }

        if (line.empty()) continue;

        sendMessage(line);
    }

    // Clean shutdown: set active false and join receive thread
    active = false;

    socket.closeSocket();

    if (receiver.joinable()) receiver.join();

    // Optionally close socket via destructor of SocketHandler when vpn_client object goes out of scope.
    tunnel.closeTunnel();

    std::cout << "[CLIENT] Client stopped.\n";
}
