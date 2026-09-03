#include "mainwindow.hpp"

#include "i18n.hpp"
#include "settings.hpp"

#include <FluentQt/FluentQt.h>

#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStyleHints>
#include <QSystemTrayIcon>

#include <filesystem>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace {

fs::path getExecutableDir() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
#else
    return fs::current_path();
#endif
}

fluent::FluentElement::Theme resolveSystemTheme() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (QGuiApplication::styleHints()) {
        const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
        if (scheme == Qt::ColorScheme::Dark) {
            return fluent::FluentElement::Dark;
        }
        if (scheme == Qt::ColorScheme::Light) {
            return fluent::FluentElement::Light;
        }
    }
#endif

#ifdef Q_OS_WIN
    const QSettings registry(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        QSettings::NativeFormat);
    if (registry.contains(QStringLiteral("AppsUseLightTheme"))) {
        return registry.value(QStringLiteral("AppsUseLightTheme"), 1).toInt() == 0
                   ? fluent::FluentElement::Dark
                   : fluent::FluentElement::Light;
    }
#endif

    if (qApp) {
        const QPalette palette = qApp->palette();
        if (palette.color(QPalette::Window).lightness() <
            palette.color(QPalette::WindowText).lightness()) {
            return fluent::FluentElement::Dark;
        }
    }

    return fluent::FluentElement::Light;
}

#ifdef _WIN32
bool acquireSingleInstance() {
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\HideDesktopIconsInstance");
    if (mutex == nullptr) {
        return true;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        return false;
    }
    return true;
}
#endif

}

int main(int argc, char* argv[]) {
    fluent::prepareHighDpiApplication();

    QApplication app(argc, argv);
    fluent::initializeResources();
    app.setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    fluent::FluentElement::setTheme(resolveSystemTheme());

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (QGuiApplication::styleHints()) {
        QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, &app,
                         [] { fluent::FluentElement::setThemeDeferred(resolveSystemTheme()); });
    }
#endif

    QApplication::setApplicationName(QStringLiteral("HideDesktopIcons"));
    QApplication::setOrganizationName(QStringLiteral("HideDesktopIcons"));
    QApplication::setQuitOnLastWindowClosed(false);

    const auto exeDir = getExecutableDir();
    hideicons::initLocales(exeDir);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QString::fromUtf8(hideicons::t("app.name")),
                              QString::fromUtf8(hideicons::t("tray.unavailable")));
        return 1;
    }

#ifdef _WIN32
    if (!acquireSingleInstance()) {
        QMessageBox::warning(nullptr, QString::fromUtf8(hideicons::t("app.name")),
                             QString::fromUtf8(hideicons::t("app.already_running")));
        return 1;
    }
#endif

    MainWindow window(exeDir);
    window.show();
    return app.exec();
}
