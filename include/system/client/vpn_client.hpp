#pragma once
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include "core/network/socket_handler.hpp"
#include "core/network/tunnel_manager.hpp"
#include "core/packet/packet_handler.hpp"
#include "core/encryption/encryption.hpp"
#include "core/encryption/key_manager.hpp"

struct FileChunkData {
    std::vector<uint8_t> chunk;
};

class VPNClient {
public:
    bool connectToServer(const std::string& ip, int port);
    void sendMessage(const std::string& message);
    void receiveLoop();
    void run();
    void sendFile(const std::string& filepath);

private:
    SocketHandler socket;
    TunnelManager tunnel;
    PacketHandler packetHandler;

    uint8_t sessionKey = 0;
    bool active = false;

    std::vector<uint8_t> tcpBuffer;

    std::ofstream outFile;
    std::string currentFilename;
    uint32_t expectedSize = 0;
    uint32_t received = 0;
    bool receivingFile = false;
};
