#pragma once

#include <QWidget>

class QComboBox;
class QLabel;
class QTableWidget;
class QVBoxLayout;
class Session;

class ArchivePage : public QWidget {
 public:
  explicit ArchivePage(Session* session, const QString& updatedAt, QWidget* parent = nullptr);

  void reload();

 private:
  void showDetail(int index);

  Session* session_ = nullptr;
  QString updatedAt_;
  QTableWidget* table_ = nullptr;
  QLabel* empty_ = nullptr;
  QVBoxLayout* detail_ = nullptr;
  int selected_ = -1;
};
