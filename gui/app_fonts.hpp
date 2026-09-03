#pragma once

#include <QFont>
#include <QString>

namespace AppFonts {

bool initialize();
QString family();
QFont regular(qreal pointSize);
QFont medium(qreal pointSize);

}
