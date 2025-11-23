#include "core/packet/packet_handler.hpp"
#include <cstdint>

std::vector<uint8_t> PacketHandler::encapsulate(uint16_t clientID, std::vector<uint8_t>& payload) {
    std::vector<uint8_t> packet;

    uint8_t version = 1;
    packet.push_back(version);

    packet.push_back((clientID >> 8) & 0xFF);
    packet.push_back((clientID & 0xFF));

    uint32_t payloadLen = static_cast<uint32_t>(payload.size());

    packet.push_back((payloadLen >> 24) & 0xFF);
    packet.push_back((payloadLen >> 16) & 0xFF);
    packet.push_back((payloadLen >> 8) & 0xFF);
    packet.push_back((payloadLen & 0xFF));

    packet.insert(packet.end(), payload.begin(), payload.end());

    return packet;
}

bool PacketHandler::decapsulate(const std::vector<uint8_t>& packet, uint16_t& outClientID, std::vector <uint8_t>& outPayload) {
    const size_t HEADER_SIZE = 7;
    if (packet.size() < HEADER_SIZE) {
        return false;
    }

    size_t index = 0;

    uint8_t version = packet[index++];
    if (version != 1) {
        return false;
    }

    uint16_t clientID = (packet[index] << 8 | packet[index + 1]);
    index += 2;

    uint32_t payloadLen = (packet[index] << 24 ) | (packet[index + 1] << 16) | (packet[index + 2] << 8) | packet[index + 3];
    index += 4;


    if (packet.size() < HEADER_SIZE + payloadLen) {
        return false;
    }

    outPayload.clear();
    outPayload.insert(outPayload.end(), packet.begin() + index, packet.begin() + index + payloadLen);
    outClientID = clientID;
    return true;
}



