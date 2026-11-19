#include "ui/widgets/WinBar.h"

#include <QPainter>
#include <QPaintEvent>

WinBar::WinBar(QWidget* parent) : QWidget(parent), color_(QStringLiteral("#e8a317")) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(16);
  setMinimumWidth(80);
}

void WinBar::setValue(double ratio) {
  value_ = ratio < 0 ? 0 : (ratio > 1 ? 1 : ratio);
  update();
}

void WinBar::setColor(const QColor& color) {
  color_ = color;
  update();
}

QSize WinBar::sizeHint() const {
  return QSize(160, 16);
}

void WinBar::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::NoPen);

  const QRectF track(0, (height() - 8) / 2.0, width(), 8);
  painter.setBrush(QColor(QStringLiteral("#243044")));
  painter.drawRoundedRect(track, 4, 4);

  QRectF fill = track;
  fill.setWidth(qMax(6.0, track.width() * value_));
  painter.setBrush(color_);
  painter.drawRoundedRect(fill, 4, 4);
}
