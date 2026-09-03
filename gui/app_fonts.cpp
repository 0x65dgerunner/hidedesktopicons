#include "app_fonts.hpp"

#include <QApplication>
#include <QFont>

namespace {

QString g_family;

void configure(QFont& font) {
    font.setHintingPreference(QFont::PreferNoHinting);
    font.setStyleStrategy(
        static_cast<QFont::StyleStrategy>(QFont::PreferAntialias | QFont::PreferQuality));
}

QFont makeFont(QFont::Weight weight, qreal pointSize) {
    QFont font(g_family);
    font.setWeight(weight);
    font.setPointSizeF(pointSize);
    configure(font);
    return font;
}

}

namespace AppFonts {

bool initialize() {
#ifdef Q_OS_WIN
    g_family = QStringLiteral("Segoe UI");
#else
    g_family = QStringLiteral("Sans Serif");
#endif

    if (QApplication::instance() != nullptr) {
        QApplication::setFont(regular(10.0));
    }
    return true;
}

QString family() {
    return g_family;
}

QFont regular(qreal pointSize) {
    return makeFont(QFont::Normal, pointSize);
}

QFont medium(qreal pointSize) {
    return makeFont(QFont::Medium, pointSize);
}

}
