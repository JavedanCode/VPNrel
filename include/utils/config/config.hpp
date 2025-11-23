#pragma once
#include <string>
#include <unordered_map>

/**
 * @class Config
 * @brief Loads and stores configuration values from a file.
 *
 * This class will handle reading key-value pairs from a configuration file,
 * so the VPN client and server can use dynamic settings (e.g., IP, port, etc.).
 *
 * Step Plan:
 *   Step 1 → Implement simple key=value file parser.
 *   Step 2 → Add default values for missing keys.
 *   Step 3 → Add validation for numeric values (like port).
 *   Step 4 → Add JSON or YAML support (optional advanced step).
 */
class Config {
public:
    // Loads configuration data from a file
    bool load(const std::string& filename);

    // Retrieves a value for the specified key
    std::string get(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> values; // Stores parsed key-value pairs
};
