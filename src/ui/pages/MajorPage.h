#pragma once

#include "domain/Types.h"

#include <QWidget>

class QLabel;
class QVBoxLayout;

class MajorPage : public QWidget {
 public:
  explicit MajorPage(const cs2::Catalog& catalog, QWidget* parent = nullptr);

 private:
  void render();

  cs2::Catalog catalog_;
  int seed_ = 1;
  QLabel* seedLabel_ = nullptr;
  QVBoxLayout* body_ = nullptr;
};
