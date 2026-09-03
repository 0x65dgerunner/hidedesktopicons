#include "icon_helper.hpp"

#include <QAbstractButton>
#include <QEvent>
#include <QFile>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

namespace AppIcons {

namespace {

class HoverIconFilter : public QObject {
public:
    HoverIconFilter(QAbstractButton* button, QIcon strokeIcon, QIcon solidIcon)
        : QObject(button), button_(button), strokeIcon_(std::move(strokeIcon)),
          solidIcon_(std::move(solidIcon)) {
        button_->installEventFilter(this);
    }

    void setIcons(QIcon strokeIcon, QIcon solidIcon) {
        strokeIcon_ = std::move(strokeIcon);
        solidIcon_ = std::move(solidIcon);
        if (button_ != nullptr && button_->underMouse()) {
            button_->setIcon(solidIcon_);
        } else if (button_ != nullptr) {
            button_->setIcon(strokeIcon_);
        }
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched != button_ || !button_->isEnabled()) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
            case QEvent::Enter:
                button_->setIcon(solidIcon_);
                break;
            case QEvent::Leave:
                if (!button_->isDown()) {
                    button_->setIcon(strokeIcon_);
                }
                break;
            case QEvent::MouseButtonRelease: {
                const auto* mouseEvent = static_cast<QMouseEvent*>(event);
                if (!button_->rect().contains(
                        button_->mapFromGlobal(mouseEvent->globalPosition().toPoint()))) {
                    button_->setIcon(strokeIcon_);
                }
                break;
            }
            default:
                break;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QAbstractButton* button_ = nullptr;
    QIcon strokeIcon_;
    QIcon solidIcon_;
};

QByteArray loadSvg(const QString& resourcePath, const QColor& color) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QByteArray data = file.readAll();
    if (color.isValid()) {
        const QByteArray hex = color.name(QColor::HexRgb).toUtf8();
        data.replace("currentColor", hex);
    }
    return data;
}

QPixmap renderSvg(const QString& resourcePath, int logicalSize, const QColor& color,
                  qreal devicePixelRatio) {
    const qreal dpr = qMax(devicePixelRatio, 1.0);
    const int physicalSize = qMax(1, qRound(logicalSize * dpr));

    QPixmap pixmap(physicalSize, physicalSize);
    pixmap.fill(Qt::transparent);

    const QByteArray svg = loadSvg(resourcePath, color);
    if (svg.isEmpty()) {
        return pixmap;
    }

    QSvgRenderer renderer(svg);
    if (!renderer.isValid()) {
        return pixmap;
    }

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    renderer.render(&painter, QRectF(0, 0, physicalSize, physicalSize));
    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}

}

QPixmap pixmap(const QString& resourcePath, int size, const QColor& color,
               qreal devicePixelRatio) {
    return renderSvg(resourcePath, size, color, devicePixelRatio);
}

QIcon icon(const QString& resourcePath, const QColor& color) {
    QIcon result;
    for (const int size : {16, 18, 20, 24, 32}) {
        result.addPixmap(renderSvg(resourcePath, size, color, 1.0));
    }
    return result;
}

QIcon hoverIcon(const QString& strokePath, const QString& solidPath, const QColor& color) {
    QIcon result;
    for (const int size : {16, 18, 20, 24, 32}) {
        result.addPixmap(renderSvg(strokePath, size, color, 1.0), QIcon::Normal, QIcon::Off);
        result.addPixmap(renderSvg(solidPath, size, color, 1.0), QIcon::Active, QIcon::Off);
        result.addPixmap(renderSvg(solidPath, size, color, 1.0), QIcon::Selected, QIcon::Off);
    }
    return result;
}

void applyHoverIcon(QAbstractButton* button, const QString& strokePath, const QString& solidPath,
                    const QColor& color) {
    if (button == nullptr) {
        return;
    }

    const QIcon strokeIcon = icon(strokePath, color);
    const QIcon solidIcon = icon(solidPath, color);

    for (QObject* child : button->children()) {
        if (auto* filter = dynamic_cast<HoverIconFilter*>(child)) {
            filter->setIcons(strokeIcon, solidIcon);
            return;
        }
    }

    button->setIcon(strokeIcon);
    new HoverIconFilter(button, strokeIcon, solidIcon);
}

}
