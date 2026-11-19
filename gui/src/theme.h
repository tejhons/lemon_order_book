#pragma once
#include <QColor>
#include <QGuiApplication>
#include <QPalette>

// Same colours as the web version, switched on the system light/dark palette.
namespace theme {

inline bool dark() {
    return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}
inline QColor bid()     { return dark() ? QColor("#4cc29b") : QColor("#1d7a5a"); }
inline QColor ask()     { return dark() ? QColor("#e0766d") : QColor("#b0413a"); }
inline QColor muted()   { return QGuiApplication::palette().color(QPalette::PlaceholderText); }
inline QColor bidFill() { QColor c = bid(); c.setAlpha(44); return c; }
inline QColor askFill() { QColor c = ask(); c.setAlpha(44); return c; }

} // namespace theme
