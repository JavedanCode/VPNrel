#include "core/encryption/key_manager.hpp"
#include "core/network/socket_handler.hpp"
#include <vector>
#include <iostream>
#include <ostream>
#include <random>

namespace KeyManager {


    uint8_t generateSessionKey() {
        std::random_device rd;
        uint8_t key = static_cast<uint8_t>(rd() & 0xFF);

        if (key == 0) {
            key = 0x5A;
        }

        return key;
    }

    bool sendSessionKey(SocketHandler& serverSocket, uint8_t key) {

        std::vector<uint8_t> buffer;
        buffer.push_back(key);

        int bytesSent = serverSocket.sendData(buffer);

        if (bytesSent != 1) {
            std::cerr << "[KeyManager] Failed to send session key. Bytes sent: " << bytesSent << std::endl;
            return false;
        }

        std::cout << "[KeyManager] Session key sent to client." << std::endl;
        return true;
    }

    bool receiveSessionKey(SocketHandler& serverSocket, uint8_t& outKey) {

        std::vector<uint8_t> buffer;

        int bytesReceived = serverSocket.receiveData(buffer);

        if (bytesReceived <= 0) {
            std::cerr << "[KeyManger] Failed to receive session key. Bytes received: " << bytesReceived << std::endl;
            return false;
        }

        outKey = buffer[0];

        std::cout << "[KeyManager] Session key received from server." << std::endl;
        return true;
    }
}
