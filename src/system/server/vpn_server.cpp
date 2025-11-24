#include "system/server/vpn_server.hpp"
#include "core/packet/message_types.hpp"
#include <iostream>

void VPNServer::start(int port) {
    std::cout << "[SERVER] Starting VPN server on port " << port << "...\n";

    if (!serverSocket.createServerSocket(port)) {
        std::cerr << "[SERVER] ERROR: Could not create listening socket.\n";
        return;
    }

    running = true;
    std::cout << "[SERVER] Server is running. Waiting for clients...\n";

    acceptClients();
}

void VPNServer::acceptClients() {

    while (running) {
        int clientFD = serverSocket.acceptClient();

        if (clientFD == -1) {
            std::cerr << "[SERVER] Failed to accept client.\n";
            continue;
        }

        int assignedID = nextClientID++;

        std::cout << "[SERVER] Accepted new client. Assigned ID = "
                  << assignedID << "\n";

        uint8_t key = KeyManager::generateSessionKey();

        ClientHandler* handler = new ClientHandler(clientFD, assignedID, key, this);

        connectedClients[assignedID] = handler;
        std::cout << "[SERVER] Client " << assignedID << " added to registry.\n";

        if (!KeyManager::sendSessionKey(handler->getSocket(), key)) {
            std::cerr << "[SERVER] Failed to send session key. Closing client.\n";

            connectedClients.erase(assignedID);
            delete handler;

            continue;
        }

        clientThreads.emplace_back(&ClientHandler::handleConnection, handler);
    }
}



void VPNServer::shutdown() {
    std::cout << "[SERVER] Shutting down VPN server...\n";

    running = false;

    for (auto& th : clientThreads) {
        if (th.joinable())
            th.join();
    }

    std::cout << "[SERVER] All client threads terminated.\n";

}

void VPNServer::routeMessage(int fromClientID, const std::vector<uint8_t>& payload) {

    std::cout << "[SERVER] Routing TEXT message from client "
              << fromClientID << "...\n";

    for (auto& [clientID, handler] : connectedClients)
    {
        if (!handler) continue;
        if (clientID == fromClientID) continue;

        handler->sendText(fromClientID, payload);
    }
}

void VPNServer::removeClient(int clientID) {
    auto it = connectedClients.find(clientID);
    if (it == connectedClients.end())
        return;

    std::cout << "[SERVER] Removing client " << clientID << "...\n";

    delete it->second;
    connectedClients.erase(it);
}

void VPNServer::routeFileStart(int fromClientID, const std::vector<uint8_t>& payload)
{
    std::cout << "[SERVER] Routing FILE_START from client " << fromClientID << "...\n";

    for (auto& [clientID, handler] : connectedClients)
    {
        if (!handler) continue;
        if (clientID == fromClientID) continue;

        handler->sendFileStart(fromClientID, payload);
    }
}

void VPNServer::routeFileChunk(int fromClientID, const std::vector<uint8_t>& payload)
{
    std::cout << "[SERVER] Routing FILE_CHUNK from client " << fromClientID << "...\n";

    for (auto& [clientID, handler] : connectedClients)
    {
        if (!handler) continue;
        if (clientID == fromClientID) continue;

        handler->sendFileChunk(fromClientID, payload);
    }
}

void VPNServer::routeFileEnd(int fromClientID)
{
    std::cout << "[SERVER] Routing FILE_END from client " << fromClientID << "...\n";

    for (auto& [clientID, handler] : connectedClients)
    {
        if (!handler) continue;
        if (clientID == fromClientID) continue;

        handler->sendFileEnd(fromClientID);
    }
}





