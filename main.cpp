#include <iostream>
#include "system/client/vpn_client.hpp"
#include "system/server/vpn_server.hpp"
#include "utils/tester/tester.hpp"
#include "utils/logger/logger.hpp"
#include "utils/config/config.hpp"

/**
 * @file main.cpp
 * @brief Entry point for the VPN project.
 *
 * This file will later allow you to launch the VPN client or server
 * depending on runtime parameters or configuration.
 *
 * Step Plan:
 *   Step 1 → Print startup banner.
 *   Step 2 → Add test calls (Tester::testXYZ()) for early module validation.
 *   Step 3 → Add config file reading (Config).
 *   Step 4 → Add command-line argument handling for "client" or "server" mode.
 *   Step 5 → Run VPNClient::run() or VPNServer::start() accordingly.
 */

int main(int argc, char* argv[]) {
    std::cout << "=============================\n";
    std::cout << "     VPN Project Skeleton     \n";
    std::cout << "=============================\n\n";

    // TODO: (Step 1) Load configuration file if available
    // Config config;
    // config.load("config.txt");

    // TODO: (Step 2) Use command-line arguments to choose mode
    // Example: ./vpn client OR ./vpn server
    // For now, just print instructions:
    std::cout << "Usage: ./vpn [client|server|test]\n\n";

    // TODO: (Step 3) Placeholder for testing individual modules
    // Example:
    // Tester::testEncryption();
    // Tester::testConnection();
    // Tester::testPacketHandling();

    // TODO: (Step 4) Placeholder for starting server or client
    // VPNServer server;
    // server.start(8080);

    // VPNClient client;
    // client.connectToServer("127.0.0.1", 8080);
    // client.run();

    std::cout << "Program finished (skeleton stage)." << std::endl;
    return 0;
}
