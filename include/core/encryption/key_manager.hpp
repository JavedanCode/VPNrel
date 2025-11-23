#pragma once
#include <cstdint>
#include "core/network/socket_handler.hpp"

namespace KeyManager {

    // Server: generates a random session key (1 byte)
    uint8_t generateSessionKey();

    // Server: sends the session key to the client over TCP
    bool sendSessionKey(SocketHandler& clientSocket, uint8_t key);

    // Client: receives the session key from the server
    bool receiveSessionKey(SocketHandler& serverSocket, uint8_t& outKey);

}
