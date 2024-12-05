#pragma once

#include <string>
#include <nlohmann/json.hpp>

class CacheItem {
public:
    int id;
    std::string faculty;
    std::string course;
    std::string title;
    std::string description;
    int votesCount;
    std::string telegramGroupLink;
    int userId;

    [[nodiscard]] nlohmann::json to_json() const;
    static CacheItem from_json(const nlohmann::json& j);
};
