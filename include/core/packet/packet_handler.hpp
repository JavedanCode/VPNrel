#pragma once
#include <cstdint>
#include <vector>

enum MessageType : uint8_t {
    MSG_TEXT = 0,
    MSG_FILE_START = 1,
    MSG_FILE_CHUNK = 2,
    MSG_FILE_END = 3
};

class PacketHandler {
public:
    // Encapsulates data with a VPN header and returns a full binary packet
    std::vector<uint8_t> encapsulate(uint16_t clientID, uint8_t messageType ,std::vector<uint8_t>& payload);

    // Extracts and verifies payload from a received packet
    bool decapsulate(const std::vector<uint8_t>& packet, uint16_t& outClientID, uint32_t& outPayloadLength, uint8_t& outMessageType, std::vector<uint8_t>& outPayload);


};
