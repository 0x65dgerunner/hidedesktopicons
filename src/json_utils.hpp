#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace hideicons {

std::string jsonString(const std::string& value);
std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& fields);
std::string jsonArray(const std::vector<std::string>& items);

std::string extractJsonString(const std::string& json, const std::string& key);
int extractJsonInt(const std::string& json, const std::string& key, int fallback);
bool extractJsonBool(const std::string& json, const std::string& key, bool fallback);
std::unordered_map<std::string, std::string> parseJsonStringMap(const std::string& json,
                                                                 const std::string& objectKey = "strings");

}
