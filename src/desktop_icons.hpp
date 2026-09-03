#pragma once

namespace hideicons {

class DesktopIcons {
public:
    DesktopIcons();

    bool initialize();
    bool isInitialized() const;
    bool iconsVisible() const;
    void setIconsVisible(bool visible);
    void toggle();

private:
    void* desktopListView_ = nullptr;
    bool iconsVisible_ = true;
};

}
