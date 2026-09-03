#include "desktop_icons.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace hideicons {

namespace {

constexpr int kShow = 5;
constexpr int kHide = 0;

void* findDesktopListView() {
#ifdef _WIN32
    HWND progman = FindWindowW(L"Progman", nullptr);
    HWND shellViewWin = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);

    if (shellViewWin == nullptr) {
        HWND desktopWnd = nullptr;
        do {
            desktopWnd = FindWindowExW(nullptr, desktopWnd, L"WorkerW", nullptr);
            shellViewWin = FindWindowExW(desktopWnd, nullptr, L"SHELLDLL_DefView", nullptr);
        } while (shellViewWin == nullptr && desktopWnd != nullptr);
    }

    return FindWindowExW(shellViewWin, nullptr, L"SysListView32", L"FolderView");
#else
    return nullptr;
#endif
}

}

DesktopIcons::DesktopIcons() = default;

bool DesktopIcons::initialize() {
    desktopListView_ = findDesktopListView();
    return desktopListView_ != nullptr;
}

bool DesktopIcons::isInitialized() const {
    return desktopListView_ != nullptr;
}

bool DesktopIcons::iconsVisible() const {
    return iconsVisible_;
}

void DesktopIcons::setIconsVisible(bool visible) {
    if (desktopListView_ == nullptr) {
        return;
    }
#ifdef _WIN32
    ShowWindow(static_cast<HWND>(desktopListView_), visible ? kShow : kHide);
    iconsVisible_ = visible;
#else
    (void)visible;
#endif
}

void DesktopIcons::toggle() {
    setIconsVisible(!iconsVisible_);
}

}
