#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace hideicons {

void initLocales(const std::filesystem::path& exeDir);
void setLanguage(const std::string& code);
std::string languageCode();
std::string languageName();
std::string detectSystemLanguage();
std::vector<std::pair<std::string, std::string>> availableLanguages();

const char* t(const char* key);

}
