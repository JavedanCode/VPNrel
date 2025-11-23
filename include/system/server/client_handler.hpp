#pragma once
#include <string>

class VPNServer;

#include "core/encryption/encryption.hpp"
#include "core/packet/packet_handler.hpp"
#include "core/network/tunnel_manager.hpp"
#include "core/encryption/key_manager.hpp"
#include "core/network/socket_handler.hpp"

class ClientHandler {
public:
    // Constructor
    ClientHandler(int clientSocketFD, int clientID, uint8_t key, VPNServer* server);
    ClientHandler(int clientSocketFD, int clientID, VPNServer* server);

    // Handles all incoming/outgoing traffic for this client
    void handleConnection();

    SocketHandler& getSocket() { return clientSocket; }

    void sendToClient(const std::string& message);


private:
    SocketHandler clientSocket;   // The client's socket
    int clientID;                // Unique ID assigned by the server
    bool active = false;         // Connection status flag

    PacketHandler packetHandler; // Handles encapsulation/decapsulation
    TunnelManager tunnel;        // Tracks client tunnel state
    uint8_t sessionKey = 0;      // Shared symmetric key for this session
    VPNServer* server;
};
