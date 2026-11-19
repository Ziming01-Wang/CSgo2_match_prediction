#include "ui/pages/PredictPage.h"

#include "app/Session.h"
#include "common/Text.h"
#include "domain/Catalog.h"
#include "domain/Preview.h"
#include "ui/Kit.h"
#include "ui/widgets/WinBar.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

QString leanOf(double winA, const QString& a, const QString& b) {
  if (winA >= 0.55)
    return a;
  if (winA <= 0.45)
    return b;
  return zh("均势");
}

QFrame* heroCard(const QString& name, const QString& meta, double win, bool lead, QWidget* parent) {
  auto* card = new QFrame(parent);
  card->setObjectName(QStringLiteral("HeroCard"));
  card->setProperty("lead", lead);
  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(18, 16, 18, 16);
  layout->setSpacing(6);
  auto* title = new QLabel(name, card);
  title->setObjectName(QStringLiteral("HeroName"));
  auto* percent = new QLabel(pct(win), card);
  percent->setObjectName(QStringLiteral("HeroPct"));
  percent->setProperty("lead", lead);
  auto* note = new QLabel(meta, card);
  note->setObjectName(QStringLiteral("Muted"));
  auto* bar = new WinBar(card);
  bar->setValue(win);
  bar->setColor(lead ? amberColor() : QColor(QStringLiteral("#7eb6ff")));
  bar->setFixedHeight(18);
  layout->addWidget(title);
  layout->addWidget(percent);
  layout->addWidget(bar);
  layout->addWidget(note);
  polish(card);
  polish(percent);
  return card;
}

}  // namespace

PredictPage::PredictPage(const cs2::Catalog& catalog, Session* session, QWidget* parent)
    : QWidget(parent), catalog_(catalog), session_(session) {
  const PageShell shell = makePageShell(this);
  shell.layout->addWidget(makePageHeader(
      zh("比赛预测"), zh("两队推演"),
      zh("选择 A、B 和赛制。胜率、地图、依据和选图路径会一起刷新。"),
      sampleStamp(catalog_.updatedAt), this));

  Section control = makeSection(zh("对阵"), zh("改选项即刷新"), this);
  auto* row = new QHBoxLayout;
  row->setSpacing(12);
  teamA_ = new QComboBox(control.frame);
  teamB_ = new QComboBox(control.frame);
  for (const cs2::Team& team : catalog_.teams) {
    teamA_->addItem(team.name, team.id);
    teamB_->addItem(team.name, team.id);
  }
  if (teamB_->count() > 1)
    teamB_->setCurrentIndex(1);
  auto* vs = new QLabel(QStringLiteral("VS"), control.frame);
  vs->setObjectName(QStringLiteral("VsMark"));
  vs->setAlignment(Qt::AlignCenter);
  vs->setFixedWidth(36);

  auto* seg = new QFrame(control.frame);
  seg->setObjectName(QStringLiteral("SegBar"));
  auto* segLayout = new QHBoxLayout(seg);
  segLayout->setContentsMargins(4, 4, 4, 4);
  segLayout->setSpacing(4);
  bo1_ = new QPushButton(QStringLiteral("BO1"), seg);
  bo3_ = new QPushButton(QStringLiteral("BO3"), seg);
  bo1_->setObjectName(QStringLiteral("SegButton"));
  bo3_->setObjectName(QStringLiteral("SegButton"));
  bo1_->setCheckable(true);
  bo3_->setCheckable(true);
  bo3_->setChecked(true);
  bo1_->setCursor(Qt::PointingHandCursor);
  bo3_->setCursor(Qt::PointingHandCursor);
  auto* group = new QButtonGroup(this);
  group->setExclusive(true);
  group->addButton(bo1_);
  group->addButton(bo3_);
  segLayout->addWidget(bo1_);
  segLayout->addWidget(bo3_);

  save_ = new QPushButton(zh("保存这次预测"), control.frame);
  save_->setObjectName(QStringLiteral("PrimaryButton"));
  save_->setCursor(Qt::PointingHandCursor);
  row->addWidget(teamA_);
  row->addWidget(vs);
  row->addWidget(teamB_);
  row->addWidget(seg);
  row->addStretch();
  row->addWidget(save_);
  control.layout->addLayout(row);

  warn_ = new QLabel(control.frame);
  warn_->setObjectName(QStringLiteral("Warn"));
  saved_ = new QLabel(control.frame);
  saved_->setObjectName(QStringLiteral("Muted"));
  control.layout->addWidget(warn_);
  control.layout->addWidget(saved_);
  shell.layout->addWidget(control.frame);

  results_ = new QVBoxLayout;
  results_->setSpacing(16);
  shell.layout->addLayout(results_);
  shell.layout->addStretch();

  connect(teamA_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { render(); });
  connect(teamB_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { render(); });
  connect(bo1_, &QPushButton::clicked, this, [this] { render(); });
  connect(bo3_, &QPushButton::clicked, this, [this] { render(); });
  connect(save_, &QPushButton::clicked, this, [this] { saveCurrent(); });
  render();
}

QString PredictPage::format() const {
  return bo1_->isChecked() ? QStringLiteral("BO1") : QStringLiteral("BO3");
}

void PredictPage::saveCurrent() {
  if (current_.teamA.isEmpty() || !session_)
    return;
  cs2::PredictionView view = current_;
  view.createdAt = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
  view.id = view.createdAt + QLatin1Char('-') + view.teamA + QLatin1Char('-') + view.teamB + QLatin1Char('-') + view.format;
  session_->save(view);
  saved_->setText(zh("已放入本次运行的预测记录。关闭窗口后不会保留。"));
}

void PredictPage::render() {
  clearLayout(results_);
  saved_->clear();
  const QString aId = teamA_->currentData().toString();
  const QString bId = teamB_->currentData().toString();
  if (aId.isEmpty() || aId == bId) {
    current_ = cs2::PredictionView();
    warn_->setText(zh("请选择两支不同的战队。"));
    save_->setEnabled(false);
    return;
  }
  warn_->clear();
  save_->setEnabled(true);
  current_ = cs2::previewMatch(catalog_, aId, bId, format());
  const cs2::Team* a = cs2::findTeam(catalog_, aId);
  const cs2::Team* b = cs2::findTeam(catalog_, bId);
  if (!a || !b || current_.teamA.isEmpty())
    return;

  auto* hero = new QHBoxLayout;
  hero->setSpacing(14);
  const bool aLead = current_.winA >= 0.5;
  const QString metaA = a->region + zh("  ·  #") + QString::number(a->rank) + zh("  ·  近期 ") + pct(a->recentWinRate);
  const QString metaB = b->region + zh("  ·  #") + QString::number(b->rank) + zh("  ·  近期 ") + pct(b->recentWinRate);
  hero->addWidget(heroCard(a->name, metaA, current_.winA, aLead, this), 1);
  auto* mid = new QLabel(current_.format, this);
  mid->setObjectName(QStringLiteral("VsMark"));
  mid->setAlignment(Qt::AlignCenter);
  mid->setFixedWidth(72);
  hero->addWidget(mid);
  hero->addWidget(heroCard(b->name, metaB, 1.0 - current_.winA, !aLead, this), 1);
  results_->addLayout(hero);

  auto* headline = new QLabel(current_.headline, this);
  headline->setWordWrap(true);
  headline->setObjectName(QStringLiteral("Muted"));
  results_->addWidget(headline);

  Section maps = makeSection(zh("地图预测"), zh("单图倾向，不等于系列胜率"), this);
  auto* mapTable = makeTable({zh("地图"), current_.teamAName, zh("样本"), zh("倾向"), current_.teamBName, zh("样本")}, maps.frame);
  resetTable(mapTable, current_.maps.size());
  for (int i = 0; i < current_.maps.size(); ++i) {
    const cs2::MapOutlook& row = current_.maps[i];
    const QString lean = leanOf(row.winA, current_.teamAName, current_.teamBName);
    writeCell(mapTable, i, 0, row.map);
    writeCell(mapTable, i, 1, pct(row.winA), Qt::AlignCenter, row.winA >= 0.5 ? amberColor() : QColor());
    writeCell(mapTable, i, 2, QString::number(row.sampleA), Qt::AlignCenter);
    writeCell(mapTable, i, 3, lean, Qt::AlignCenter, lean == zh("均势") ? QColor() : amberColor());
    writeCell(mapTable, i, 4, pct(1.0 - row.winA), Qt::AlignCenter);
    writeCell(mapTable, i, 5, QString::number(row.sampleB), Qt::AlignCenter);
  }
  fitTable(mapTable);
  maps.layout->addWidget(mapTable);
  results_->addWidget(maps.frame);

  Section veto = makeSection(current_.vetoTitle, zh("不同选图下的结果"), this);
  for (const cs2::VetoCase& item : current_.vetoes) {
    auto* line = new QWidget(veto.frame);
    auto* layout = new QHBoxLayout(line);
    layout->setContentsMargins(0, 4, 0, 4);
    auto* name = new QLabel(item.name, line);
    name->setObjectName(QStringLiteral("SectionTitle"));
    name->setFixedWidth(120);
    auto* bar = new WinBar(line);
    bar->setValue(item.seriesWinA);
    bar->setFixedWidth(140);
    auto* rate = new QLabel(pct(item.seriesWinA), line);
    rate->setFixedWidth(56);
    rate->setObjectName(QStringLiteral("SectionTitle"));
    auto* detail = new QLabel(item.line, line);
    detail->setWordWrap(true);
    detail->setObjectName(QStringLiteral("Muted"));
    layout->addWidget(name);
    layout->addWidget(bar);
    layout->addWidget(rate);
    layout->addWidget(detail, 1);
    veto.layout->addWidget(line);
  }
  results_->addWidget(veto.frame);

  Section reasons = makeSection(zh("预测依据"), QString(), this);
  for (const cs2::Factor& factor : current_.factors) {
    auto* line = new QWidget(reasons.frame);
    auto* layout = new QHBoxLayout(line);
    layout->setContentsMargins(0, 6, 0, 6);
    auto* name = new QLabel(factor.name, line);
    name->setFixedWidth(88);
    name->setObjectName(QStringLiteral("SectionTitle"));
    auto* bar = new WinBar(line);
    bar->setValue(factor.towardA);
    bar->setFixedWidth(120);
    auto* rate = new QLabel(pct(factor.towardA), line);
    rate->setFixedWidth(56);
    auto* detail = new QLabel(factor.detail, line);
    detail->setWordWrap(true);
    detail->setObjectName(QStringLiteral("Muted"));
    layout->addWidget(name);
    layout->addWidget(bar);
    layout->addWidget(rate);
    layout->addWidget(detail, 1);
    reasons.layout->addWidget(line);
  }
  auto* disclaimer = new QLabel(current_.disclaimer, reasons.frame);
  disclaimer->setWordWrap(true);
  disclaimer->setObjectName(QStringLiteral("Disclaimer"));
  reasons.layout->addWidget(disclaimer);
  results_->addWidget(reasons.frame);
}
