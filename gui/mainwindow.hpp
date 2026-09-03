#pragma once

#include "desktop_icons.hpp"
#include "settings.hpp"

#include <FluentQt/FluentQt.h>

#include <QSystemTrayIcon>

#include <filesystem>

class QTimer;
class QAction;
class QCloseEvent;
class QKeyEvent;
class QMouseEvent;
class QShowEvent;

class MainWindow : public fluent::windowing::Window {
    Q_OBJECT

public:
    explicit MainWindow(const std::filesystem::path& exeDir, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void onThemeUpdated() override;

private slots:
    void onHotkeyPoll();
    void onEditHotkeyClicked();
    void onResetHotkeyClicked();
    void onToggleIconsClicked();
    void onPauseClicked();
    void onMinimizeToTrayToggled(bool checked);
    void onStartWithWindowsToggled(bool checked);
    void onRuntimeModeChanged(int index);
    void onLanguageChanged(int index);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayTogglePause();

private:
    void setupTitleBar();
    void customizeCaptionButtons();
    void updateCaptionButtonIcons();
    void setupTray();
    void applySettings();
    void saveCurrentSettings();
    void updateHotkeyLabel();
    void updatePauseUi();
    void refreshLocalizedUi();
    void showFromTray();
    void hideToTray();
    void startHotkeyCapture();
    void stopHotkeyCapture();
    bool captureHotkeyFromEvent(QKeyEvent* event);
    bool captureHotkeyFromMouse(QMouseEvent* event);
    uint32_t qtModifiersToFlags(Qt::KeyboardModifiers modifiers) const;
    QString trKey(const char* key) const;

    std::filesystem::path exeDir_;
    hideicons::AppSettings settings_;
    hideicons::DesktopIcons desktopIcons_;

    fluent::textfields::Label* titleLabel_ = nullptr;
    fluent::textfields::Label* hotkeySectionLabel_ = nullptr;
    fluent::textfields::Label* behaviorSectionLabel_ = nullptr;
    fluent::textfields::Label* languageLabel_ = nullptr;
    fluent::textfields::Label* runtimeLabel_ = nullptr;
    fluent::textfields::LineEdit* hotkeyField_ = nullptr;
    fluent::basicinput::Button* editHotkeyButton_ = nullptr;
    fluent::basicinput::Button* resetHotkeyButton_ = nullptr;
    fluent::basicinput::Button* toggleIconsButton_ = nullptr;
    fluent::basicinput::Button* pauseButton_ = nullptr;
    fluent::basicinput::CheckBox* minimizeToTrayCheck_ = nullptr;
    fluent::basicinput::CheckBox* startWithWindowsCheck_ = nullptr;
    fluent::basicinput::ComboBox* languageCombo_ = nullptr;
    fluent::basicinput::ComboBox* runtimeCombo_ = nullptr;

    QSystemTrayIcon* trayIcon_ = nullptr;
    QAction* trayPauseAction_ = nullptr;
    QTimer* hotkeyTimer_ = nullptr;

    bool editingHotkey_ = false;
    bool hotkeyHandled_ = false;
    bool paused_ = false;
    bool quitting_ = false;
};
