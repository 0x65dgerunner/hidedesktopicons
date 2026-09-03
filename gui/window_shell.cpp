#include "window_shell.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

RoundedWindowShell::RoundedWindowShell(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("RoundedWindowShell"));
    setAttribute(Qt::WA_StyledBackground, true);
}

void RoundedWindowShell::setCornerRadius(int radius) {
    if (cornerRadius_ == radius) {
        return;
    }
    cornerRadius_ = radius;
    update();
}

void RoundedWindowShell::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF bounds = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath background;
    if (cornerRadius_ > 0) {
        background.addRoundedRect(bounds, cornerRadius_, cornerRadius_);
    } else {
        background.addRect(bounds);
    }

    painter.fillPath(background, QColor(QStringLiteral("#0a0a0a")));
    QWidget::paintEvent(event);

    QPen borderPen(QColor(QStringLiteral("#2c2c30")), 1.0);
    borderPen.setCosmetic(true);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    if (cornerRadius_ > 0) {
        painter.drawRoundedRect(bounds, cornerRadius_, cornerRadius_);
    } else {
        painter.drawRect(bounds);
    }

    Q_UNUSED(event);
}
