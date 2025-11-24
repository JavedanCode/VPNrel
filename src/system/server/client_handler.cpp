#include "system/server/client_handler.hpp"
#include "system/server/vpn_server.hpp"
#include <iostream>

ClientHandler::ClientHandler(int clientSocketFD, int clientID, uint8_t key, VPNServer* server)
    : clientID(clientID), sessionKey(key), server(server)
{
    clientSocket.setSocket(clientSocketFD);
    tunnel.initializeTunnel("unknown");
    active = true;
}

void ClientHandler::handleConnection() {
    std::cout << "[ClientHandler] Handling connection for client "
              << clientID << "...\n";

    active = true;
    tcpBuffer.clear();

    while (active) {

        std::vector<uint8_t> recvBuf;
        int bytesReceived = clientSocket.receiveData(recvBuf);

        if (bytesReceived <= 0) {
            std::cout << "[ClientHandler] Client " << clientID
                      << " disconnected.\n";
            active = false;
            server->removeClient(clientID);
            break;
        }

        tcpBuffer.insert(tcpBuffer.end(), recvBuf.begin(), recvBuf.end());

        while (tcpBuffer.size() >= 8) {
            uint32_t payloadLen =
                (static_cast<uint32_t>(tcpBuffer[3]) << 24) |
                (static_cast<uint32_t>(tcpBuffer[4]) << 16) |
                (static_cast<uint32_t>(tcpBuffer[5]) << 8)  |
                 static_cast<uint32_t>(tcpBuffer[6]);

            uint32_t totalSize = 8 + payloadLen;

            if (tcpBuffer.size() < totalSize)
                break; // wait for more data

            std::vector<uint8_t> packet(
                tcpBuffer.begin(),
                tcpBuffer.begin() + totalSize
            );

            tcpBuffer.erase(
                tcpBuffer.begin(),
                tcpBuffer.begin() + totalSize
            );

            uint16_t embeddedID = 0;
            uint32_t payloadLenOut = 0;
            uint8_t messageType = 0;
            std::vector<uint8_t> payload;

            if (!packetHandler.decapsulate(packet,
                                           embeddedID,
                                           payloadLenOut,
                                           messageType,
                                           payload)) {
                std::cerr << "[ClientHandler] Decapsulation failed for client "
                          << clientID << ".\n";
                continue;
            }

            if (messageType != MSG_FILE_END && !payload.empty()) {
                EncryptionModule::decrypt(payload, sessionKey);
            }

            switch (messageType)
            {
                case MSG_TEXT:
                {
                    std::string msg(payload.begin(), payload.end());
                    std::cout << "[ClientHandler] Client " << clientID
                              << " says: " << msg << "\n";

                    // route decrypted text to other clients
                    server->routeMessage(clientID, payload);
                    break;
                }

                case MSG_FILE_START:
                    server->routeFileStart(clientID, payload);
                    break;

                case MSG_FILE_CHUNK:
                    server->routeFileChunk(clientID, payload);
                    break;

                case MSG_FILE_END:
                    server->routeFileEnd(clientID);
                    break;

                default:
                    std::cerr << "[ClientHandler] Unknown messageType "
                              << (int)messageType
                              << " from client " << clientID << "\n";
                    break;
            }
        }
    }

    tunnel.closeTunnel();
    std::cout << "[ClientHandler] Tunnel closed for client "
              << clientID << ".\n";
}

void ClientHandler::sendText(int fromID, const std::vector<uint8_t>& rawPayload)
{

    std::vector<uint8_t> encrypted = rawPayload;
    EncryptionModule::encrypt(encrypted, sessionKey);

    auto packet = packetHandler.encapsulate(fromID, MSG_TEXT, encrypted);
    clientSocket.sendData(packet);
}

void ClientHandler::sendFileStart(int fromID, const std::vector<uint8_t>& payload)
{
    auto encrypted = payload;
    EncryptionModule::encrypt(encrypted, sessionKey);

    auto packet = packetHandler.encapsulate(fromID, MSG_FILE_START, encrypted);
    clientSocket.sendData(packet);
}

void ClientHandler::sendFileChunk(int fromID, const std::vector<uint8_t>& payload)
{
    auto encrypted = payload;
    EncryptionModule::encrypt(encrypted, sessionKey);

    auto packet = packetHandler.encapsulate(fromID, MSG_FILE_CHUNK, encrypted);
    clientSocket.sendData(packet);
}

void ClientHandler::sendFileEnd(int fromID)
{
    std::vector<uint8_t> empty;
    auto packet = packetHandler.encapsulate(fromID, MSG_FILE_END, empty);
    clientSocket.sendData(packet);

}
