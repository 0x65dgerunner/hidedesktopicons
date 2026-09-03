#include "custom_title_bar.hpp"

#include "app_fonts.hpp"
#include "icon_helper.hpp"

#include <QDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>

namespace {

const QColor kTitleIconColor(QStringLiteral("#b8b8bd"));
constexpr int kIconTitleGapPx = 4;
constexpr int kTitleBarIconPx = 24;

QString titleBarStyleSheet() {
    return QStringLiteral(
        "CustomTitleBar#CustomTitleBar {"
        "  background-color: #0a0a0a;"
        "  border-bottom: 1px solid #1a1a1e;"
        "}"
        "QLabel#TitleBarIcon {"
        "  background: transparent;"
        "}"
        "QLabel#TitleBarLabel {"
        "  color: #a8a8ad;"
        "  background: transparent;"
        "  padding: 0;"
        "  margin: 0;"
        "}"
        "QToolButton#TitleBarButton {"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 0;"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 32px;"
        "  max-height: 32px;"
        "  background: transparent;"
        "}"
        "QToolButton#TitleBarButton:hover {"
        "  background-color: #1f1f23;"
        "}"
        "QToolButton#TitleBarButton:disabled {"
        "  background: transparent;"
        "}"
        "QToolButton#TitleBarCloseButton {"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 0;"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 32px;"
        "  max-height: 32px;"
        "  background: transparent;"
        "}"
        "QToolButton#TitleBarCloseButton:hover {"
        "  background-color: #c42b1c;"
        "}");
}

}

CustomTitleBar::Options CustomTitleBar::mainWindowOptions() {
    return {true, false, true, false};
}

void applyFramelessChrome(QWidget* window) {
    if (window == nullptr) {
        return;
    }
    window->setWindowFlags(window->windowFlags() | Qt::FramelessWindowHint);
}

CustomTitleBar::CustomTitleBar(QWidget* window, const QString& title, const Options& options,
                               QWidget* parent)
    : QWidget(parent), window_(window), maximizeEnabled_(options.maximizeEnabled) {
    setObjectName(QStringLiteral("CustomTitleBar"));
    setFixedHeight(36);
    setStyleSheet(titleBarStyleSheet());
    setupUi(options);
    setTitle(title);

    if (window_ != nullptr) {
        window_->installEventFilter(this);
    }
}

void CustomTitleBar::setupUi(const Options& options) {
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(10, 0, 4, 0);
    row->setSpacing(0);

    iconLabel_ = new QLabel(this);
    iconLabel_->setObjectName(QStringLiteral("TitleBarIcon"));
    iconLabel_->setFixedSize(kTitleBarIconPx, 32);
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setContentsMargins(0, 0, 0, 0);
    const QIcon appIcon = AppIcons::icon(AppIcons::strokeSettings(), kTitleIconColor);
    if (!appIcon.isNull()) {
        iconLabel_->setPixmap(appIcon.pixmap(kTitleBarIconPx, kTitleBarIconPx));
    }
    row->addWidget(iconLabel_);

    auto* titleGap = new QWidget(this);
    titleGap->setFixedWidth(kIconTitleGapPx);
    titleGap->setAttribute(Qt::WA_TransparentForMouseEvents);
    row->addWidget(titleGap);

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName(QStringLiteral("TitleBarLabel"));
    titleLabel_->setFont(AppFonts::regular(9.0));
    titleLabel_->setContentsMargins(0, 0, 0, 0);
    row->addWidget(titleLabel_, 1);

    if (options.showMinimize) {
        minimizeButton_ = makeButton(QStringLiteral("TitleBarButton"));
        minimizeButton_->setIconSize(QSize(14, 14));
        AppIcons::applyHoverIcon(minimizeButton_, AppIcons::strokeMinimize(),
                                 AppIcons::solidMinimize(), kTitleIconColor);
        connect(minimizeButton_, &QToolButton::clicked, window_, [this]() {
            if (window_ != nullptr) {
                window_->showMinimized();
            }
        });
        row->addWidget(minimizeButton_);
    }

    if (options.showMaximize) {
        maximizeButton_ = makeButton(QStringLiteral("TitleBarButton"));
        if (options.maximizeEnabled) {
            updateMaximizeIcon();
            connect(maximizeButton_, &QToolButton::clicked, this, &CustomTitleBar::toggleMaximize);
        } else {
            maximizeButton_->setEnabled(false);
            maximizeButton_->setIcon(AppIcons::icon(AppIcons::strokeMaximize(),
                                                    QColor(QStringLiteral("#4a4a50"))));
        }
        row->addWidget(maximizeButton_);
    }

    if (options.showClose) {
        closeButton_ = makeButton(QStringLiteral("TitleBarCloseButton"), true);
        closeButton_->setIconSize(QSize(14, 14));
        AppIcons::applyHoverIcon(closeButton_, AppIcons::strokeClose(), AppIcons::solidClose(),
                                 kTitleIconColor);
        connect(closeButton_, &QToolButton::clicked, this, &CustomTitleBar::closeWindow);
        row->addWidget(closeButton_);
    }
}

QToolButton* CustomTitleBar::makeButton(const QString& objectName, bool isClose) {
    auto* button = new QToolButton(this);
    button->setObjectName(objectName);
    button->setCursor(Qt::ArrowCursor);
    button->setAutoRaise(false);
    button->setFocusPolicy(Qt::NoFocus);
    button->setIconSize(QSize(14, 14));
    (void)isClose;
    return button;
}

void CustomTitleBar::setTitle(const QString& title) {
    if (titleLabel_ != nullptr) {
        titleLabel_->setText(title);
    }
    if (window_ != nullptr) {
        window_->setWindowTitle(title);
    }
}

void CustomTitleBar::refreshFonts() {
    if (titleLabel_ != nullptr) {
        titleLabel_->setFont(AppFonts::regular(9.0));
    }
}

bool CustomTitleBar::eventFilter(QObject* watched, QEvent* event) {
    if (watched == window_ && event->type() == QEvent::WindowStateChange) {
        updateMaximizeIcon();
    }
    return QWidget::eventFilter(watched, event);
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && window_ != nullptr) {
        if (QWidget* child = childAt(event->pos()); child != nullptr) {
            if (qobject_cast<QToolButton*>(child) != nullptr) {
                QWidget::mousePressEvent(event);
                return;
            }
        }
        dragging_ = true;
        dragOffset_ = event->globalPosition().toPoint() - window_->frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_ && window_ != nullptr && (event->buttons() & Qt::LeftButton)) {
        if (window_->isMaximized()) {
            const double ratio =
                static_cast<double>(event->pos().x()) / static_cast<double>(width());
            window_->showNormal();
            dragOffset_.setX(static_cast<int>(window_->width() * ratio));
            dragOffset_.setY(event->pos().y());
        }
        window_->move(event->globalPosition().toPoint() - dragOffset_);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && maximizeButton_ != nullptr && maximizeEnabled_) {
        toggleMaximize();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CustomTitleBar::updateMaximizeIcon() {
    if (maximizeButton_ == nullptr || window_ == nullptr) {
        return;
    }

    const bool maximized = window_->isMaximized();
    AppIcons::applyHoverIcon(
        maximizeButton_,
        maximized ? AppIcons::strokeRestore() : AppIcons::strokeMaximize(),
        maximized ? AppIcons::solidRestore() : AppIcons::solidMaximize(), kTitleIconColor);
}

void CustomTitleBar::toggleMaximize() {
    if (window_ == nullptr) {
        return;
    }
    if (window_->isMaximized()) {
        window_->showNormal();
    } else {
        window_->showMaximized();
    }
    updateMaximizeIcon();
}

void CustomTitleBar::closeWindow() {
    if (window_ == nullptr) {
        return;
    }
    if (auto* dialog = qobject_cast<QDialog*>(window_)) {
        dialog->reject();
        return;
    }
    window_->close();
}
