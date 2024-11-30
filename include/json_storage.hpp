#pragma once

#include <string>
#include <unordered_map>

class JSONStorage {
public:
    explicit JSONStorage(const std::string& storage_path);

    void save(const std::unordered_map<std::string, std::string>& data);
    std::unordered_map<std::string, std::string> load();

    size_t get_disk_usage() const;

private:
    std::string storage_path_;
};
