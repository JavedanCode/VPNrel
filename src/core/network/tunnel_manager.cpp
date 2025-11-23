#include "core/network/tunnel_manager.hpp"

bool TunnelManager::initializeTunnel(const std::string& clientIP) {
    this->clientIP = clientIP;
    active = true;
    return true;
}

void TunnelManager::closeTunnel() {
    active = false;
}

bool TunnelManager::isActive() const {
    return active;
}
