#pragma once

#include <QMouseEvent>
#include <QPoint>
#include <QWidget>

class QLabel;
class QToolButton;

class CustomTitleBar : public QWidget {
    Q_OBJECT

public:
    struct Options {
        bool showMinimize = true;
        bool showMaximize = true;
        bool showClose = true;
        bool maximizeEnabled = true;
    };

    static Options mainWindowOptions();

    CustomTitleBar(QWidget* window, const QString& title, const Options& options,
                   QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void refreshFonts();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void setupUi(const Options& options);
    QToolButton* makeButton(const QString& objectName, bool isClose = false);
    void updateMaximizeIcon();
    void toggleMaximize();
    void closeWindow();

    QWidget* window_ = nullptr;
    QLabel* iconLabel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QToolButton* minimizeButton_ = nullptr;
    QToolButton* maximizeButton_ = nullptr;
    QToolButton* closeButton_ = nullptr;
    bool maximizeEnabled_ = true;
    bool dragging_ = false;
    QPoint dragOffset_;
};

void applyFramelessChrome(QWidget* window);
