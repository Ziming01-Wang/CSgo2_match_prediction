#include "ui/pages/HomePage.h"

#include "common/Text.h"
#include "domain/Catalog.h"
#include "ui/Kit.h"
#include "ui/widgets/WinBar.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

QString faceoff(const cs2::Catalog& catalog, const cs2::Match& match) {
  const int scoreA = cs2::seriesScore(match, true);
  const int scoreB = cs2::seriesScore(match, false);
  const bool aWon = scoreA >= scoreB;
  return QStringLiteral("%1    %2-%3    %4")
      .arg(cs2::teamName(catalog, aWon ? match.teamA : match.teamB))
      .arg(qMax(scoreA, scoreB))
      .arg(qMin(scoreA, scoreB))
      .arg(cs2::teamName(catalog, aWon ? match.teamB : match.teamA));
}

QWidget* formRow(const cs2::Team& team, QWidget* parent) {
  auto* wrap = new QWidget(parent);
  auto* layout = new QHBoxLayout(wrap);
  layout->setContentsMargins(0, 6, 0, 6);
  layout->setSpacing(10);

  auto* rank = new QLabel(QStringLiteral("#") + QString::number(team.rank), wrap);
  rank->setFixedWidth(36);
  rank->setObjectName(team.rank <= 3 ? QStringLiteral("Eyebrow") : QStringLiteral("Muted"));
  layout->addWidget(rank);

  auto* text = new QVBoxLayout;
  text->setSpacing(2);
  auto* name = new QLabel(team.name, wrap);
  name->setObjectName(QStringLiteral("SectionTitle"));
  auto* meta = new QLabel(team.region + zh("  ·  近五场"), wrap);
  meta->setObjectName(QStringLiteral("Muted"));
  text->addWidget(name);
  text->addWidget(meta);
  layout->addLayout(text, 1);
  layout->addWidget(makeFormPills(team.form, wrap));

  auto* rate = new QLabel(pct(team.recentWinRate), wrap);
  rate->setObjectName(QStringLiteral("SectionTitle"));
  rate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  rate->setFixedWidth(64);
  layout->addWidget(rate);
  return wrap;
}

}  // namespace

QWidget* buildHomePage(const cs2::Catalog& catalog, QWidget* parent) {
  auto* page = new QWidget(parent);
  const PageShell shell = makePageShell(page);
  shell.layout->addWidget(makePageHeader(
      zh("比赛数据"), zh("近三月职业赛"),
      zh("窗口 ") + catalog.windowLabel + zh("。下面的场次、状态和地图榜都来自同一份样本目录。"),
      sampleStamp(catalog.updatedAt), page));

  auto* metrics = new QHBoxLayout;
  metrics->setSpacing(14);
  metrics->addWidget(makeMetric(zh("近三月场次"), QString::number(catalog.matches.size()), zh("系列赛"), page));
  metrics->addWidget(makeMetric(zh("活跃战队"), QString::number(catalog.teams.size()), zh("样本池"), page));
  metrics->addWidget(makeMetric(zh("覆盖地图"), QString::number(cs2::activeMaps().size()), zh("现役地图池"), page));
  metrics->addWidget(makeMetric(zh("数据更新"), catalog.updatedAt.left(10), catalog.updatedAt.mid(11), page));
  shell.layout->addLayout(metrics);

  auto* middle = new QHBoxLayout;
  middle->setSpacing(14);

  Section matches = makeSection(zh("最近比赛结果"), zh("胜者在左"), page);
  auto* matchTable = makeTable({zh("日期"), zh("赛事"), zh("对阵"), zh("赛制"), zh("地图")}, matches.frame);
  const QVector<cs2::Match> recent = cs2::recentMatches(catalog);
  resetTable(matchTable, recent.size());
  for (int i = 0; i < recent.size(); ++i) {
    const cs2::Match& match = recent[i];
    writeCell(matchTable, i, 0, match.date);
    writeCell(matchTable, i, 1, match.eventName);
    writeCell(matchTable, i, 2, faceoff(catalog, match), Qt::AlignVCenter | Qt::AlignLeft, amberColor());
    writeCell(matchTable, i, 3, match.format);
    writeCell(matchTable, i, 4, cs2::mapLine(match));
  }
  lockTable(matchTable, 460);
  matches.layout->addWidget(matchTable);
  middle->addWidget(matches.frame, 3);

  Section form = makeSection(zh("战队近期状态"), zh("由系列赛汇总"), page);
  auto* formScroll = new QScrollArea(form.frame);
  formScroll->setWidgetResizable(true);
  formScroll->setFrameShape(QFrame::NoFrame);
  formScroll->setMinimumHeight(460);
  formScroll->setMaximumHeight(460);
  auto* formBody = new QWidget;
  auto* formLayout = new QVBoxLayout(formBody);
  formLayout->setContentsMargins(0, 0, 0, 0);
  formLayout->setSpacing(0);
  for (const cs2::Team& team : catalog.teams)
    formLayout->addWidget(formRow(team, formBody));
  formLayout->addStretch();
  formScroll->setWidget(formBody);
  form.layout->addWidget(formScroll);
  middle->addWidget(form.frame, 2);
  shell.layout->addLayout(middle);

  Section ranking = makeSection(zh("地图胜率排行"), zh("每张图取最高胜率队伍"), page);
  auto* rankTable = makeTable({zh("地图"), zh("战队"), zh("胜/场"), zh("胜率"), zh(""), zh("该图队伍场次")}, ranking.frame);
  const QVector<cs2::MapLeader> leaders = cs2::mapLeaders(catalog);
  resetTable(rankTable, leaders.size());
  for (int i = 0; i < leaders.size(); ++i) {
    const cs2::MapLeader& row = leaders[i];
    const double rate = row.played ? double(row.wins) / double(row.played) : 0;
    writeCell(rankTable, i, 0, row.map);
    writeCell(rankTable, i, 1, cs2::teamName(catalog, row.teamId));
    writeCell(rankTable, i, 2, QString::number(row.wins) + QLatin1Char('/') + QString::number(row.played), Qt::AlignCenter);
    writeCell(rankTable, i, 3, pct(rate), Qt::AlignCenter, amberColor());
    auto* bar = new WinBar(rankTable);
    bar->setValue(rate);
    rankTable->setCellWidget(i, 4, bar);
    writeCell(rankTable, i, 5, QString::number(row.pool), Qt::AlignCenter);
  }
  fitTable(rankTable);
  ranking.layout->addWidget(rankTable);
  shell.layout->addWidget(ranking.frame);
  shell.layout->addStretch();
  return page;
}
