#include "system/server/vpn_server.hpp"

int main() {
    VPNServer server;
    server.start(5000);   // same port as client
    return 0;
}