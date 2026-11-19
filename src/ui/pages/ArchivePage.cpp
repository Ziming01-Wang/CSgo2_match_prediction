#include "ui/pages/ArchivePage.h"

#include "app/Session.h"
#include "common/Text.h"
#include "ui/Kit.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

double alignedWin(const cs2::PredictionView& base, const cs2::PredictionView& other) {
  if (other.format != base.format)
    return -1;
  if (other.teamA == base.teamA && other.teamB == base.teamB)
    return other.winA;
  if (other.teamA == base.teamB && other.teamB == base.teamA)
    return 1.0 - other.winA;
  return -1;
}

}  // namespace

ArchivePage::ArchivePage(Session* session, const QString& updatedAt, QWidget* parent)
    : QWidget(parent), session_(session), updatedAt_(updatedAt) {
  const PageShell shell = makePageShell(this);
  shell.layout->addWidget(makePageHeader(
      zh("预测记录"), zh("回看与比较"),
      zh("保存在本次运行的内存里。同一对阵、同一赛制可以比较胜率差。关闭窗口后清空。"),
      sampleStamp(updatedAt_), this));

  auto* actions = new QHBoxLayout;
  auto* remove = new QPushButton(zh("删除选中"), this);
  auto* clear = new QPushButton(zh("清空"), this);
  remove->setObjectName(QStringLiteral("GhostButton"));
  clear->setObjectName(QStringLiteral("GhostButton"));
  remove->setCursor(Qt::PointingHandCursor);
  clear->setCursor(Qt::PointingHandCursor);
  actions->addWidget(remove);
  actions->addWidget(clear);
  actions->addStretch();
  shell.layout->addLayout(actions);

  empty_ = new QLabel(zh("还没有保存的预测。到「比赛预测」选择两支战队后，可以保存当前推演。"), this);
  empty_->setWordWrap(true);
  empty_->setObjectName(QStringLiteral("EmptyHint"));
  shell.layout->addWidget(empty_);

  table_ = makeTable({zh("时间"), zh("对阵"), zh("赛制"), zh("A 胜率"), zh("倾向")}, this);
  shell.layout->addWidget(table_);

  detail_ = new QVBoxLayout;
  detail_->setSpacing(12);
  shell.layout->addLayout(detail_);
  shell.layout->addStretch();

  connect(table_, &QTableWidget::itemSelectionChanged, this, [this] {
    const int row = table_->currentRow();
    showDetail(row);
  });
  connect(remove, &QPushButton::clicked, this, [this] {
    if (session_ && selected_ >= 0)
      session_->removeAt(selected_);
  });
  connect(clear, &QPushButton::clicked, this, [this] {
    if (session_)
      session_->clear();
  });
  if (session_)
    connect(session_, &Session::changed, this, [this] { reload(); });
  reload();
}

void ArchivePage::reload() {
  const QString previous = selected_ >= 0 && session_ && selected_ < session_->records().size()
                               ? session_->records().at(selected_).id
                               : QString();
  const QVector<cs2::PredictionView> rows = session_ ? session_->records() : QVector<cs2::PredictionView>();
  empty_->setVisible(rows.isEmpty());
  table_->setVisible(!rows.isEmpty());
  if (rows.isEmpty()) {
    selected_ = -1;
    clearLayout(detail_);
    resetTable(table_, 0);
    fitTable(table_);
    return;
  }

  resetTable(table_, rows.size());
  int restore = 0;
  for (int i = 0; i < rows.size(); ++i) {
    const cs2::PredictionView& view = rows[i];
    if (view.id == previous)
      restore = i;
    const QString lean = view.winA >= 0.5 ? view.teamAName : view.teamBName;
    writeCell(table_, i, 0, view.createdAt);
    writeCell(table_, i, 1, view.teamAName + zh("  vs  ") + view.teamBName);
    writeCell(table_, i, 2, view.format, Qt::AlignCenter);
    writeCell(table_, i, 3, pct(view.winA), Qt::AlignCenter, amberColor());
    writeCell(table_, i, 4, lean);
  }
  fitTable(table_);
  table_->selectRow(restore);
  showDetail(restore);
}

void ArchivePage::showDetail(int index) {
  selected_ = index;
  clearLayout(detail_);
  if (!session_ || index < 0 || index >= session_->records().size())
    return;
  const cs2::PredictionView view = session_->records().at(index);

  Section card = makeSection(view.teamAName + zh("  vs  ") + view.teamBName, view.format + zh("  ·  ") + view.createdAt, this);
  auto* headline = new QLabel(view.teamAName + QLatin1Char(' ') + pct(view.winA) + zh("    ") + view.teamBName + QLatin1Char(' ') + pct(1.0 - view.winA), card.frame);
  headline->setObjectName(QStringLiteral("SectionTitle"));
  card.layout->addWidget(headline);
  auto* summary = new QLabel(view.headline, card.frame);
  summary->setWordWrap(true);
  summary->setObjectName(QStringLiteral("Muted"));
  card.layout->addWidget(summary);

  for (const cs2::Factor& factor : view.factors) {
    auto* line = new QLabel(factor.name + zh("：") + factor.detail, card.frame);
    line->setWordWrap(true);
    line->setObjectName(QStringLiteral("Muted"));
    card.layout->addWidget(line);
  }

  auto* compareRow = new QHBoxLayout;
  auto* caption = new QLabel(zh("对比记录"), card.frame);
  caption->setObjectName(QStringLiteral("Muted"));
  auto* combo = new QComboBox(card.frame);
  combo->addItem(zh("不对比"), -1);
  const QVector<cs2::PredictionView>& records = session_->records();
  for (int i = 0; i < records.size(); ++i) {
    if (i == index)
      continue;
    const cs2::PredictionView& other = records[i];
    combo->addItem(other.createdAt + QStringLiteral("  ") + other.teamAName + zh(" vs ") + other.teamBName + QStringLiteral("  ") + other.format, i);
  }
  auto* delta = new QLabel(card.frame);
  delta->setWordWrap(true);
  delta->setObjectName(QStringLiteral("Muted"));
  compareRow->addWidget(caption);
  compareRow->addWidget(combo, 1);
  card.layout->addLayout(compareRow);
  card.layout->addWidget(delta);

  connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo, delta, view](int) {
    const int otherIndex = combo->currentData().toInt();
    if (!session_ || otherIndex < 0 || otherIndex >= session_->records().size()) {
      delta->clear();
      return;
    }
    const cs2::PredictionView other = session_->records().at(otherIndex);
    const double aligned = alignedWin(view, other);
    if (aligned < 0) {
      delta->setText(zh("对阵或赛制不同，只保留各自的数字，不算胜率差。"));
      return;
    }
    const double diff = (view.winA - aligned) * 100.0;
    delta->setText(view.teamAName + zh(" 的胜率比这条对比记录 ") + (diff >= 0 ? zh("高 ") : zh("低 ")) +
                   QString::number(qAbs(diff), 'f', 1) + zh(" 个百分点。"));
  });

  detail_->addWidget(card.frame);
}
