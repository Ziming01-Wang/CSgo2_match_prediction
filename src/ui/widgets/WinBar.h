#pragma once

#include <QColor>
#include <QWidget>

class WinBar : public QWidget {
 public:
  explicit WinBar(QWidget* parent = nullptr);

  void setValue(double ratio);
  void setColor(const QColor& color);

  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  double value_ = 0;
  QColor color_;
};
