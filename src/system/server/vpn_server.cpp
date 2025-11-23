#include "system/server/vpn_server.hpp"
#include <iostream>


void VPNServer::start(int port) {
    std::cout << "[SERVER] Starting VPN server on port " << port << "...\n";

    if (!serverSocket.createServerSocket(port)) {
        std::cerr << "[SERVER] ERROR: Could not create listening socket.\n";
        return;
    }

    running = true;
    std::cout << "[SERVER] Server is running. Waiting for clients...\n";

    // Begin accepting clients (blocking loop)
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

        // 1) Generate session key
        uint8_t key = KeyManager::generateSessionKey();

        // 2) Create handler (handler owns the socket)
        ClientHandler* handler = new ClientHandler(clientFD, assignedID, key, this);

        // 3) Add to registry BEFORE starting thread
        connectedClients[assignedID] = handler;
        std::cout << "[SERVER] Client " << assignedID << " added to registry.\n";

        // 4) Send session key using the handler's socket
        if (!KeyManager::sendSessionKey(handler->getSocket(), key)) {
            std::cerr << "[SERVER] Failed to send session key. Closing client.\n";

            // Remove from registry & free handler
            connectedClients.erase(assignedID);
            delete handler;  // SocketHandler destructor will close the FD

            continue;
        }

        // 5) Launch thread to handle this client
        clientThreads.emplace_back(&ClientHandler::handleConnection, handler);
    }
}



void VPNServer::shutdown() {
    std::cout << "[SERVER] Shutting down VPN server...\n";

    running = false;

    // Join all client threads
    for (auto& th : clientThreads) {
        if (th.joinable())
            th.join();
    }

    std::cout << "[SERVER] All client threads terminated.\n";

    // SocketHandler destructor will close listening socket
}

void VPNServer::routeMessage(int fromClientID, const std::string& message) {
    std::cout << "[SERVER] Routing message from client " << fromClientID << "...\n";

    for (auto& [clientID, handler] : connectedClients) {

        if (clientID == fromClientID)
            continue;   // Do not send message back to sender

        if (!handler) {
            std::cerr << "[SERVER] Null handler for client " << clientID << "\n";
            continue;
        }

        // Forward message
        handler->sendToClient("[From " + std::to_string(fromClientID) + "] " + message);

        std::cout << "[SERVER] Forwarded message to client " << clientID << "\n";
    }
}

void VPNServer::removeClient(int clientID) {
    std::cout << "[SERVER] Removing client " << clientID << "...\n";

    auto it = connectedClients.find(clientID);
    if (it != connectedClients.end()) {
        delete it->second;  // free ClientHandler
        connectedClients.erase(it);
    }
}


