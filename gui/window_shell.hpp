#pragma once

#include <QWidget>

class QPaintEvent;

constexpr int kWindowCornerRadius = 12;

class RoundedWindowShell : public QWidget {
    Q_OBJECT

public:
    explicit RoundedWindowShell(QWidget* parent = nullptr);

    void setCornerRadius(int radius);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int cornerRadius_ = kWindowCornerRadius;
};
