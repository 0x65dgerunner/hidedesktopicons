#pragma once

#include <cstdint>
#include <string>

namespace hideicons {

enum HotkeyModifier : uint32_t {
    kModifierNone = 0,
    kModifierCtrl = 1 << 0,
    kModifierAlt = 1 << 1,
    kModifierShift = 1 << 2,
};

struct HotkeyBinding {
    uint32_t modifiers = kModifierNone;
    int virtualKey = 0x05; // VK_XBUTTON1

    static HotkeyBinding defaultBinding();
};

std::string formatHotkey(const HotkeyBinding& binding);
bool isHotkeyPressed(const HotkeyBinding& binding);
bool isModifierVirtualKey(int virtualKey);
bool isMouseButtonVirtualKey(int virtualKey);
bool canUseRegisterHotKey(const HotkeyBinding& binding);
std::string virtualKeyDisplayName(int virtualKey);

}
