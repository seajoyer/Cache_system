#include "cache/cache_item.hpp"

nlohmann::json CacheItem::to_json() const {
    return {
        {"id", id},
        {"faculty", faculty},
        {"course", course},
        {"title", title},
        {"description", description},
        {"votesCount", votesCount},
        {"telegramGroupLink", telegramGroupLink},
        {"userId", userId}
    };
}

CacheItem CacheItem::from_json(const nlohmann::json& j) {
    CacheItem item;
    item.id = j["id"].get<int>();
    item.faculty = j["faculty"].get<std::string>();
    item.course = j["course"].get<std::string>();
    item.title = j["title"].get<std::string>();
    item.description = j["description"].get<std::string>();
    item.votesCount = j["votesCount"].get<int>();
    item.telegramGroupLink = j["telegramGroupLink"].get<std::string>();
    item.userId = j["userId"].get<int>();
    return item;
}
