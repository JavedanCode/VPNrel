#pragma once
#include <vector>
#include <cstdint>

namespace EncryptionModule {

    // Encrypts binary data in-place using XOR with a symmetric key
    void encrypt(std::vector<uint8_t>& data, uint8_t key);

    // Decrypts binary data in-place (XOR encryption is symmetrical)
    void decrypt(std::vector<uint8_t>& data, uint8_t key);

}
