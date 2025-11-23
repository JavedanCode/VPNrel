#pragma once
#include <string>

/**
 * @class TunnelManager
 * @brief Manages the lifecycle and state of the VPN tunnel.
 *
 * This class keeps track of whether a tunnel between the client and server
 * is active, closed, or needs reinitialization.
 *
 * Step Plan:
 *   Step 1 → Add simple active flag management.
 *   Step 2 → Integrate with SocketHandler to monitor connection state.
 *   Step 3 → Add statistics or heartbeat (optional).
 */
class TunnelManager {
public:
    // Initializes or starts a tunnel session
    bool initializeTunnel(const std::string& clientIP);

    // Closes or tears down the tunnel
    void closeTunnel();

    // Checks if the tunnel is currently active
    bool isActive() const;

private:
    bool active = false;     // true when tunnel is active
    std::string clientIP;    // optional: store client address for reference
};
