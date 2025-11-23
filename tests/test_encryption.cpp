#include <iostream>
#include <vector>
#include <cstdint>
#include "core/encryption/encryption.hpp"

int main() {
    // Original plaintext
    std::string message = "Hello VPN!";
    std::vector<uint8_t> data(message.begin(), message.end());

    // Copy original for comparison
    std::vector<uint8_t> originalData = data;

    uint8_t key = 0x5A; // sample XOR key

    // Encrypt
    EncryptionModule::encrypt(data, key);

    std::cout << "[TEST] Encrypted bytes: ";
    for (uint8_t b : data) {
        std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::dec << std::endl;

    // Decrypt
    EncryptionModule::decrypt(data, key);

    // Convert decrypted bytes back to string
    std::string decrypted(data.begin(), data.end());

    std::cout << "[TEST] Decrypted message: " << decrypted << std::endl;

    // Verify
    if (data == originalData) {
        std::cout << "[SUCCESS] Decryption matches original!" << std::endl;
    } else {
        std::cout << "[FAIL] Decryption does NOT match original!" << std::endl;
    }

    return 0;
}
