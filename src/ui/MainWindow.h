#pragma once

#include "domain/Types.h"

#include <QMainWindow>

class QPushButton;
class QStackedWidget;
class Session;

class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  void refreshArchiveLabel();

  cs2::Catalog catalog_;
  Session* session_ = nullptr;
  QStackedWidget* stack_ = nullptr;
  QPushButton* archiveButton_ = nullptr;
};
