#include "ui/Theme.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QPalette>

void applyTheme(QApplication& app) {
  app.setStyle(QStringLiteral("Fusion"));

  QPalette palette;
  const QColor window("#0c1016");
  const QColor text("#e7edf5");
  const QColor base("#141a23");
  const QColor muted("#8b97a8");
  palette.setColor(QPalette::Window, window);
  palette.setColor(QPalette::WindowText, text);
  palette.setColor(QPalette::Base, base);
  palette.setColor(QPalette::AlternateBase, QColor("#121820"));
  palette.setColor(QPalette::Text, text);
  palette.setColor(QPalette::Button, QColor("#1a2230"));
  palette.setColor(QPalette::ButtonText, text);
  palette.setColor(QPalette::Highlight, QColor("#3a2e18"));
  palette.setColor(QPalette::HighlightedText, QColor("#f5c15a"));
  palette.setColor(QPalette::ToolTipBase, QColor("#1a2230"));
  palette.setColor(QPalette::ToolTipText, text);
  palette.setColor(QPalette::PlaceholderText, muted);
  app.setPalette(palette);

  QFont font(QStringLiteral("Microsoft YaHei UI"));
  font.setPixelSize(14);
  app.setFont(font);

  QFile file(QStringLiteral(":/style/app.qss"));
  if (file.open(QIODevice::ReadOnly))
    app.setStyleSheet(QString::fromUtf8(file.readAll()));
}
