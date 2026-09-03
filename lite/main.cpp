#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

#include "desktop_icons.hpp"
#include "hotkey.hpp"
#include "settings.hpp"
#include "startup.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace {

constexpr wchar_t kWindowClass[] = L"HideDesktopIconsLiteHost";
constexpr wchar_t kMutexName[] = L"Local\\HideDesktopIconsInstance";
constexpr int kHotkeyId = 1;
constexpr UINT_PTR kPollTimerId = 2;
constexpr UINT kPollIntervalMs = 80;
constexpr UINT kTrayIconId = 1;
constexpr UINT WM_TRAYICON = WM_APP + 1;
constexpr int kCmdPause = 1001;
constexpr int kCmdExit = 1002;

enum class LitePresentation {
    Tray,
    Invisible,
};

struct AppState {
    HWND hwnd = nullptr;
    HINSTANCE instance = nullptr;
    fs::path exeDir;
    hideicons::AppSettings settings;
    hideicons::DesktopIcons desktopIcons;
    LitePresentation presentation = LitePresentation::Tray;
    bool paused = false;
    bool trayRegistered = false;
    bool hotkeyRegistered = false;
    bool hotkeyPolling = false;
    bool hotkeyHandled = false;
    NOTIFYICONDATAW trayData{};
};

AppState g_state;

fs::path getExecutablePath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return fs::path(buffer);
}

LitePresentation resolvePresentation(int argc, wchar_t* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (_wcsicmp(argv[i], L"--invisible") == 0) {
            return LitePresentation::Invisible;
        }
        if (_wcsicmp(argv[i], L"--tray") == 0) {
            return LitePresentation::Tray;
        }
    }

    const fs::path exeDir = getExecutablePath().parent_path();
    const hideicons::AppSettings settings = hideicons::loadSettings(exeDir);
    if (settings.runtimeMode == hideicons::RuntimeMode::Invisible) {
        return LitePresentation::Invisible;
    }
    return LitePresentation::Tray;
}

UINT hotkeyModifiers(const hideicons::HotkeyBinding& binding) {
    UINT mods = MOD_NOREPEAT;
    if ((binding.modifiers & hideicons::kModifierAlt) != 0) {
        mods |= MOD_ALT;
    }
    if ((binding.modifiers & hideicons::kModifierCtrl) != 0) {
        mods |= MOD_CONTROL;
    }
    if ((binding.modifiers & hideicons::kModifierShift) != 0) {
        mods |= MOD_SHIFT;
    }
    return mods;
}

bool registerGlobalHotkey(HWND hwnd) {
    UnregisterHotKey(hwnd, kHotkeyId);
    return RegisterHotKey(hwnd, kHotkeyId, hotkeyModifiers(g_state.settings.hotkey),
                          static_cast<UINT>(g_state.settings.hotkey.virtualKey)) != FALSE;
}

void unregisterGlobalHotkey(HWND hwnd) {
    UnregisterHotKey(hwnd, kHotkeyId);
}

void updateTrayMenu() {
    HMENU menu = CreatePopupMenu();
    if (menu == nullptr) {
        return;
    }

    const wchar_t* pauseLabel = g_state.paused ? L"Resume shortcut" : L"Pause shortcut";
    InsertMenuW(menu, 0, MF_BYPOSITION | MF_STRING, kCmdPause, pauseLabel);
    InsertMenuW(menu, 1, MF_BYPOSITION | MF_STRING, kCmdExit, L"Exit");

    POINT cursor{};
    GetCursorPos(&cursor);
    SetForegroundWindow(g_state.hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, g_state.hwnd, nullptr);
    PostMessageW(g_state.hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

bool registerTrayIcon() {
    g_state.trayData = {};
    g_state.trayData.cbSize = sizeof(NOTIFYICONDATAW);
    g_state.trayData.hWnd = g_state.hwnd;
    g_state.trayData.uID = kTrayIconId;
    g_state.trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_state.trayData.uCallbackMessage = WM_TRAYICON;
    g_state.trayData.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy_s(g_state.trayData.szTip, L"Hide Icons");

    return Shell_NotifyIconW(NIM_ADD, &g_state.trayData) != FALSE;
}

void unregisterTrayIcon() {
    if (!g_state.trayRegistered) {
        return;
    }
    Shell_NotifyIconW(NIM_DELETE, &g_state.trayData);
    g_state.trayRegistered = false;
}

void onHotkeyTriggered() {
    if (g_state.paused || !g_state.desktopIcons.isInitialized()) {
        return;
    }
    g_state.desktopIcons.toggle();
}

void onHotkeyPoll() {
    if (g_state.paused || !g_state.desktopIcons.isInitialized()) {
        g_state.hotkeyHandled = false;
        return;
    }

    if (hideicons::isHotkeyPressed(g_state.settings.hotkey)) {
        if (!g_state.hotkeyHandled) {
            onHotkeyTriggered();
            g_state.hotkeyHandled = true;
        }
    } else {
        g_state.hotkeyHandled = false;
    }
}

void startHotkeyPolling(HWND hwnd) {
    if (g_state.hotkeyPolling) {
        return;
    }
    SetTimer(hwnd, kPollTimerId, kPollIntervalMs, nullptr);
    g_state.hotkeyPolling = true;
}

void stopHotkeyPolling(HWND hwnd) {
    if (!g_state.hotkeyPolling) {
        return;
    }
    KillTimer(hwnd, kPollTimerId);
    g_state.hotkeyPolling = false;
    g_state.hotkeyHandled = false;
}

void setupHotkeyInput(HWND hwnd) {
    g_state.hotkeyRegistered = false;
    stopHotkeyPolling(hwnd);
    unregisterGlobalHotkey(hwnd);

    if (hideicons::canUseRegisterHotKey(g_state.settings.hotkey)) {
        g_state.hotkeyRegistered = registerGlobalHotkey(hwnd);
    }

    if (!g_state.hotkeyRegistered) {
        startHotkeyPolling(hwnd);
    }
}

void onTrayCommand(int command) {
    switch (command) {
        case kCmdPause:
            g_state.paused = !g_state.paused;
            break;
        case kCmdExit:
            PostMessageW(g_state.hwnd, WM_CLOSE, 0, 0);
            break;
        default:
            break;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_HOTKEY:
            if (wParam == kHotkeyId) {
                onHotkeyTriggered();
            }
            return 0;

        case WM_TIMER:
            if (wParam == kPollTimerId) {
                onHotkeyPoll();
            }
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
                updateTrayMenu();
            } else if (lParam == WM_LBUTTONDBLCLK) {
                g_state.paused = !g_state.paused;
            }
            return 0;

        case WM_COMMAND:
            onTrayCommand(static_cast<int>(LOWORD(wParam)));
            return 0;

        case WM_DESTROY:
            stopHotkeyPolling(hwnd);
            unregisterGlobalHotkey(hwnd);
            unregisterTrayIcon();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

bool acquireSingleInstance() {
    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    if (mutex == nullptr) {
        return true;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        return false;
    }
    return true;
}

}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    if (!acquireSingleInstance()) {
        return 0;
    }

    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    g_state.presentation = resolvePresentation(argc, argv);
    if (argv != nullptr) {
        LocalFree(argv);
    }

    g_state.instance = instance;
    g_state.exeDir = getExecutablePath().parent_path();
    g_state.settings = hideicons::loadSettings(g_state.exeDir);
    hideicons::applyStartWithWindowsSetting(g_state.settings, g_state.exeDir);

    if (!g_state.desktopIcons.initialize()) {
        return 1;
    }

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kWindowClass;
    RegisterClassExW(&windowClass);

    g_state.hwnd = CreateWindowExW(0, kWindowClass, L"Hide Icons Lite", WS_OVERLAPPED, 0, 0, 0, 0,
                                   nullptr, nullptr, instance, nullptr);
    if (g_state.hwnd == nullptr) {
        return 1;
    }

    setupHotkeyInput(g_state.hwnd);

    if (!g_state.hotkeyRegistered && !g_state.hotkeyPolling &&
        g_state.presentation == LitePresentation::Tray) {
        MessageBoxW(g_state.hwnd, L"Could not register the global shortcut. Another app may "
                                  L"already be using it.",
                    L"Hide Icons Lite", MB_ICONWARNING | MB_OK);
    }

    if (g_state.presentation == LitePresentation::Tray) {
        g_state.trayRegistered = registerTrayIcon();
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

#else

int main() {
    return 1;
}

#endif
