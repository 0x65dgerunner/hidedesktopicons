#include "hotkey.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <sstream>
#include <string>

namespace hideicons {

HotkeyBinding HotkeyBinding::defaultBinding() {
    return {};
}

namespace {

#ifdef _WIN32
std::string wideToUtf8(const wchar_t* text) {
    if (text == nullptr || text[0] == L'\0') {
        return {};
    }
    const int size =
        WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string out(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.empty() ? nullptr : &out[0], size, nullptr,
                        nullptr);
    return out;
}

bool isExtendedVirtualKey(int vk) {
    switch (vk) {
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_NUMLOCK:
        case VK_DIVIDE:
        case VK_RCONTROL:
        case VK_RMENU:
        case VK_RWIN:
        case VK_APPS:
            return true;
        default:
            return false;
    }
}
#endif

std::string virtualKeyNameFallback(int vk) {
    switch (vk) {
        case VK_XBUTTON1: return "XButton1";
        case VK_XBUTTON2: return "XButton2";
        case VK_MBUTTON: return "Middle";
        case VK_SPACE: return "Space";
        case VK_RETURN: return "Enter";
        case VK_TAB: return "Tab";
        case VK_ESCAPE: return "Esc";
        case VK_BACK: return "Backspace";
        case VK_DELETE: return "Delete";
        case VK_INSERT: return "Insert";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "PageUp";
        case VK_NEXT: return "PageDown";
        case VK_LEFT: return "Left";
        case VK_RIGHT: return "Right";
        case VK_UP: return "Up";
        case VK_DOWN: return "Down";
        case VK_SNAPSHOT: return "Print Screen";
        case VK_PAUSE: return "Pause";
        case VK_SCROLL: return "Scroll Lock";
        case VK_NUMLOCK: return "Num Lock";
        case VK_CAPITAL: return "Caps Lock";
        default:
            break;
    }

    if (vk >= VK_F1 && vk <= VK_F24) {
        return "F" + std::to_string(vk - VK_F1 + 1);
    }
    if (vk >= 'A' && vk <= 'Z') {
        return std::string(1, static_cast<char>(vk));
    }
    if (vk >= '0' && vk <= '9') {
        return std::string(1, static_cast<char>(vk));
    }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        return "Num " + std::string(1, static_cast<char>('0' + (vk - VK_NUMPAD0)));
    }

    return "VK " + std::to_string(vk);
}

bool modifierSatisfied(uint32_t required, uint32_t flag, int vk) {
    if ((required & flag) != 0) {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
    }
    return true;
}

}

bool isModifierVirtualKey(int virtualKey) {
#ifdef _WIN32
    switch (virtualKey) {
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
        case VK_LWIN:
        case VK_RWIN:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LMENU:
        case VK_RMENU:
            return true;
        default:
            return false;
    }
#else
    (void)virtualKey;
    return false;
#endif
}

bool isMouseButtonVirtualKey(int virtualKey) {
#ifdef _WIN32
    switch (virtualKey) {
        case VK_LBUTTON:
        case VK_RBUTTON:
        case VK_MBUTTON:
        case VK_XBUTTON1:
        case VK_XBUTTON2:
            return true;
        default:
            return false;
    }
#else
    (void)virtualKey;
    return false;
#endif
}

bool canUseRegisterHotKey(const HotkeyBinding& binding) {
#ifdef _WIN32
    return binding.virtualKey > 0 && !isMouseButtonVirtualKey(binding.virtualKey);
#else
    (void)binding;
    return false;
#endif
}

std::string virtualKeyDisplayName(int virtualKey) {
#ifdef _WIN32
    if (virtualKey <= 0) {
        return {};
    }

    if (virtualKey == VK_XBUTTON1) {
        return "XButton1";
    }
    if (virtualKey == VK_XBUTTON2) {
        return "XButton2";
    }

    const UINT scanCode = MapVirtualKeyW(static_cast<UINT>(virtualKey), MAPVK_VK_TO_VSC);
    if (scanCode != 0) {
        LONG lParam = static_cast<LONG>(scanCode) << 16;
        if (isExtendedVirtualKey(virtualKey)) {
            lParam |= 1 << 24;
        }

        wchar_t buffer[128]{};
        if (GetKeyNameTextW(lParam, buffer, static_cast<int>(sizeof(buffer) / sizeof(buffer[0]))) >
            0) {
            const std::string name = wideToUtf8(buffer);
            if (!name.empty()) {
                return name;
            }
        }
    }

    return virtualKeyNameFallback(virtualKey);
#else
    return virtualKeyNameFallback(virtualKey);
#endif
}

std::string formatHotkey(const HotkeyBinding& binding) {
    std::ostringstream out;
    bool first = true;

    auto appendPart = [&](const char* label) {
        if (!first) {
            out << " + ";
        }
        out << label;
        first = false;
    };

    if ((binding.modifiers & kModifierCtrl) != 0) {
        appendPart("Ctrl");
    }
    if ((binding.modifiers & kModifierAlt) != 0) {
        appendPart("Alt");
    }
    if ((binding.modifiers & kModifierShift) != 0) {
        appendPart("Shift");
    }

    const std::string keyName = virtualKeyDisplayName(binding.virtualKey);
    if (first) {
        return keyName;
    }
    out << " + " << keyName;
    return out.str();
}

bool isHotkeyPressed(const HotkeyBinding& binding) {
#ifdef _WIN32
    if (!modifierSatisfied(binding.modifiers, kModifierCtrl, VK_CONTROL)) {
        return false;
    }
    if (!modifierSatisfied(binding.modifiers, kModifierAlt, VK_MENU)) {
        return false;
    }
    if (!modifierSatisfied(binding.modifiers, kModifierShift, VK_SHIFT)) {
        return false;
    }
    return (GetAsyncKeyState(binding.virtualKey) & 0x8000) != 0;
#else
    (void)binding;
    return false;
#endif
}

}
