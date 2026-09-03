#pragma once

#include <QMenu>

class RoundedMenu final : public QMenu {
public:
    explicit RoundedMenu(QWidget* parent = nullptr);

    void applyTrayStyle();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;
};
