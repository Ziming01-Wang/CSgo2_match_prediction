#include "ui/pages/TeamPage.h"

#include "common/Text.h"
#include "domain/Catalog.h"
#include "ui/Kit.h"
#include "ui/widgets/WinBar.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

TeamPage::TeamPage(const cs2::Catalog& catalog, QWidget* parent) : QWidget(parent), catalog_(catalog) {
  const PageShell shell = makePageShell(this);
  shell.layout->addWidget(makePageHeader(
      zh("战队分析"), zh("单队样本"),
      zh("选择一支战队，查看它在当前窗口里的系列赛、地图和交手。"),
      sampleStamp(catalog_.updatedAt), this));

  Section control = makeSection(zh("战队"), zh("按样本强度排序"), this);
  picker_ = new QComboBox(control.frame);
  for (const cs2::Team& team : catalog_.teams)
    picker_->addItem(team.name, team.id);
  control.layout->addWidget(picker_);
  shell.layout->addWidget(control.frame);

  body_ = new QVBoxLayout;
  body_->setSpacing(16);
  shell.layout->addLayout(body_);
  shell.layout->addStretch();

  connect(picker_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { render(); });
  render();
}

QString TeamPage::flipped(const QString& score) const {
  const int dash = score.indexOf(QLatin1Char('-'));
  if (dash < 0)
    return score;
  return score.mid(dash + 1) + QLatin1Char('-') + score.left(dash);
}

void TeamPage::render() {
  clearLayout(body_);
  const QString id = picker_->currentData().toString();
  const cs2::Team* team = cs2::findTeam(catalog_, id);
  if (!team)
    return;

  auto* identity = new QFrame;
  identity->setObjectName(QStringLiteral("Card"));
  auto* row = new QHBoxLayout(identity);
  row->setContentsMargins(16, 14, 16, 14);
  auto* text = new QVBoxLayout;
  text->setSpacing(4);
  auto* name = new QLabel(team->name, identity);
  name->setObjectName(QStringLiteral("Title"));
  auto* meta = new QLabel(team->region + zh("   ·   样本排名 #") + QString::number(team->rank) + zh("   ·   强度 ") +
                              QString::number(team->rating, 'f', 0),
                          identity);
  meta->setObjectName(QStringLiteral("Muted"));
  text->addWidget(name);
  text->addWidget(meta);
  text->addWidget(makeFormPills(team->form, identity));
  row->addLayout(text, 1);
  body_->addWidget(identity);

  int mapWins = 0;
  int mapPlayed = 0;
  for (const cs2::MapStat& stat : cs2::mapsFor(catalog_, team->id)) {
    mapWins += stat.wins;
    mapPlayed += stat.played;
  }
  const double mapRate = mapPlayed ? double(mapWins) / double(mapPlayed) : 0;
  auto* metrics = new QHBoxLayout;
  metrics->setSpacing(14);
  metrics->addWidget(makeMetric(zh("近期胜率"), pct(team->recentWinRate),
                                QString::number(team->seriesWins) + QLatin1Char('/') + QString::number(team->seriesPlayed) + zh(" 场系列赛"),
                                this));
  metrics->addWidget(makeMetric(zh("地图胜率"), pct(mapRate), QString::number(mapWins) + QLatin1Char('/') + QString::number(mapPlayed) + zh(" 张地图"), this));
  metrics->addWidget(makeMetric(zh("地图场次"), QString::number(mapPlayed), zh("窗口内有比分的地图"), this));
  body_->addLayout(metrics);

  auto* split = new QHBoxLayout;
  split->setSpacing(14);

  Section maps = makeSection(zh("不同地图胜率"), zh("没打过的图记为 0"), this);
  auto* mapTable = makeTable({zh("地图"), zh("胜/场"), zh("胜率"), zh("")}, maps.frame);
  const QStringList pool = cs2::activeMaps();
  resetTable(mapTable, pool.size());
  for (int i = 0; i < pool.size(); ++i) {
    const cs2::MapStat* stat = cs2::findMap(catalog_, team->id, pool[i]);
    const int played = stat ? stat->played : 0;
    const int wins = stat ? stat->wins : 0;
    const double rate = played ? double(wins) / double(played) : 0;
    writeCell(mapTable, i, 0, pool[i]);
    writeCell(mapTable, i, 1, QString::number(wins) + QLatin1Char('/') + QString::number(played), Qt::AlignCenter);
    writeCell(mapTable, i, 2, played ? pct(rate) : QStringLiteral("—"), Qt::AlignCenter, played ? amberColor() : QColor());
    auto* bar = new WinBar(mapTable);
    bar->setValue(rate);
    if (!played)
      bar->setColor(QColor(QStringLiteral("#314056")));
    mapTable->setCellWidget(i, 3, bar);
  }
  fitTable(mapTable);
  maps.layout->addWidget(mapTable);
  split->addWidget(maps.frame, 1);

  Section recent = makeSection(zh("最近比赛记录"), QString(), this);
  auto* recentTable = makeTable({zh("日期"), zh("赛事"), zh("对手"), zh("比分"), zh("赛制")}, recent.frame);
  const QVector<cs2::Match> matches = cs2::matchesFor(catalog_, team->id);
  if (matches.isEmpty()) {
    writeEmpty(recentTable, zh("这个窗口没有比赛"));
  } else {
    resetTable(recentTable, matches.size());
    for (int i = 0; i < matches.size(); ++i) {
      const cs2::Match& match = matches[i];
      const bool sideA = match.teamA == team->id;
      const int mine = cs2::seriesScore(match, sideA);
      const int theirs = cs2::seriesScore(match, !sideA);
      const bool won = mine > theirs;
      writeCell(recentTable, i, 0, match.date);
      writeCell(recentTable, i, 1, match.eventName);
      writeCell(recentTable, i, 2, cs2::teamName(catalog_, cs2::opponentId(match, team->id)));
      writeCell(recentTable, i, 3, QString::number(mine) + QLatin1Char('-') + QString::number(theirs), Qt::AlignCenter, won ? winColor() : lossColor());
      writeCell(recentTable, i, 4, match.format, Qt::AlignCenter);
      recentTable->item(i, 3)->setToolTip(cs2::mapLine(match));
    }
    fitTable(recentTable);
  }
  recent.layout->addWidget(recentTable);
  split->addWidget(recent.frame, 1);
  body_->addLayout(split);

  Section h2h = makeSection(zh("历史交手"), zh("只统计当前窗口"), this);
  auto* h2hTable = makeTable({zh("对手"), zh("场次"), zh("战绩"), zh("最近一场"), zh("日期")}, h2h.frame);
  const QVector<cs2::HeadToHead> rows = cs2::h2hFor(catalog_, team->id);
  if (rows.isEmpty()) {
    writeEmpty(h2hTable, zh("近三月无交手样本"));
  } else {
    resetTable(h2hTable, rows.size());
    for (int i = 0; i < rows.size(); ++i) {
      const cs2::HeadToHead& row = rows[i];
      const bool left = row.teamA == team->id;
      const int wins = left ? row.winsA : row.winsB;
      const int losses = left ? row.winsB : row.winsA;
      writeCell(h2hTable, i, 0, cs2::teamName(catalog_, left ? row.teamB : row.teamA));
      writeCell(h2hTable, i, 1, QString::number(wins + losses), Qt::AlignCenter);
      writeCell(h2hTable, i, 2, QString::number(wins) + QLatin1Char('-') + QString::number(losses), Qt::AlignCenter, wins >= losses ? winColor() : lossColor());
      writeCell(h2hTable, i, 3, left ? row.lastScore : flipped(row.lastScore), Qt::AlignCenter);
      writeCell(h2hTable, i, 4, row.lastDate);
    }
    fitTable(h2hTable);
  }
  h2h.layout->addWidget(h2hTable);
  body_->addWidget(h2h.frame);
}
