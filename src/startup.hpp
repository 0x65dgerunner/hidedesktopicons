#pragma once

#include "settings.hpp"

#include <filesystem>
#include <string>

namespace hideicons {

bool isStartWithWindowsEnabled();
void setStartWithWindows(bool enabled, const std::filesystem::path& exePath,
                         const std::wstring& arguments = std::wstring{});

std::filesystem::path guiExecutablePath(const std::filesystem::path& installDir);
std::filesystem::path liteExecutablePath(const std::filesystem::path& installDir);
std::wstring liteArgumentsForMode(RuntimeMode mode);
void applyStartWithWindowsSetting(const AppSettings& settings,
                                  const std::filesystem::path& installDir);

}
