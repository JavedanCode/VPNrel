#pragma once
#include <vector>
#include <cstdint>

namespace EncryptionModule {

    void encrypt(std::vector<uint8_t>& data, uint8_t key);

    void decrypt(std::vector<uint8_t>& data, uint8_t key);

}
