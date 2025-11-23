#pragma once
#include <string>
#include "core/network/socket_handler.hpp"
#include "core/network/tunnel_manager.hpp"
#include "core/packet/packet_handler.hpp"
#include "core/encryption/encryption.hpp"
#include "core/encryption/key_manager.hpp"

class VPNClient {
public:
    // Establishes a connection with the VPN server
    bool connectToServer(const std::string& ip, int port);

    // Sends an encrypted, encapsulated message to the server
    void sendMessage(const std::string& message);

    // Continuously listens for messages from the server
    void receiveLoop();

    // Main client execution loop (entry point)
    void run();

private:
    SocketHandler socket;          // Handles network connection
    TunnelManager tunnel;          // Tracks tunnel state
    PacketHandler packetHandler;   // Handles encapsulation and checksums

    uint8_t sessionKey = 0;        // Symmetric encryption key
    bool active = false;
};
