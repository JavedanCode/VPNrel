#include "system/server/client_handler.hpp"
#include "utils/logger/logger.hpp"
#include <iostream>

#include "system/server/vpn_server.hpp"


ClientHandler::ClientHandler(int clientSocketFD, int clientID, uint8_t key, VPNServer* server)
    : clientID(clientID), sessionKey(key), server(server)
{
    clientSocket.setSocket(clientSocketFD);
    tunnel.initializeTunnel("unknown");
    active = true;
}

ClientHandler::ClientHandler(int clientSocketFD, int clientID, VPNServer* server)
    : clientID(clientID), server(server)
{
    clientSocket.setSocket(clientSocketFD);
    tunnel.initializeTunnel("unknown");
    active = true;
}





void ClientHandler::handleConnection() {
    std::cout << "[ClientHandler] Handling connection for client " << clientID << "...\n";

    // Mark tunnel active (already done in constructor, but good for clarity)
    tunnel.initializeTunnel("unknown");

    // Now we move on to receiving packets continuously in Phase 2
    // We'll add the loop next.

    // -------------------------------------------
    // STEP 2 — Receive loop
    // -------------------------------------------
    std::cout << "[ClientHandler] Entering receive loop for client " << clientID << "...\n";

    while (active) {
        std::vector<uint8_t> packetBytes;

        // Read data from client
        int bytesReceived = clientSocket.receiveData(packetBytes);

        // Case 1: Client disconnected
        if (bytesReceived <= 0) {
            std::cout << "[ClientHandler] Client " << clientID
                      << " disconnected.\n";
            active = false;
            if (server) {
                server->removeClient(clientID);
            }
            break;
        }

        // Case 2: Not enough data for a packet header
        if (packetBytes.size() < 7) { // version(1) + clientID(2) + length(4)
            std::cerr << "[ClientHandler] Received malformed packet from client "
                      << clientID << ".\n";
            continue;  // skip and wait for next
        }

        // Decapsulate packet
        uint16_t extractedClientID = 0;
        std::vector<uint8_t> payload;

        if (!packetHandler.decapsulate(packetBytes, extractedClientID, payload)) {
            std::cerr << "[ClientHandler] Failed to decapsulate packet from client "
                      << clientID << ".\n";
            continue;
        }

        // Decrypt message
        EncryptionModule::decrypt(payload, sessionKey);

        // Convert decrypted payload into string
        std::string message(payload.begin(), payload.end());

        // Log message
        std::cout << "[ClientHandler] Client " << clientID
                  << " says: " << message << "\n";

        // Route the message to other clients
        if (server) {
            server->routeMessage(clientID, message);
        }
    }

    // Cleanup
    tunnel.closeTunnel();
    std::cout << "[ClientHandler] Tunnel closed for client " << clientID << ".\n";
}

void ClientHandler::sendToClient(const std::string& message) {
    if (!active) {
        std::cerr << "[ClientHandler] Cannot send, client inactive.\n";
        return;
    }

    // Convert message → binary
    std::vector<uint8_t> payload(message.begin(), message.end());

    // Encrypt using session key
    EncryptionModule::encrypt(payload, sessionKey);

    // Use the clientID assigned by the server
    uint16_t id = clientID;

    // Build packet
    std::vector<uint8_t> packet = packetHandler.encapsulate(id, payload);

    // Send packet
    int bytesSent = clientSocket.sendData(packet);

    if (bytesSent <= 0) {
        std::cerr << "[ClientHandler] Failed to send message to client "
                  << clientID << ".\n";
    } else {
        std::cout << "[ClientHandler] Sent " << bytesSent
                  << " bytes to client " << clientID << ".\n";
    }
}


