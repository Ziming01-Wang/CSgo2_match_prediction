#pragma once

#include "domain/Types.h"

#include <QWidget>

class QComboBox;
class QVBoxLayout;

class TeamPage : public QWidget {
 public:
  explicit TeamPage(const cs2::Catalog& catalog, QWidget* parent = nullptr);

 private:
  void render();
  QString flipped(const QString& score) const;

  cs2::Catalog catalog_;
  QComboBox* picker_ = nullptr;
  QVBoxLayout* body_ = nullptr;
};
