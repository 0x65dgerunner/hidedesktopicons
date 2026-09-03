#pragma once

#include "hotkey.hpp"

#include <filesystem>

namespace hideicons {

enum class RuntimeMode {
    Gui,
    Tray,
    Invisible,
};

struct AppSettings {
    HotkeyBinding hotkey = HotkeyBinding::defaultBinding();
    bool minimizeToTrayOnClose = true;
    bool startWithWindows = false;
    std::string language;
    RuntimeMode runtimeMode = RuntimeMode::Gui;
};

std::string runtimeModeToString(RuntimeMode mode);
RuntimeMode parseRuntimeMode(const std::string& value);

std::filesystem::path settingsPath(const std::filesystem::path& exeDir);
AppSettings loadSettings(const std::filesystem::path& exeDir);
void saveSettings(const std::filesystem::path& exeDir, const AppSettings& settings);

}
