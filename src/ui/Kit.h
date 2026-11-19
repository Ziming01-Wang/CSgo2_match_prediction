#pragma once

#include <QColor>
#include <QString>
#include <QStringList>

class QFrame;
class QLabel;
class QLayout;
class QScrollArea;
class QTableWidget;
class QWidget;
class QVBoxLayout;

struct PageShell {
  QScrollArea* scroll = nullptr;
  QWidget* body = nullptr;
  QVBoxLayout* layout = nullptr;
};

struct Section {
  QFrame* frame = nullptr;
  QVBoxLayout* layout = nullptr;
};

PageShell makePageShell(QWidget* page);
Section makeSection(const QString& title, const QString& hint, QWidget* parent);
QWidget* makePageHeader(const QString& eyebrow, const QString& title, const QString& desc, const QString& stamp, QWidget* parent);
QString sampleStamp(const QString& updatedAt);
QFrame* makeMetric(const QString& label, const QString& value, const QString& hint, QWidget* parent);
QLabel* makePill(const QString& text, const QString& kind, QWidget* parent);
QWidget* makeFormPills(const QString& form, QWidget* parent);
QTableWidget* makeTable(const QStringList& headers, QWidget* parent);
void writeCell(QTableWidget* table, int row, int col, const QString& text,
               int align = 0, const QColor& color = QColor());
void resetTable(QTableWidget* table, int rows);
void fitTable(QTableWidget* table);
void lockTable(QTableWidget* table, int height);
void writeEmpty(QTableWidget* table, const QString& text);
void clearLayout(QLayout* layout);
void polish(QWidget* widget);

QColor amberColor();
QColor winColor();
QColor lossColor();
