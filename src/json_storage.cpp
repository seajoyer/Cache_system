#include "json_storage.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

JSONStorage::JSONStorage(const std::string& filename) : filename_(filename) {}

void JSONStorage::save(const std::string& key, const std::string& value) {
    nlohmann::json data;

    // Load existing data
    std::ifstream input_file(filename_);
    if (input_file.is_open()) {
        input_file >> data;
        input_file.close();
    }

    // Update or add new key-value pair
    data[key] = value;

    // Save updated data
    std::ofstream output_file(filename_);
    output_file << data.dump(4);
    output_file.close();
}

std::string JSONStorage::load(const std::string& key) {
    nlohmann::json data;

    std::ifstream input_file(filename_);
    if (input_file.is_open()) {
        input_file >> data;
        input_file.close();
    }

    if (data.contains(key)) {
        return data[key];
    }

    throw std::out_of_range("Key not found in storage");
}

void JSONStorage::remove(const std::string& key) {
    nlohmann::json data;

    std::ifstream input_file(filename_);
    if (input_file.is_open()) {
        input_file >> data;
        input_file.close();
    }

    data.erase(key);

    std::ofstream output_file(filename_);
    output_file << data.dump(4);
    output_file.close();
}

std::unordered_map<std::string, std::string> JSONStorage::load_all() {
    nlohmann::json data;
    std::unordered_map<std::string, std::string> result;

    std::ifstream input_file(filename_);
    if (input_file.is_open()) {
        input_file >> data;
        input_file.close();
    }

    for (auto it = data.begin(); it != data.end(); ++it) {
        result[it.key()] = it.value();
    }

    return result;
}
