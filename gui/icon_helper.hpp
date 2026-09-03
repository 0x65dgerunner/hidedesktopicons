#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

class QAbstractButton;

namespace AppIcons {

QIcon icon(const QString& resourcePath, const QColor& color = QColor());
QIcon hoverIcon(const QString& strokePath, const QString& solidPath,
                const QColor& color = QColor());
void applyHoverIcon(QAbstractButton* button, const QString& strokePath, const QString& solidPath,
                    const QColor& color = QColor());
QPixmap pixmap(const QString& resourcePath, int size, const QColor& color = QColor(),
               qreal devicePixelRatio = 1.0);

#define APP_ICON_PATHS(name, file)                                      \
    inline QString stroke##name() {                                     \
        return QStringLiteral(":/icons/stroke/" file);                  \
    }                                                                   \
    inline QString solid##name() {                                      \
        return QStringLiteral(":/icons/solid/" file);                   \
    }

APP_ICON_PATHS(Settings, "settings.svg")
APP_ICON_PATHS(Close, "close.svg")
APP_ICON_PATHS(Minimize, "minimize.svg")
APP_ICON_PATHS(Maximize, "maximize.svg")
APP_ICON_PATHS(Restore, "restore.svg")
APP_ICON_PATHS(MouseClick, "mouse-click.svg")
APP_ICON_PATHS(MouseScroll, "mouse-scroll.svg")
APP_ICON_PATHS(Lock, "lock.svg")
APP_ICON_PATHS(Unlock, "unlock.svg")

#undef APP_ICON_PATHS

}
