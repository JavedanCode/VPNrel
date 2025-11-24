#pragma once
#include <string>
#include <vector>

#include "core/encryption/encryption.hpp"
#include "core/packet/packet_handler.hpp"
#include "core/network/tunnel_manager.hpp"
#include "core/encryption/key_manager.hpp"
#include "core/network/socket_handler.hpp"
#include "core/packet/message_types.hpp"

class VPNServer;

class ClientHandler {
public:

    ClientHandler(int clientSocketFD, int clientID, uint8_t key, VPNServer* server);

    void handleConnection();

    SocketHandler& getSocket() { return clientSocket; }

    void sendText(int fromID, const std::vector<uint8_t>& rawPayload);
    void sendFileStart(int fromID, const std::vector<uint8_t>& payload);
    void sendFileChunk(int fromID, const std::vector<uint8_t>& payload);
    void sendFileEnd(int fromID);

private:

    SocketHandler clientSocket;
    int clientID;
    bool active = false;

    PacketHandler packetHandler;
    TunnelManager tunnel;
    uint8_t sessionKey = 0;
    VPNServer* server = nullptr;

    std::vector<uint8_t> recvBuffer;
    std::vector<uint8_t> tcpBuffer;
};
