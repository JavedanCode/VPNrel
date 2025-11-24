#include "core/encryption/encryption.hpp"
#include <string>

namespace EncryptionModule {

    void encrypt(std::vector<uint8_t>& data, uint8_t key) {
        for (auto& b : data)
            b ^= key;
    }

    void decrypt(std::vector<uint8_t>& data, uint8_t key) {
        for (auto& b : data)
            b ^= key;
    }

}
