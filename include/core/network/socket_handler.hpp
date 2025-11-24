#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <winsock2.h>


/**
 * @class SocketHandler
 * @brief Provides basic socket operations for connecting, sending, and receiving data.
 *
 * This class abstracts low-level socket functions (create, bind, connect, send, receive)
 * so the VPN client and server can use it easily.
 *
 * Step Plan:
 *   Step 1 → Implement TCP client connection.
 *   Step 2 → Add server socket creation and accept().
 *   Step 3 → Add send/receive methods.
 *   Step 4 → Handle cleanup and error checking.
 */
class SocketHandler {
public:
    SocketHandler();
    ~SocketHandler();

    // Connects to a remote server (used by the VPN client)
    bool connectToServer(const std::string& ip, int port);

    // Creates a listening server socket (used by the VPN server)
    bool createServerSocket(int port);

    // Accepts an incoming client connection (returns socket descriptor)
    SOCKET acceptClient();

    // Sends raw data to the connected socket
    int sendData(const std::vector<uint8_t>& data);

    // Receives raw data from the connected socket
    int receiveData(std::vector<uint8_t>& buffer);

    void setSocket(SOCKET socket);

    void closeSocket();

    SOCKET getRawSocket() const;



private:
    SOCKET sockfd;  // Main socket descriptor
    // TODO: Add sockaddr_in members and related fields later
};
