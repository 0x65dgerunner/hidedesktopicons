#include "mainwindow.hpp"

#include "hotkey.hpp"
#include "i18n.hpp"
#include "icon_helper.hpp"
#include "startup.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

#include "design/Spacing.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace {

constexpr int kWindowWidth = 420;
constexpr int kWindowHeight = 508;
constexpr int kActionButtonHeight = 34;
constexpr int kCaptionButtonWidth = 32;
constexpr int kCaptionIconSize = 12;
constexpr int kTitleBarHeight = 32;
constexpr int kVisibleCaptionButtonCount = 2;

QIcon captionStrokeIcon(const QString& resourcePath, const QColor& color) {
    QIcon icon;
    for (const int size : {kCaptionIconSize, kCaptionIconSize + 2}) {
        icon.addPixmap(AppIcons::pixmap(resourcePath, size, color, 1.0));
    }
    return icon;
}

fluent::basicinput::Button* findCaptionButton(QWidget* root, const char* objectName) {
    return root != nullptr ? root->findChild<fluent::basicinput::Button*>(objectName) : nullptr;
}

void applyCaptionButtonIcon(fluent::basicinput::Button* button, const QIcon& icon) {
    if (button == nullptr) {
        return;
    }
    button->setIconGlyph(QString());
    button->setIcon(icon);
    button->setFixedSize(kCaptionButtonWidth, kTitleBarHeight);
}

fluent::textfields::Label* makeSectionTitle(const QString& text, QWidget* parent) {
    auto* label = new fluent::textfields::Label(text, parent);
    label->setFluentTypography(Typography::FontRole::Subtitle);
    return label;
}

fluent::textfields::Label* makeFieldLabel(const QString& text, QWidget* parent) {
    auto* label = new fluent::textfields::Label(text, parent);
    label->setFluentTypography(Typography::FontRole::Body);
    return label;
}

fluent::basicinput::Button* makeButton(const QString& text, QWidget* parent,
                                         fluent::basicinput::Button::ButtonStyle style =
                                             fluent::basicinput::Button::Standard) {
    auto* button = new fluent::basicinput::Button(text, parent);
    button->setFluentStyle(style);
    button->setFluentSize(fluent::basicinput::Button::StandardSize);
    button->setFixedHeight(kActionButtonHeight);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return button;
}

void hideMaximizeCaptionButton(QWidget* root) {
    auto* maximizeButton = findCaptionButton(root, "fluentWindowMaximizeButton");
    if (maximizeButton == nullptr) {
        return;
    }

    maximizeButton->disconnect();
    maximizeButton->hide();
    maximizeButton->setEnabled(false);
    maximizeButton->setFixedSize(0, 0);
}

}

MainWindow::MainWindow(const std::filesystem::path& exeDir, QWidget* parent)
    : fluent::windowing::Window(parent), exeDir_(exeDir) {
    hideicons::initLocales(exeDir_);
    settings_ = hideicons::loadSettings(exeDir_);
    hideicons::setLanguage(settings_.language);
    desktopIcons_.initialize();

    setFixedSize(kWindowWidth, kWindowHeight);
    setFocusPolicy(Qt::StrongFocus);
    setBackdropEffect(fluent::windowing::BackdropEffect::Mica);

    auto* central = new QWidget(this);
    central->setAutoFillBackground(false);
    central->setAttribute(Qt::WA_TranslucentBackground);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(12);

    hotkeySectionLabel_ = makeSectionTitle(QString(), central);
    layout->addWidget(hotkeySectionLabel_);

    auto* hotkeyBlock = new QWidget(central);
    hotkeyBlock->setAutoFillBackground(false);
    hotkeyBlock->setAttribute(Qt::WA_TranslucentBackground);
    hotkeyBlock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    hotkeyBlock->setFixedHeight(::Spacing::ControlHeight::Standard + 8 + kActionButtonHeight);
    auto* hotkeyBlockLayout = new QVBoxLayout(hotkeyBlock);
    hotkeyBlockLayout->setContentsMargins(0, 0, 0, 0);
    hotkeyBlockLayout->setSpacing(8);

    hotkeyField_ = new fluent::textfields::LineEdit(hotkeyBlock);
    hotkeyField_->setReadOnly(true);
    hotkeyField_->setFrameVisible(true);
    hotkeyField_->setClearButtonEnabled(false);
    hotkeyField_->setAlignment(Qt::AlignCenter);
    hotkeyField_->setFontRole(Typography::FontRole::BodyStrong);
    hotkeyField_->setFixedHeight(::Spacing::ControlHeight::Standard);
    hotkeyField_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    hotkeyBlockLayout->addWidget(hotkeyField_);

    auto* hotkeyRow = new QHBoxLayout();
    hotkeyRow->setSpacing(8);
    hotkeyRow->setContentsMargins(0, 0, 0, 0);
    editHotkeyButton_ = makeButton(QString(), hotkeyBlock);
    resetHotkeyButton_ = makeButton(QString(), hotkeyBlock);
    connect(editHotkeyButton_, &fluent::basicinput::Button::clicked, this,
            &MainWindow::onEditHotkeyClicked);
    connect(resetHotkeyButton_, &fluent::basicinput::Button::clicked, this,
            &MainWindow::onResetHotkeyClicked);
    hotkeyRow->addWidget(editHotkeyButton_, 1);
    hotkeyRow->addWidget(resetHotkeyButton_, 1);
    hotkeyBlockLayout->addLayout(hotkeyRow);

    layout->addWidget(hotkeyBlock);

    layout->addWidget(new fluent::layout::Divider(Qt::Horizontal, central));

    behaviorSectionLabel_ = makeSectionTitle(QString(), central);
    layout->addWidget(behaviorSectionLabel_);

    minimizeToTrayCheck_ = new fluent::basicinput::CheckBox(central);
    connect(minimizeToTrayCheck_, &fluent::basicinput::CheckBox::toggled, this,
            &MainWindow::onMinimizeToTrayToggled);
    layout->addWidget(minimizeToTrayCheck_);

    startWithWindowsCheck_ = new fluent::basicinput::CheckBox(central);
    connect(startWithWindowsCheck_, &fluent::basicinput::CheckBox::toggled, this,
            &MainWindow::onStartWithWindowsToggled);
    layout->addWidget(startWithWindowsCheck_);

    auto* languageRow = new QHBoxLayout();
    languageRow->setSpacing(8);
    languageLabel_ = makeFieldLabel(QString(), central);
    languageCombo_ = new fluent::basicinput::ComboBox(central);
    const auto languages = hideicons::availableLanguages();
    int languageIndex = 0;
    for (int i = 0; i < static_cast<int>(languages.size()); ++i) {
        languageCombo_->addItem(QString::fromStdString(languages[static_cast<size_t>(i)].second),
                                QString::fromStdString(languages[static_cast<size_t>(i)].first));
        if (languages[static_cast<size_t>(i)].first == settings_.language) {
            languageIndex = i;
        }
    }
    languageCombo_->blockSignals(true);
    languageCombo_->setCurrentIndex(languageIndex);
    languageCombo_->blockSignals(false);
    connect(languageCombo_, QOverload<int>::of(&fluent::basicinput::ComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);
    languageRow->addWidget(languageLabel_);
    languageRow->addWidget(languageCombo_, 1);
    layout->addLayout(languageRow);

    auto* runtimeRow = new QHBoxLayout();
    runtimeRow->setSpacing(8);
    runtimeLabel_ = makeFieldLabel(QString(), central);
    runtimeCombo_ = new fluent::basicinput::ComboBox(central);
    runtimeCombo_->addItem(QString(), QStringLiteral("gui"));
    runtimeCombo_->addItem(QString(), QStringLiteral("tray"));
    runtimeCombo_->addItem(QString(), QStringLiteral("invisible"));
    connect(runtimeCombo_, QOverload<int>::of(&fluent::basicinput::ComboBox::currentIndexChanged),
            this, &MainWindow::onRuntimeModeChanged);
    runtimeRow->addWidget(runtimeLabel_);
    runtimeRow->addWidget(runtimeCombo_, 1);
    layout->addLayout(runtimeRow);

    layout->addWidget(new fluent::layout::Divider(Qt::Horizontal, central));

    toggleIconsButton_ = makeButton(QString(), central, fluent::basicinput::Button::Accent);
    connect(toggleIconsButton_, &fluent::basicinput::Button::clicked, this,
            &MainWindow::onToggleIconsClicked);
    layout->addWidget(toggleIconsButton_);

    pauseButton_ = makeButton(QString(), central);
    connect(pauseButton_, &fluent::basicinput::Button::clicked, this, &MainWindow::onPauseClicked);
    layout->addWidget(pauseButton_);

    setContentWidget(central);

    setupTitleBar();
    customizeCaptionButtons();

    hotkeyTimer_ = new QTimer(this);
    hotkeyTimer_->setInterval(80);
    connect(hotkeyTimer_, &QTimer::timeout, this, &MainWindow::onHotkeyPoll);
    hotkeyTimer_->start();

    setupTray();
    applySettings();
    refreshLocalizedUi();
    updateHotkeyLabel();
    updatePauseUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupTitleBar() {
    if (titleBar() == nullptr) {
        return;
    }

    titleBar()->setTitleBarHeight(kTitleBarHeight);

    auto* content = new QWidget(this);
    auto* layout = new QHBoxLayout(content);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(0);

    titleLabel_ = new fluent::textfields::Label(content);
    titleLabel_->setFluentTypography(Typography::FontRole::Body);
    layout->addWidget(titleLabel_, 0, Qt::AlignVCenter);
    layout->addStretch(1);

    titleBar()->setContentWidget(content);
    titleBar()->installEventFilter(this);
}

void MainWindow::customizeCaptionButtons() {
    hideMaximizeCaptionButton(this);

    if (auto* host = findChild<QWidget*>(QStringLiteral("fluentWindowCaptionButtonHost"))) {
        host->setFixedSize(kCaptionButtonWidth * kVisibleCaptionButtonCount, kTitleBarHeight);
    }

    if (titleBar() != nullptr) {
        titleBar()->setSystemReservedTrailingWidth(kCaptionButtonWidth * kVisibleCaptionButtonCount);
        titleBar()->refreshChromeExclusions();
    }

    updateCaptionButtonIcons();
}

void MainWindow::updateCaptionButtonIcons() {
    const QColor iconColor = themeColors().textSecondary;

    applyCaptionButtonIcon(findCaptionButton(this, "fluentWindowMinimizeButton"),
                           captionStrokeIcon(AppIcons::strokeMinimize(), iconColor));
    applyCaptionButtonIcon(findCaptionButton(this, "fluentWindowCloseButton"),
                           captionStrokeIcon(AppIcons::strokeClose(), iconColor));
}

void MainWindow::onThemeUpdated() {
    fluent::windowing::Window::onThemeUpdated();
    updateCaptionButtonIcons();
}

void MainWindow::showEvent(QShowEvent* event) {
    fluent::windowing::Window::showEvent(event);
    customizeCaptionButtons();
}

QString MainWindow::trKey(const char* key) const {
    return QString::fromUtf8(hideicons::t(key));
}

void MainWindow::refreshLocalizedUi() {
    const QString appName = trKey("app.name");
    setWindowTitle(appName);
    if (titleLabel_ != nullptr) {
        titleLabel_->setText(appName);
    }
    if (hotkeySectionLabel_ != nullptr) {
        hotkeySectionLabel_->setText(trKey("section.hotkey"));
    }
    if (behaviorSectionLabel_ != nullptr) {
        behaviorSectionLabel_->setText(trKey("section.behavior"));
    }
    if (languageLabel_ != nullptr) {
        languageLabel_->setText(trKey("lang.label"));
    }
    if (runtimeLabel_ != nullptr) {
        runtimeLabel_->setText(trKey("runtime.label"));
    }
    if (runtimeCombo_ != nullptr) {
        const QString guiLabel = trKey("runtime.gui");
        const QString trayLabel = trKey("runtime.tray");
        const QString invisibleLabel = trKey("runtime.invisible");
        for (int i = 0; i < runtimeCombo_->count(); ++i) {
            const QString code = runtimeCombo_->itemData(i).toString();
            if (code == QStringLiteral("gui")) {
                runtimeCombo_->setItemText(i, guiLabel);
            } else if (code == QStringLiteral("tray")) {
                runtimeCombo_->setItemText(i, trayLabel);
            } else if (code == QStringLiteral("invisible")) {
                runtimeCombo_->setItemText(i, invisibleLabel);
            }
        }
    }
    if (minimizeToTrayCheck_ != nullptr) {
        minimizeToTrayCheck_->setText(trKey("check.minimize_tray"));
    }
    if (startWithWindowsCheck_ != nullptr) {
        startWithWindowsCheck_->setText(trKey("check.start_windows"));
    }
    if (toggleIconsButton_ != nullptr) {
        toggleIconsButton_->setText(trKey("btn.toggle_icons"));
    }
    if (editHotkeyButton_ != nullptr) {
        editHotkeyButton_->setText(editingHotkey_ ? trKey("btn.save") : trKey("btn.edit"));
    }
    if (resetHotkeyButton_ != nullptr) {
        resetHotkeyButton_->setText(trKey("btn.default"));
    }
    if (trayIcon_ != nullptr) {
        trayIcon_->setToolTip(trKey("tray.tooltip"));
    }
    updatePauseUi();
}

void MainWindow::setupTray() {
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(AppIcons::icon(AppIcons::strokeSettings()));

    auto* menu = new fluent::menus_toolbars::FluentMenu(QString(), this);
    trayPauseAction_ = menu->addAction(QString());
    connect(trayPauseAction_, &QAction::triggered, this, &MainWindow::onTrayTogglePause);

    trayIcon_->setContextMenu(menu);
    connect(trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    trayIcon_->show();
}

void MainWindow::applySettings() {
    minimizeToTrayCheck_->blockSignals(true);
    startWithWindowsCheck_->blockSignals(true);
    minimizeToTrayCheck_->setChecked(settings_.minimizeToTrayOnClose);
    startWithWindowsCheck_->setChecked(settings_.startWithWindows);
    minimizeToTrayCheck_->blockSignals(false);
    startWithWindowsCheck_->blockSignals(false);

    hideicons::applyStartWithWindowsSetting(settings_, exeDir_);

    if (runtimeCombo_ != nullptr) {
        runtimeCombo_->blockSignals(true);
        const QString mode = QString::fromStdString(hideicons::runtimeModeToString(settings_.runtimeMode));
        for (int i = 0; i < runtimeCombo_->count(); ++i) {
            if (runtimeCombo_->itemData(i).toString() == mode) {
                runtimeCombo_->setCurrentIndex(i);
                break;
            }
        }
        runtimeCombo_->blockSignals(false);
    }
}

void MainWindow::saveCurrentSettings() {
    hideicons::saveSettings(exeDir_, settings_);
}

void MainWindow::updateHotkeyLabel() {
    if (hotkeyField_ == nullptr) {
        return;
    }

    hotkeyField_->setText(QString::fromStdString(hideicons::formatHotkey(settings_.hotkey)));
    if (editingHotkey_) {
        hotkeyField_->setFocus(Qt::OtherFocusReason);
    }
}

void MainWindow::updatePauseUi() {
    if (pauseButton_ != nullptr) {
        pauseButton_->setText(paused_ ? trKey("btn.active") : trKey("btn.pause"));
        pauseButton_->setFluentStyle(paused_ ? fluent::basicinput::Button::Accent
                                             : fluent::basicinput::Button::Standard);
    }
    if (trayPauseAction_ != nullptr) {
        trayPauseAction_->setText(paused_ ? trKey("tray.resume_hotkey") : trKey("tray.pause_hotkey"));
    }
}

void MainWindow::showFromTray() {
    show();
    raise();
    requestForegroundActivation();
}

void MainWindow::hideToTray() {
    hide();
    if (trayIcon_ != nullptr) {
        trayIcon_->showMessage(trKey("tray.tooltip"), trKey("tray.hidden_message"),
                               QSystemTrayIcon::Information, 2000);
    }
}

void MainWindow::startHotkeyCapture() {
    qApp->installEventFilter(this);
    grabKeyboard();
    setFocus(Qt::OtherFocusReason);
    updateHotkeyLabel();
}

void MainWindow::stopHotkeyCapture() {
    releaseKeyboard();
    qApp->removeEventFilter(this);
    updateHotkeyLabel();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (titleBar() != nullptr && watched == titleBar() &&
        event->type() == QEvent::MouseButtonDblClick) {
        return true;
    }

    if (!editingHotkey_) {
        return fluent::windowing::Window::eventFilter(watched, event);
    }

    if (event->type() == QEvent::KeyPress) {
        if (captureHotkeyFromEvent(static_cast<QKeyEvent*>(event))) {
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress) {
        if (captureHotkeyFromMouse(static_cast<QMouseEvent*>(event))) {
            return true;
        }
    }

    return fluent::windowing::Window::eventFilter(watched, event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (quitting_) {
        event->accept();
        return;
    }

    if (settings_.minimizeToTrayOnClose) {
        event->ignore();
        hideToTray();
        return;
    }

    quitting_ = true;
    QApplication::quit();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (editingHotkey_ && captureHotkeyFromEvent(event)) {
        return;
    }
    fluent::windowing::Window::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (editingHotkey_ && captureHotkeyFromMouse(event)) {
        return;
    }
    fluent::windowing::Window::mousePressEvent(event);
}

void MainWindow::onHotkeyPoll() {
    if (paused_ || editingHotkey_ || !desktopIcons_.isInitialized()) {
        hotkeyHandled_ = false;
        return;
    }

    if (hideicons::isHotkeyPressed(settings_.hotkey)) {
        if (!hotkeyHandled_) {
            desktopIcons_.toggle();
            hotkeyHandled_ = true;
        }
    } else {
        hotkeyHandled_ = false;
    }
}

void MainWindow::onEditHotkeyClicked() {
    editingHotkey_ = !editingHotkey_;
    if (editingHotkey_) {
        startHotkeyCapture();
    } else {
        stopHotkeyCapture();
        saveCurrentSettings();
    }
    refreshLocalizedUi();
    updateHotkeyLabel();
}

void MainWindow::onResetHotkeyClicked() {
    settings_.hotkey = hideicons::HotkeyBinding::defaultBinding();
    if (editingHotkey_) {
        stopHotkeyCapture();
        editingHotkey_ = false;
    }
    refreshLocalizedUi();
    updateHotkeyLabel();
    saveCurrentSettings();
}

void MainWindow::onToggleIconsClicked() {
    if (!desktopIcons_.isInitialized()) {
        return;
    }
    desktopIcons_.toggle();
}

void MainWindow::onPauseClicked() {
    paused_ = !paused_;
    updatePauseUi();
}

void MainWindow::onMinimizeToTrayToggled(bool checked) {
    settings_.minimizeToTrayOnClose = checked;
    saveCurrentSettings();
}

void MainWindow::onStartWithWindowsToggled(bool checked) {
    settings_.startWithWindows = checked;
    hideicons::applyStartWithWindowsSetting(settings_, exeDir_);
    saveCurrentSettings();
}

void MainWindow::onRuntimeModeChanged(int index) {
    if (index < 0 || runtimeCombo_ == nullptr) {
        return;
    }

    const QString code = runtimeCombo_->itemData(index).toString();
    if (code.isEmpty()) {
        return;
    }

    settings_.runtimeMode = hideicons::parseRuntimeMode(code.toUtf8().toStdString());
    if (settings_.startWithWindows) {
        hideicons::applyStartWithWindowsSetting(settings_, exeDir_);
    }
    saveCurrentSettings();
    refreshLocalizedUi();
}

void MainWindow::onLanguageChanged(int index) {
    if (index < 0 || languageCombo_ == nullptr) {
        return;
    }

    const QString code = languageCombo_->itemData(index).toString();
    if (code.isEmpty()) {
        return;
    }

    settings_.language = code.toUtf8().toStdString();
    hideicons::setLanguage(settings_.language);
    saveCurrentSettings();
    refreshLocalizedUi();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        showFromTray();
    }
}

void MainWindow::onTrayTogglePause() {
    onPauseClicked();
}

uint32_t MainWindow::qtModifiersToFlags(Qt::KeyboardModifiers modifiers) const {
    uint32_t flags = hideicons::kModifierNone;
    if (modifiers.testFlag(Qt::ControlModifier)) {
        flags |= hideicons::kModifierCtrl;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        flags |= hideicons::kModifierAlt;
    }
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        flags |= hideicons::kModifierShift;
    }
    return flags;
}

bool MainWindow::captureHotkeyFromMouse(QMouseEvent* event) {
#ifdef _WIN32
    int vk = 0;
    if (event->button() == Qt::BackButton) {
        vk = VK_XBUTTON1;
    } else if (event->button() == Qt::ForwardButton) {
        vk = VK_XBUTTON2;
    } else if (event->button() == Qt::MiddleButton) {
        vk = VK_MBUTTON;
    }
    if (vk == 0) {
        return false;
    }

    settings_.hotkey.modifiers = qtModifiersToFlags(event->modifiers());
    settings_.hotkey.virtualKey = vk;
    updateHotkeyLabel();
    event->accept();
    return true;
#else
    (void)event;
    return false;
#endif
}

bool MainWindow::captureHotkeyFromEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return true;
    }

    if (event->key() == Qt::Key_Escape) {
        editingHotkey_ = false;
        stopHotkeyCapture();
        refreshLocalizedUi();
        return true;
    }

#ifdef _WIN32
    UINT vk = event->nativeVirtualKey();
    if (vk == 0) {
        vk = MapVirtualKeyW(static_cast<UINT>(event->nativeScanCode()), MAPVK_VSC_TO_VK);
    }
    if (vk == 0 || hideicons::isModifierVirtualKey(static_cast<int>(vk))) {
        return true;
    }

    settings_.hotkey.modifiers = qtModifiersToFlags(event->modifiers());
    settings_.hotkey.virtualKey = static_cast<int>(vk);
    updateHotkeyLabel();
    return true;
#else
    (void)event;
    return false;
#endif
}
