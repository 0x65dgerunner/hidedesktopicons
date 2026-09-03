#include "settings.hpp"

#include "i18n.hpp"
#include "json_utils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace hideicons {

std::string runtimeModeToString(RuntimeMode mode) {
    switch (mode) {
        case RuntimeMode::Tray:
            return "tray";
        case RuntimeMode::Invisible:
            return "invisible";
        default:
            return "gui";
    }
}

RuntimeMode parseRuntimeMode(const std::string& value) {
    if (value == "tray") {
        return RuntimeMode::Tray;
    }
    if (value == "invisible") {
        return RuntimeMode::Invisible;
    }
    return RuntimeMode::Gui;
}

std::filesystem::path settingsPath(const std::filesystem::path& exeDir) {
    return exeDir / "settings.json";
}

AppSettings loadSettings(const std::filesystem::path& exeDir) {
    AppSettings settings;
    settings.language = detectSystemLanguage();

    const auto path = settingsPath(exeDir);
    if (!std::filesystem::exists(path)) {
        saveSettings(exeDir, settings);
        return settings;
    }

    std::ifstream input(path);
    if (!input) {
        return settings;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string json = buffer.str();

    settings.hotkey.modifiers =
        static_cast<uint32_t>(extractJsonInt(json, "hotkeyModifiers", settings.hotkey.modifiers));
    settings.hotkey.virtualKey =
        extractJsonInt(json, "hotkeyVirtualKey", settings.hotkey.virtualKey);
    settings.minimizeToTrayOnClose =
        extractJsonBool(json, "minimizeToTrayOnClose", settings.minimizeToTrayOnClose);
    settings.startWithWindows = extractJsonBool(json, "startWithWindows", settings.startWithWindows);

    const auto language = extractJsonString(json, "language");
    if (!language.empty()) {
        settings.language = language;
    }

    const auto runtimeMode = extractJsonString(json, "runtimeMode");
    if (!runtimeMode.empty()) {
        settings.runtimeMode = parseRuntimeMode(runtimeMode);
    }

    return settings;
}

void saveSettings(const std::filesystem::path& exeDir, const AppSettings& settings) {
    const auto path = settingsPath(exeDir);
    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        throw std::runtime_error("Failed to save settings");
    }
    output << "{\n"
           << "  \"hotkeyModifiers\": " << settings.hotkey.modifiers << ",\n"
           << "  \"hotkeyVirtualKey\": " << settings.hotkey.virtualKey << ",\n"
           << "  \"minimizeToTrayOnClose\": "
           << (settings.minimizeToTrayOnClose ? "true" : "false") << ",\n"
           << "  \"startWithWindows\": " << (settings.startWithWindows ? "true" : "false") << ",\n"
           << "  \"language\": " << jsonString(settings.language) << ",\n"
           << "  \"runtimeMode\": " << jsonString(runtimeModeToString(settings.runtimeMode))
           << "\n"
           << "}\n";
}

}
