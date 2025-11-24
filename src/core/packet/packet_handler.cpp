#include "core/packet/packet_handler.hpp"
#include <cstdint>

std::vector<uint8_t> PacketHandler::encapsulate(uint16_t clientID, uint8_t messageType, std::vector<uint8_t>& payload) {
    std::vector<uint8_t> packet;

    uint32_t payloadLen = payload.size();

    packet.reserve(8 + payloadLen);

    packet.push_back(1);

    packet.push_back((clientID >> 8) & 0xFF);
    packet.push_back(clientID & 0xFF);

    packet.push_back((payloadLen >> 24) & 0xFF);
    packet.push_back((payloadLen >> 16) & 0xFF);
    packet.push_back((payloadLen >> 8) & 0xFF);
    packet.push_back(payloadLen & 0xFF);

    packet.push_back(messageType);

    packet.insert(packet.end(), payload.begin(), payload.end());

    return packet;
}

bool PacketHandler::decapsulate(const std::vector<uint8_t>& packet, uint16_t& outClientID, uint32_t& outPayloadLength, uint8_t& outMessageType, std::vector<uint8_t>& outPayload)
{
    if (packet.size() < 8) {
        return false;
    }

    outClientID = (packet[1] << 8) | packet[2];

    outPayloadLength =
        (packet[3] << 24) |
        (packet[4] << 16) |
        (packet[5] << 8)  |
        (packet[6]);

    outMessageType = packet[7];

    if (packet.size() < 8 + outPayloadLength) {
        return false;
    }

    outPayload.clear();
    outPayload.insert(
        outPayload.end(),
        packet.begin() + 8,
        packet.begin() + 8 + outPayloadLength
    );

    return true;
}



