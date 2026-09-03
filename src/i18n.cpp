#include "i18n.hpp"

#include "json_utils.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace hideicons {

namespace {

namespace fs = std::filesystem;

struct LanguagePack {
    std::string code;
    std::string name;
    std::unordered_map<std::string, std::string> strings;
};

std::vector<LanguagePack> g_languages;
LanguagePack* g_active = nullptr;
LanguagePack* g_fallback = nullptr;
fs::path g_localesDir;
bool g_initialized = false;

std::string readTextFile(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

LanguagePack* findLanguage(const std::string& code) {
    for (auto& pack : g_languages) {
        if (pack.code == code) {
            return &pack;
        }
    }
    return nullptr;
}

bool loadLanguageFile(const fs::path& path) {
    const std::string content = readTextFile(path);
    if (content.empty()) {
        return false;
    }

    LanguagePack pack;
    pack.code = path.stem().string();
    pack.name = extractJsonString(content, "name");
    if (pack.name.empty()) {
        pack.name = pack.code;
    }
    pack.strings = parseJsonStringMap(content, "strings");
    if (pack.strings.empty()) {
        return false;
    }

    g_languages.push_back(std::move(pack));
    return true;
}

void chooseFallback() {
    g_fallback = findLanguage("en");
    if (g_fallback == nullptr) {
        g_fallback = findLanguage("pt");
    }
    if (g_fallback == nullptr && !g_languages.empty()) {
        g_fallback = &g_languages.front();
    }
    if (g_fallback == g_active) {
        for (auto& pack : g_languages) {
            if (&pack != g_active) {
                g_fallback = &pack;
                break;
            }
        }
    }
}

}

void initLocales(const fs::path& exeDir) {
    g_localesDir = exeDir / "locales";
    g_languages.clear();
    g_active = nullptr;
    g_fallback = nullptr;
    g_initialized = true;

    if (!fs::exists(g_localesDir)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(g_localesDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".json") {
            continue;
        }
        loadLanguageFile(entry.path());
    }

    std::sort(g_languages.begin(), g_languages.end(),
              [](const LanguagePack& a, const LanguagePack& b) { return a.code < b.code; });
}

std::string detectSystemLanguage() {
#ifdef _WIN32
    const LANGID lang = GetUserDefaultUILanguage();
    const WORD primary = PRIMARYLANGID(lang);
    const WORD sub = SUBLANGID(lang);

    switch (primary) {
    case LANG_ARABIC:
        return "ar";
    case LANG_CHINESE:
        if (sub == SUBLANG_CHINESE_TRADITIONAL || sub == SUBLANG_CHINESE_HONGKONG ||
            sub == SUBLANG_CHINESE_MACAU) {
            return "zh-TW";
        }
        return "zh";
    case LANG_CZECH:
        return "cs";
    case LANG_DANISH:
        return "da";
    case LANG_DUTCH:
        return "nl";
    case LANG_FINNISH:
        return "fi";
    case LANG_FRENCH:
        return "fr";
    case LANG_GERMAN:
        return "de";
    case LANG_GREEK:
        return "el";
    case LANG_HEBREW:
        return "he";
    case LANG_HINDI:
        return "hi";
    case LANG_HUNGARIAN:
        return "hu";
    case LANG_INDONESIAN:
        return "id";
    case LANG_ITALIAN:
        return "it";
    case LANG_JAPANESE:
        return "ja";
    case LANG_KOREAN:
        return "ko";
    case LANG_MALAY:
        return "ms";
    case LANG_NORWEGIAN:
        return "nb";
    case LANG_POLISH:
        return "pl";
    case LANG_PORTUGUESE:
        return "pt";
    case LANG_ROMANIAN:
        return "ro";
    case LANG_RUSSIAN:
        return "ru";
    case LANG_SPANISH:
        return "es";
    case LANG_SWEDISH:
        return "sv";
    case LANG_THAI:
        return "th";
    case LANG_TURKISH:
        return "tr";
    case LANG_UKRAINIAN:
        return "uk";
    case LANG_VIETNAMESE:
        return "vi";
    default:
        return "en";
    }
#else
    return "en";
#endif
}

void setLanguage(const std::string& code) {
    if (!g_initialized) {
        initLocales(fs::current_path());
    }

    LanguagePack* pack = findLanguage(code);
    if (pack == nullptr) {
        pack = findLanguage("en");
    }
    if (pack == nullptr && !g_languages.empty()) {
        pack = &g_languages.front();
    }

    g_active = pack;
    chooseFallback();
}

std::string languageCode() {
    return g_active != nullptr ? g_active->code : "en";
}

std::string languageName() {
    return g_active != nullptr ? g_active->name : "English";
}

std::vector<std::pair<std::string, std::string>> availableLanguages() {
    std::vector<std::pair<std::string, std::string>> result;
    result.reserve(g_languages.size());
    for (const auto& pack : g_languages) {
        result.emplace_back(pack.code, pack.name);
    }
    return result;
}

const char* t(const char* key) {
    if (g_active == nullptr) {
        setLanguage("en");
    }
    if (g_active != nullptr) {
        const auto it = g_active->strings.find(key);
        if (it != g_active->strings.end()) {
            return it->second.c_str();
        }
    }
    if (g_fallback != nullptr && g_fallback != g_active) {
        const auto it = g_fallback->strings.find(key);
        if (it != g_fallback->strings.end()) {
            return it->second.c_str();
        }
    }
    return key;
}

}
