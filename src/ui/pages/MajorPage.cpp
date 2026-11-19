#include "ui/pages/MajorPage.h"

#include "common/Text.h"
#include "domain/Preview.h"
#include "ui/Kit.h"
#include "ui/widgets/WinBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

MajorPage::MajorPage(const cs2::Catalog& catalog, QWidget* parent) : QWidget(parent), catalog_(catalog) {
  const PageShell shell = makePageShell(this);
  shell.layout->addWidget(makePageHeader(
      zh("Major 模拟"), zh("瑞士轮与淘汰赛"),
      zh("用样本强度做一次可重复的赛事推演，方便先把晋级、签表和夺冠概率的位置定下来。"),
      sampleStamp(catalog_.updatedAt), this));

  Section control = makeSection(zh("参赛池"), zh("16 支样本战队"), this);
  QStringList names;
  for (const cs2::Team& team : catalog_.teams)
    names << team.name;
  auto* pool = new QLabel(names.join(zh("    ")), control.frame);
  pool->setWordWrap(true);
  pool->setObjectName(QStringLiteral("Muted"));
  control.layout->addWidget(pool);

  auto* row = new QHBoxLayout;
  seedLabel_ = new QLabel(control.frame);
  seedLabel_->setObjectName(QStringLiteral("SectionTitle"));
  auto* rerun = new QPushButton(zh("重新推演"), control.frame);
  rerun->setObjectName(QStringLiteral("PrimaryButton"));
  rerun->setCursor(Qt::PointingHandCursor);
  row->addWidget(seedLabel_);
  row->addStretch();
  row->addWidget(rerun);
  control.layout->addLayout(row);
  auto* note = new QLabel(zh("晋级名单会随推演序号变化。夺冠概率只在晋级队里按样本强度分配，不由这一次签表的胜者决定。"), control.frame);
  note->setWordWrap(true);
  note->setObjectName(QStringLiteral("Muted"));
  control.layout->addWidget(note);
  shell.layout->addWidget(control.frame);

  body_ = new QVBoxLayout;
  body_->setSpacing(16);
  shell.layout->addLayout(body_);
  shell.layout->addStretch();
  connect(rerun, &QPushButton::clicked, this, [this] {
    ++seed_;
    render();
  });
  render();
}

void MajorPage::render() {
  clearLayout(body_);
  seedLabel_->setText(zh("推演序号  ") + QString::number(seed_));
  const cs2::MajorPreview preview = cs2::previewMajor(catalog_, seed_);

  Section swiss = makeSection(zh("瑞士轮"), zh("前 8 晋级"), this);
  auto* swissTable = makeTable({zh("排名"), zh("战队"), zh("地区"), zh("战绩"), zh("状态"), zh("晋级概率")}, swiss.frame);
  resetTable(swissTable, preview.swiss.size());
  for (int i = 0; i < preview.swiss.size(); ++i) {
    const cs2::SwissSlot& row = preview.swiss[i];
    const bool advanced = row.status == zh("晋级");
    writeCell(swissTable, i, 0, QString::number(i + 1), Qt::AlignCenter);
    writeCell(swissTable, i, 1, row.teamName);
    writeCell(swissTable, i, 2, row.region);
    writeCell(swissTable, i, 3, row.record, Qt::AlignCenter);
    writeCell(swissTable, i, 4, row.status, Qt::AlignCenter, advanced ? winColor() : lossColor());
    writeCell(swissTable, i, 5, pct(row.advance), Qt::AlignCenter);
  }
  fitTable(swissTable);
  swiss.layout->addWidget(swissTable);
  body_->addWidget(swiss.frame);

  auto* split = new QHBoxLayout;
  split->setSpacing(14);

  Section bracket = makeSection(zh("淘汰赛签表"), zh("由晋级顺序配对"), this);
  auto* ties = makeTable({zh("轮次"), zh("对阵"), zh("倾向"), zh("左侧胜率")}, bracket.frame);
  resetTable(ties, preview.ties.size());
  for (int i = 0; i < preview.ties.size(); ++i) {
    const cs2::PlayoffTie& row = preview.ties[i];
    writeCell(ties, i, 0, row.round);
    writeCell(ties, i, 1, row.faceoff);
    writeCell(ties, i, 2, row.lean, Qt::AlignCenter, amberColor());
    writeCell(ties, i, 3, pct(row.winA), Qt::AlignCenter);
  }
  fitTable(ties);
  bracket.layout->addWidget(ties);
  split->addWidget(bracket.frame, 3);

  Section title = makeSection(zh("最终夺冠概率"), zh("晋级队内归一化"), this);
  for (const cs2::OddsRow& row : preview.champions) {
    auto* line = new QWidget(title.frame);
    auto* layout = new QHBoxLayout(line);
    layout->setContentsMargins(0, 4, 0, 4);
    auto* name = new QLabel(row.teamName, line);
    name->setFixedWidth(120);
    auto* bar = new WinBar(line);
    bar->setValue(row.probability);
    auto* rate = new QLabel(pct(row.probability), line);
    rate->setFixedWidth(56);
    rate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(name);
    layout->addWidget(bar, 1);
    layout->addWidget(rate);
    title.layout->addWidget(line);
  }
  split->addWidget(title.frame, 2);
  body_->addLayout(split);
}
