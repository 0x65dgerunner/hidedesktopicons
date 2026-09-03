#include "json_utils.hpp"

#include <cstdio>
#include <string>
#include <unordered_map>

namespace hideicons {

namespace {

std::string escapeJson(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
                break;
        }
    }
    return out;
}

std::string unescapeJsonString(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c != '\\') {
            out += c;
            continue;
        }
        if (i + 1 >= value.size()) {
            break;
        }
        const char next = value[++i];
        switch (next) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case 'u':
                if (i + 4 < value.size()) {
                    unsigned int code = 0;
                    for (int digit = 0; digit < 4; ++digit) {
                        const char hex = value[i + 1 + digit];
                        code <<= 4;
                        if (hex >= '0' && hex <= '9') {
                            code |= static_cast<unsigned int>(hex - '0');
                        } else if (hex >= 'a' && hex <= 'f') {
                            code |= static_cast<unsigned int>(hex - 'a' + 10);
                        } else if (hex >= 'A' && hex <= 'F') {
                            code |= static_cast<unsigned int>(hex - 'A' + 10);
                        }
                    }
                    i += 4;
                    if (code <= 0x7F) {
                        out += static_cast<char>(code);
                    }
                }
                break;
            default:
                out += next;
                break;
        }
    }
    return out;
}

bool readJsonStringToken(const std::string& json, size_t& pos, std::string& out) {
    while (pos < json.size() && json[pos] != '"') {
        ++pos;
    }
    if (pos >= json.size()) {
        return false;
    }
    ++pos;

    std::string raw;
    bool escape = false;
    for (; pos < json.size(); ++pos) {
        const char c = json[pos];
        if (escape) {
            raw += '\\';
            raw += c;
            escape = false;
            continue;
        }
        if (c == '\\') {
            escape = true;
            continue;
        }
        if (c == '"') {
            ++pos;
            out = unescapeJsonString(raw);
            return true;
        }
        raw += c;
    }
    return false;
}

size_t findBalancedObjectEnd(const std::string& json, size_t openBracePos) {
    if (openBracePos >= json.size() || json[openBracePos] != '{') {
        return std::string::npos;
    }

    int depth = 0;
    for (size_t i = openBracePos; i < json.size(); ++i) {
        if (json[i] == '{') {
            ++depth;
        } else if (json[i] == '}') {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

}

std::unordered_map<std::string, std::string> parseJsonStringMap(const std::string& json,
                                                                 const std::string& objectKey) {
    std::string scope = json;
    if (!objectKey.empty()) {
        const std::string needle = "\"" + objectKey + "\"";
        const auto keyPos = json.find(needle);
        if (keyPos == std::string::npos) {
            return {};
        }
        const auto colonPos = json.find(':', keyPos + needle.size());
        if (colonPos == std::string::npos) {
            return {};
        }
        const auto openBracePos = json.find('{', colonPos);
        if (openBracePos == std::string::npos) {
            return {};
        }
        const auto closeBracePos = findBalancedObjectEnd(json, openBracePos);
        if (closeBracePos == std::string::npos) {
            return {};
        }
        scope = json.substr(openBracePos, closeBracePos - openBracePos + 1);
    }

    std::unordered_map<std::string, std::string> result;
    size_t pos = 0;
    while (pos < scope.size()) {
        std::string key;
        if (!readJsonStringToken(scope, pos, key)) {
            break;
        }

        while (pos < scope.size() && scope[pos] != ':') {
            ++pos;
        }
        if (pos >= scope.size()) {
            break;
        }
        ++pos;

        std::string value;
        if (!readJsonStringToken(scope, pos, value)) {
            break;
        }

        result.emplace(std::move(key), std::move(value));
    }

    return result;
}

std::string jsonString(const std::string& value) {
    return "\"" + escapeJson(value) + "\"";
}

std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& fields) {
    std::string out = "{";
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) out += ",";
        out += jsonString(fields[i].first) + ":" + fields[i].second;
    }
    out += "}";
    return out;
}

std::string jsonArray(const std::vector<std::string>& items) {
    std::string out = "[";
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) out += ",";
        out += items[i];
    }
    out += "]";
    return out;
}

std::string extractJsonString(const std::string& json, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";

    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '"') return "";

    size_t tokenPos = pos;
    std::string value;
    if (!readJsonStringToken(json, tokenPos, value)) {
        return "";
    }
    return value;
}

int extractJsonInt(const std::string& json, const std::string& key, int fallback) {
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) {
        return fallback;
    }
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return fallback;
    }
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        ++pos;
    }
    try {
        return std::stoi(json.substr(pos));
    } catch (...) {
        return fallback;
    }
}

bool extractJsonBool(const std::string& json, const std::string& key, bool fallback) {
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) {
        return fallback;
    }
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return fallback;
    }
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        ++pos;
    }
    if (json.compare(pos, 4, "true") == 0) {
        return true;
    }
    if (json.compare(pos, 5, "false") == 0) {
        return false;
    }
    return fallback;
}

}
