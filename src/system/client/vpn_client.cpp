#include "system/client/vpn_client.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iterator>


bool VPNClient::connectToServer(const std::string& ip, int port) {
    std::cout << "[CLIENT] Connecting to " << ip << ":" << port << "...\n";

    if (!socket.connectToServer(ip, port)) {
        std::cerr << "[CLIENT] Failed to connect to server.\n";
        return false;
    }

    std::cout << "[CLIENT] Connected!\n";
    return true;
}

void VPNClient::sendMessage(const std::string& message) {
    if (!active) {
        std::cerr << "[CLIENT] Not active. Cannot send message.\n";
        return;
    }

    std::vector<uint8_t> payload(message.begin(), message.end());
    EncryptionModule::encrypt(payload, sessionKey);

    auto packet = packetHandler.encapsulate(0, MSG_TEXT, payload);


    int bytesSent = socket.sendData(packet);
    if (bytesSent <= 0)
        std::cerr << "[CLIENT] Failed to send packet.\n";
    else
        std::cout << "[CLIENT] Sent " << bytesSent << " bytes.\n";
}

void VPNClient::receiveLoop() {
    std::cout << "[CLIENT] Receive loop started.\n";

    std::vector<uint8_t> tcpBuffer;

    bool receivingFile = false;
    std::ofstream out;
    std::string filename;
    uint32_t expectedSize = 0;
    uint32_t received = 0;

    while (active) {

        std::vector<uint8_t> recvBuf;
        int bytesReceived = socket.receiveData(recvBuf);

        if (bytesReceived <= 0) {
            // 0 => clean disconnect, <0 => error
            break;
        }


        tcpBuffer.insert(tcpBuffer.end(), recvBuf.begin(), recvBuf.end());


        while (tcpBuffer.size() >= 8) {


            uint32_t payloadLength =
                (static_cast<uint32_t>(tcpBuffer[3]) << 24) |
                (static_cast<uint32_t>(tcpBuffer[4]) << 16) |
                (static_cast<uint32_t>(tcpBuffer[5]) << 8)  |
                 static_cast<uint32_t>(tcpBuffer[6]);

            uint32_t totalSize = 8 + payloadLength;

            if (tcpBuffer.size() < totalSize)
                break;

            std::vector<uint8_t> packet(
                tcpBuffer.begin(),
                tcpBuffer.begin() + totalSize
            );

            tcpBuffer.erase(
                tcpBuffer.begin(),
                tcpBuffer.begin() + totalSize
            );

            uint16_t fromClientID = 0;
            uint32_t payloadLengthOut = 0;
            uint8_t messageType = 0;
            std::vector<uint8_t> payload;

            if (!packetHandler.decapsulate(packet,
                                           fromClientID,
                                           payloadLengthOut,
                                           messageType,
                                           payload)) {
                std::cerr << "[CLIENT] Failed to decapsulate incoming packet.\n";
                continue;
            }

            // decrypt payload (we encrypt all except possibly empty FILE_END)
            if (!payload.empty())
                EncryptionModule::decrypt(payload, sessionKey);

            if (messageType == MSG_FILE_END && payloadLengthOut == 0 && !receivingFile) {
                // ignore stray empty FILE_END
                continue;
            }



            switch (messageType)
            {
                case MSG_TEXT:
                {
                    std::string msg(payload.begin(), payload.end());
                    std::cout << "[CLIENT] Message from "
                              << fromClientID << ": " << msg << "\n";
                    break;
                }

                case MSG_FILE_START:
                {

                    if (payload.size() < 2 + 4) {
                        std::cerr << "[CLIENT] FILE_START meta malformed (too short)\n";
                        break;
                    }
                    std::cout << "[CLIENT] Saving to absolute path: " << std::filesystem::absolute("RECEIVED_" + filename) << "\n";


                    uint16_t nameLen =
                        (static_cast<uint16_t>(payload[0]) << 8) |
                         static_cast<uint16_t>(payload[1]);

                    if (payload.size() < 2 + nameLen + 4) {
                        std::cerr << "[CLIENT] FILE_START meta malformed (nameLen mismatch)\n";
                        break;
                    }

                    filename = std::string(
                        payload.begin() + 2,
                        payload.begin() + 2 + nameLen
                    );

                    expectedSize =
                        (static_cast<uint32_t>(payload[2 + nameLen])     << 24) |
                        (static_cast<uint32_t>(payload[3 + nameLen])     << 16) |
                        (static_cast<uint32_t>(payload[4 + nameLen])     << 8)  |
                         static_cast<uint32_t>(payload[5 + nameLen]);

                    out.open("RECEIVED_" + filename, std::ios::binary);
                    if (!out.is_open()) {
                        std::cerr << "[CLIENT] ERROR: unable to create local file.\n";
                        receivingFile = false;
                        break;
                    }

                    receivingFile = true;
                    received = 0;

                    std::cout << "[CLIENT] Receiving file: " << filename
                              << " (" << expectedSize << " bytes)\n";
                    break;
                }

                case MSG_FILE_CHUNK:
                {
                    if (!receivingFile) {
                        std::cerr << "[CLIENT] Got file chunk but no file open\n";
                        break;
                    }

                    if (!payload.empty()) {
                        out.write(reinterpret_cast<const char*>(payload.data()),
                                  static_cast<std::streamsize>(payload.size()));
                        received += static_cast<uint32_t>(payload.size());

                        std::cout << "[CLIENT] Received chunk: "
                                  << payload.size() << " bytes ("
                                  << received << "/" << expectedSize << ")\n";

                        if (received >= expectedSize) {
                            std::cout << "[CLIENT] Reached expected size (" << expectedSize
                                      << ") without FILE_END. Closing file anyway.\n";
                            if (out.is_open())
                                out.close();
                            receivingFile = false;

                            std::cout << "[CLIENT] File transfer COMPLETE. Saved as: "
                                      << "RECEIVED_" << filename << "\n";
                        }
                    }
                    break;
                }


                case MSG_FILE_END:
                {
                    if (!receivingFile) {
                        //ignore
                        break;
                    }

                    if (out.is_open())
                        out.close();
                    receivingFile = false;

                    std::cout << "[CLIENT] File transfer COMPLETE. Saved as: "
                              << "RECEIVED_" << filename << "\n";
                    break;
                }


                default:
                    std::cerr << "[CLIENT] Unknown packet type\n";
                    break;
            }
        }
    }

    if (receivingFile && out.is_open()) {
        out.close();
    }

    std::cout << "[CLIENT] Receive loop terminated.\n";
}

void VPNClient::run() {
    std::string ip = "127.0.0.1";
    int port = 5000;

    if (!connectToServer(ip, port)) {
        return;
    }

    if (!KeyManager::receiveSessionKey(socket, sessionKey)) {
        std::cerr << "[CLIENT] Failed to receive session key. \n";
        return;
    }

    std::cout << "[CLIENT] Session key received: " << (int)sessionKey << "\n";

    tunnel.initializeTunnel(ip);
    active = true;

    std::thread receiver(&VPNClient::receiveLoop, this);

    std::cout << "Enter messages to send to the VPN server. Type /quit to exit.\n";
    while (active) {

        std::string line;

        if (!std::getline(std::cin, line)) {
            std::cout << "[CLIENT] Warning: stdin not ready. Retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (line.empty()) continue;

        if (line == "/quit" || line == "quit") {
            std::cout << "[CLIENT] Quit requested. Shutting down.\n";
            active = false;
            break;
        }

        if (line.empty()) continue;

        if (line.rfind("/sendfile ", 0) == 0) {
            std::string path = line.substr(10);
            sendFile(path);
        } else {
            sendMessage(line);
        }

    }

    active = false;
    socket.closeSocket();
    if (receiver.joinable()) receiver.join();
    tunnel.closeTunnel();
    std::cout << "[CLIENT] Client stopped.\n";
}

void VPNClient::sendFile(const std::string& filepath) {
    if (!active) {
        std::cerr << "[CLIENT] Cannot send file, client inactive.\n";
        return;
    }

    if (!std::filesystem::exists(filepath)) {
        std::cerr << "[CLIENT] ERROR: File does not exist.\n";
        return;
    }

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[CLIENT] ERROR: Could not open file.\n";
        return;
    }

    std::vector<uint8_t> fileBytes{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    file.close();

    uint32_t fileSize = static_cast<uint32_t>(fileBytes.size());
    std::string filename = std::filesystem::path(filepath).filename().string();

    std::cout << "[CLIENT] Sending file: " << filename
              << " (" << fileSize << " bytes)\n";
    {
        std::vector<uint8_t> meta;
        // filename length
        uint16_t len = filename.size();
        meta.push_back((len >> 8) & 0xFF);
        meta.push_back(len & 0xFF);
        // filename bytes
        meta.insert(meta.end(), filename.begin(), filename.end());
        // file size (4 bytes)
        meta.push_back((fileSize >> 24) & 0xFF);
        meta.push_back((fileSize >> 16) & 0xFF);
        meta.push_back((fileSize >> 8) & 0xFF);
        meta.push_back(fileSize & 0xFF);

        EncryptionModule::encrypt(meta, sessionKey);

        auto packet = packetHandler.encapsulate(0, MSG_FILE_START, meta);
        socket.sendData(packet);
    }

    const size_t CHUNK_SIZE = 4088;

    for (size_t offset = 0; offset < fileSize; offset += CHUNK_SIZE) {
        size_t remaining = fileSize - offset;
        size_t size = std::min(CHUNK_SIZE, remaining);

        std::vector<uint8_t> chunk(fileBytes.begin() + offset, fileBytes.begin() + offset + size);

        EncryptionModule::encrypt(chunk, sessionKey);

        auto packet = packetHandler.encapsulate(0, MSG_FILE_CHUNK, chunk);
        socket.sendData(packet);

        std::cout << "[CLIENT] Sent chunk: " << size << " bytes\n";
    }

    {
        std::vector<uint8_t> empty;
        auto packet = packetHandler.encapsulate(0, MSG_FILE_END, empty);
        socket.sendData(packet);

    }

    std::cout << "[CLIENT] File sent successfully.\n";
}
