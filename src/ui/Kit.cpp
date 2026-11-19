#include "ui/Kit.h"

#include "common/Text.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

PageShell makePageShell(QWidget* page) {
  page->setObjectName(QStringLiteral("AppPage"));
  page->setAttribute(Qt::WA_StyledBackground, true);

  auto* outer = new QVBoxLayout(page);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(0);

  PageShell shell;
  shell.scroll = new QScrollArea(page);
  shell.scroll->setWidgetResizable(true);
  shell.scroll->setFrameShape(QFrame::NoFrame);
  shell.scroll->viewport()->setAutoFillBackground(false);
  outer->addWidget(shell.scroll);

  shell.body = new QWidget;
  shell.body->setObjectName(QStringLiteral("PageBody"));
  shell.body->setAttribute(Qt::WA_StyledBackground, true);
  shell.body->setMinimumWidth(980);
  shell.scroll->setWidget(shell.body);

  shell.layout = new QVBoxLayout(shell.body);
  shell.layout->setContentsMargins(28, 24, 28, 32);
  shell.layout->setSpacing(16);
  return shell;
}

Section makeSection(const QString& title, const QString& hint, QWidget* parent) {
  Section section;
  section.frame = new QFrame(parent);
  section.frame->setObjectName(QStringLiteral("Card"));
  section.layout = new QVBoxLayout(section.frame);
  section.layout->setContentsMargins(16, 14, 16, 16);
  section.layout->setSpacing(10);

  auto* head = new QHBoxLayout;
  head->setSpacing(12);
  auto* label = new QLabel(title, section.frame);
  label->setObjectName(QStringLiteral("SectionTitle"));
  head->addWidget(label);
  head->addStretch();
  if (!hint.isEmpty()) {
    auto* note = new QLabel(hint, section.frame);
    note->setObjectName(QStringLiteral("Muted"));
    head->addWidget(note);
  }
  section.layout->addLayout(head);
  return section;
}

QString sampleStamp(const QString& updatedAt) {
  return zh("样本更新 ") + updatedAt + QLatin1Char('\n') + zh("未接入比赛数据源");
}

QWidget* makePageHeader(const QString& eyebrow, const QString& title, const QString& desc, const QString& stamp, QWidget* parent) {
  auto* wrap = new QWidget(parent);
  auto* row = new QHBoxLayout(wrap);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(16);

  auto* left = new QVBoxLayout;
  left->setSpacing(4);
  auto* eye = new QLabel(eyebrow, wrap);
  eye->setObjectName(QStringLiteral("Eyebrow"));
  auto* heading = new QLabel(title, wrap);
  heading->setObjectName(QStringLiteral("Title"));
  auto* note = new QLabel(desc, wrap);
  note->setObjectName(QStringLiteral("Muted"));
  note->setWordWrap(true);
  left->addWidget(eye);
  left->addWidget(heading);
  left->addWidget(note);
  row->addLayout(left, 1);

  auto* chip = new QLabel(stamp, wrap);
  chip->setObjectName(QStringLiteral("Stamp"));
  chip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  row->addWidget(chip, 0, Qt::AlignTop);
  return wrap;
}

QFrame* makeMetric(const QString& label, const QString& value, const QString& hint, QWidget* parent) {
  auto* card = new QFrame(parent);
  card->setObjectName(QStringLiteral("Card"));
  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 14, 16, 14);
  layout->setSpacing(4);
  auto* caption = new QLabel(label, card);
  caption->setObjectName(QStringLiteral("MetricLabel"));
  auto* number = new QLabel(value, card);
  number->setObjectName(QStringLiteral("MetricValue"));
  auto* extra = new QLabel(hint, card);
  extra->setObjectName(QStringLiteral("MetricHint"));
  layout->addWidget(caption);
  layout->addWidget(number);
  layout->addWidget(extra);
  return card;
}

QLabel* makePill(const QString& text, const QString& kind, QWidget* parent) {
  auto* label = new QLabel(text, parent);
  label->setAlignment(Qt::AlignCenter);
  label->setFixedSize(22, 22);
  if (kind == QLatin1String("win"))
    label->setObjectName(QStringLiteral("PillWin"));
  else if (kind == QLatin1String("loss"))
    label->setObjectName(QStringLiteral("PillLoss"));
  else
    label->setObjectName(QStringLiteral("PillMuted"));
  return label;
}

QWidget* makeFormPills(const QString& form, QWidget* parent) {
  auto* wrap = new QWidget(parent);
  auto* layout = new QHBoxLayout(wrap);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(4);
  if (form.isEmpty() || form == QLatin1String("-")) {
    layout->addWidget(makePill(QStringLiteral("-"), QStringLiteral("muted"), wrap));
  } else {
    for (const QChar ch : form) {
      const bool win = ch == QLatin1Char('W');
      layout->addWidget(makePill(win ? QStringLiteral("W") : QStringLiteral("L"), win ? QStringLiteral("win") : QStringLiteral("loss"), wrap));
    }
  }
  layout->addStretch();
  return wrap;
}

QTableWidget* makeTable(const QStringList& headers, QWidget* parent) {
  auto* table = new QTableWidget(0, headers.size(), parent);
  table->setHorizontalHeaderLabels(headers);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setShowGrid(false);
  table->setAlternatingRowColors(true);
  table->setFocusPolicy(Qt::NoFocus);
  table->setWordWrap(false);
  table->setTextElideMode(Qt::ElideRight);
  table->verticalHeader()->setDefaultSectionSize(40);
  table->horizontalHeader()->setStretchLastSection(true);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->horizontalHeader()->setMinimumSectionSize(64);
  table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  return table;
}

void writeCell(QTableWidget* table, int row, int col, const QString& text, int align, const QColor& color) {
  auto* item = new QTableWidgetItem(text);
  item->setTextAlignment(align == 0 ? int(Qt::AlignVCenter | Qt::AlignLeft) : align);
  item->setToolTip(text);
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  if (color.isValid())
    item->setForeground(color);
  table->setItem(row, col, item);
}

void resetTable(QTableWidget* table, int rows) {
  table->clearSpans();
  table->clearContents();
  table->setRowCount(rows);
}

void fitTable(QTableWidget* table) {
  const int rows = qMax(1, table->rowCount());
  const int height = 44 + rows * 40;
  table->setMinimumHeight(height);
  table->setMaximumHeight(height);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void lockTable(QTableWidget* table, int height) {
  table->setMinimumHeight(height);
  table->setMaximumHeight(height);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void writeEmpty(QTableWidget* table, const QString& text) {
  resetTable(table, 1);
  writeCell(table, 0, 0, text);
  if (table->columnCount() > 1)
    table->setSpan(0, 0, 1, table->columnCount());
  fitTable(table);
}

void clearLayout(QLayout* layout) {
  if (!layout)
    return;
  while (QLayoutItem* item = layout->takeAt(0)) {
    if (QWidget* widget = item->widget()) {
      widget->hide();
      widget->deleteLater();
    }
    if (QLayout* child = item->layout())
      clearLayout(child);
    delete item;
  }
}

void polish(QWidget* widget) {
  if (!widget)
    return;
  widget->style()->unpolish(widget);
  widget->style()->polish(widget);
  widget->update();
}

QColor amberColor() { return QColor(QStringLiteral("#e8a317")); }
QColor winColor() { return QColor(QStringLiteral("#3ecf8e")); }
QColor lossColor() { return QColor(QStringLiteral("#e36b6b")); }
