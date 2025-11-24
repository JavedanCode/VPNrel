#include "core/encryption/key_manager.hpp"
#include "core/network/socket_handler.hpp"
#include <vector>
#include <iostream>
#include <ostream>
#include <random>
#include <thread>

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

    bool receiveSessionKey(SocketHandler& socket, uint8_t& outKey) {
        char keyByte = 0;

        while (true)
        {
            int n = recv(socket.getRawSocket(), &keyByte, 1, 0);

            if (n == 1) {
                outKey = static_cast<uint8_t>(keyByte);
                std::cout << "[KeyManager] Session key received: " << (int)outKey << "\n";
                return true;
            }

            if (n == 0) {
                std::cerr << "[KeyManager] Connection closed while receiving key.\n";
                return false;
            }

            if (n == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    // just try again
                    continue;
                }

                std::cerr << "[KeyManager] recv() error: " << err << "\n";
                return false;
            }
        }

        return false;
    }

}
