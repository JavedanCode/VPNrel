#pragma once
#include <cstdint>
#include <vector>

class PacketHandler {
public:
    // Encapsulates data with a VPN header and returns a full binary packet
    std::vector<uint8_t> encapsulate(uint16_t clientID, std::vector<uint8_t>& payload);

    // Extracts and verifies payload from a received packet
    bool decapsulate(const std::vector<uint8_t>& packet, uint16_t& outClientID, std::vector <uint8_t>& outPayload);
};
