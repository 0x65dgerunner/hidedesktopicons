#include "startup.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace hideicons {

namespace {

constexpr wchar_t kRunKeyPath[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kAppValueName[] = L"HideDesktopIcons";
constexpr wchar_t kGuiExeName[] = L"HideDesktopIcons.exe";
constexpr wchar_t kLiteExeName[] = L"HideDesktopIconsLite.exe";

}

bool isStartWithWindowsEnabled() {
#ifdef _WIN32
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t buffer[MAX_PATH]{};
    DWORD size = sizeof(buffer);
    const LONG result =
        RegQueryValueExW(key, kAppValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer),
                         &size);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && buffer[0] != L'\0';
#else
    return false;
#endif
}

void setStartWithWindows(bool enabled, const std::filesystem::path& exePath,
                         const std::wstring& arguments) {
#ifdef _WIN32
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, nullptr, 0, KEY_SET_VALUE, nullptr,
                        &key, nullptr) != ERROR_SUCCESS) {
        return;
    }

    if (enabled) {
        std::wstring value = L"\"" + exePath.wstring() + L"\"";
        if (!arguments.empty()) {
            value += L" " + arguments;
        }
        RegSetValueExW(key, kAppValueName, 0, REG_SZ,
                       reinterpret_cast<const BYTE*>(value.c_str()),
                       static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(key, kAppValueName);
    }

    RegCloseKey(key);
#else
    (void)enabled;
    (void)exePath;
    (void)arguments;
#endif
}

std::filesystem::path guiExecutablePath(const std::filesystem::path& installDir) {
    return installDir / kGuiExeName;
}

std::filesystem::path liteExecutablePath(const std::filesystem::path& installDir) {
    return installDir / kLiteExeName;
}

std::wstring liteArgumentsForMode(RuntimeMode mode) {
    if (mode == RuntimeMode::Invisible) {
        return L"--invisible";
    }
    return L"--tray";
}

void applyStartWithWindowsSetting(const AppSettings& settings,
                                  const std::filesystem::path& installDir) {
    if (!settings.startWithWindows) {
        setStartWithWindows(false, guiExecutablePath(installDir));
        return;
    }

    if (settings.runtimeMode == RuntimeMode::Gui) {
        setStartWithWindows(true, guiExecutablePath(installDir));
        return;
    }

    setStartWithWindows(true, liteExecutablePath(installDir),
                        liteArgumentsForMode(settings.runtimeMode));
}

}
