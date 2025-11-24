#include "core/network/socket_handler.hpp"
#include <cstdint>
#include <iostream>
#include <ostream>
#include <winsock2.h>
#include <ws2tcpip.h> // for inet_pton, etc.
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

SocketHandler::SocketHandler() {

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (result != 0) {

        std::cerr << "WSAStartup failed with error : " << result << std::endl;
    }

    sockfd = INVALID_SOCKET;
}

SocketHandler::~SocketHandler() {
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }

    WSACleanup();
}

bool SocketHandler::connectToServer(const std::string& ip, int port) {
    //Create the Socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Error: socket() failed. Code: " << WSAGetLastError() << std::endl;
        return false;
    }

    //Build teh server address struct
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    int result = inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
    if (result <= 0 ) {
        std::cerr << "Error: Invalid IP address format." << std::endl;
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        return false;
    }

    result = connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Error: connect() failed. Code: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        return false;
    }

    std::cout << "Connected to server at " << ip << ":" << port << std::endl;
    return true;
}

bool SocketHandler::createServerSocket(int port) {

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Error: socket() failed. Code: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    int result = bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Error: bind() failed. Code: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        return false;
    }

    result = listen(sockfd, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "Error: listen() failed. Code: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;

}

SOCKET SocketHandler::acceptClient() {
    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(sockfd, (sockaddr*)&clientAddr, &clientSize);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error: accept() failed. Code: " << WSAGetLastError() << std::endl;
        return INVALID_SOCKET;
    }

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);

    std::cout << "Client connected from " << clientIP << ":" << clientPort << std::endl;

    return clientSocket;
}

int SocketHandler::sendData(const std::vector<uint8_t>& data) {
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Error: sendData() called with invalid socket." << std::endl;
        return -1;
    }

    int totalSent = 0;
    int dataSize = static_cast<int>(data.size());

    const char* buffer = reinterpret_cast<const char*>(data.data());

    while (totalSent < dataSize) {
        int bytesSent = send(sockfd, buffer + totalSent, dataSize - totalSent, 0);

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error: send() failed. Code: "  << WSAGetLastError() << std::endl;
            return -1;
        }

        if (bytesSent == 0) {
            break;
        }

        totalSent += bytesSent;
    }
    return totalSent;
    }

void SocketHandler::setSocket(SOCKET socket) {
    sockfd = socket;
}

void SocketHandler::closeSocket() {
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
}

int SocketHandler::receiveData(std::vector<uint8_t>& packetOut)
{
    packetOut.clear();

    static std::vector<uint8_t> buffer;
    const int TEMP_BUF_SIZE = 4096;
    char temp[TEMP_BUF_SIZE];

    int bytesReceived = recv(sockfd, temp, TEMP_BUF_SIZE, 0);

    if (bytesReceived <= 0)
        return bytesReceived;

    buffer.insert(buffer.end(),
                  (uint8_t*)temp,
                  (uint8_t*)temp + bytesReceived);

    if (buffer.size() < 8)
        return -2;

    uint32_t payloadLen =
        (buffer[3] << 24) |
        (buffer[4] << 16) |
        (buffer[5] << 8)  |
         buffer[6];

    uint32_t totalPacketSize = 8 + payloadLen;

    if (buffer.size() < totalPacketSize)
        return -2;

    packetOut.insert(packetOut.end(),
                     buffer.begin(),
                     buffer.begin() + totalPacketSize);

    buffer.erase(buffer.begin(),
                 buffer.begin() + totalPacketSize);

    return totalPacketSize;
}


SOCKET SocketHandler::getRawSocket() const {
    return sockfd;
}



