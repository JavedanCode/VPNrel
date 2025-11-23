#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include "core/network/socket_handler.hpp"
#include "system/server/client_handler.hpp"
#include "utils/logger/logger.hpp"
#include <unordered_map>

class VPNServer {
public:
    // Starts the VPN server on a given port
    void start(int port);

    // Accepts incoming clients and spawns handler threads
    void acceptClients();

    // Gracefully shuts down the server and connected clients
    void shutdown();

    void routeMessage(int fromClientID, const std::string& message);

    void removeClient(int clientID);


private:
    std::unordered_map<int, ClientHandler*> connectedClients;
    SocketHandler serverSocket;                // Main listening socket
    std::vector<std::thread> clientThreads;    // Threads handling each client
    std::atomic<bool> running = false;         // Server run-state flag
    int nextClientID = 1;                      // ID counter for connected clients
};
