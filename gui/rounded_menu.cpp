#include "rounded_menu.hpp"

#include "app_fonts.hpp"

#include <QColor>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QStyleFactory>

namespace {

constexpr int kMenuRadius = 8;

}

RoundedMenu::RoundedMenu(QWidget* parent) : QMenu(parent) {
    setFont(AppFonts::regular(10.0));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
}

void RoundedMenu::applyTrayStyle() {
    setObjectName(QStringLiteral("TrayMenu"));
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(QStringLiteral(
        "QMenu#TrayMenu {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 4px;"
        "}"
        "QMenu#TrayMenu::item {"
        "  background-color: transparent;"
        "  color: #e8e8ea;"
        "  border-radius: 5px;"
        "  padding: 5px 10px;"
        "  margin: 1px 0;"
        "}"
        "QMenu#TrayMenu::item:selected {"
        "  background-color: #1a1a1e;"
        "}"
        "QMenu#TrayMenu::separator {"
        "  height: 1px;"
        "  background: #2c2c30;"
        "  margin: 3px 6px;"
        "}"));
}

bool RoundedMenu::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::Enter:
        case QEvent::MouseMove:
        case QEvent::HoverEnter:
        case QEvent::HoverMove:
            setCursor(Qt::PointingHandCursor);
            break;
        default:
            break;
    }
    return QMenu::event(event);
}

void RoundedMenu::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath background;
    background.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), kMenuRadius, kMenuRadius);

    painter.fillPath(background, QColor(QStringLiteral("#0a0a0a")));
    painter.setPen(QPen(QColor(QStringLiteral("#2c2c30")), 1.0));
    painter.drawPath(background);

    painter.setClipPath(background);
    QMenu::paintEvent(event);
}
