#pragma once

#include "domain/Types.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class Session;

class PredictPage : public QWidget {
 public:
  explicit PredictPage(const cs2::Catalog& catalog, Session* session, QWidget* parent = nullptr);

 private:
  void render();
  void saveCurrent();
  QString format() const;

  cs2::Catalog catalog_;
  Session* session_ = nullptr;
  QComboBox* teamA_ = nullptr;
  QComboBox* teamB_ = nullptr;
  QPushButton* bo1_ = nullptr;
  QPushButton* bo3_ = nullptr;
  QPushButton* save_ = nullptr;
  QLabel* warn_ = nullptr;
  QLabel* saved_ = nullptr;
  QVBoxLayout* results_ = nullptr;
  cs2::PredictionView current_;
};
