#include "domain/Preview.h"

#include "common/Text.h"
#include "domain/Catalog.h"

#include <QHash>

#include <algorithm>
#include <cmath>

namespace cs2 {
namespace {

double clampRate(double value) {
  return qBound(0.08, value, 0.92);
}

double shrink(double raw, int sample) {
  const double weight = double(qMax(0, sample)) / double(qMax(0, sample) + 6);
  return 0.5 + (raw - 0.5) * weight;
}

double rateOf(const MapStat* stat, double fallback, int* played) {
  if (!stat || stat->played <= 0) {
    if (played)
      *played = 0;
    return fallback;
  }
  if (played)
    *played = stat->played;
  return double(stat->wins) / double(stat->played);
}

double ratingPart(const Team& a, const Team& b) {
  return clampRate(0.5 + (a.rating - b.rating) / 80.0);
}

double mapEdge(const Catalog& catalog, const Team& a, const Team& b, const QString& map, int* sampleA, int* sampleB) {
  int playedA = 0;
  int playedB = 0;
  const double rawA = rateOf(findMap(catalog, a.id, map), a.recentWinRate, &playedA);
  const double rawB = rateOf(findMap(catalog, b.id, map), b.recentWinRate, &playedB);
  const double denom = rawA + rawB;
  const double edge = denom <= 0.0 ? 0.5 : rawA / denom;
  const double shrunk = shrink(edge, qMin(playedA, playedB));
  if (sampleA)
    *sampleA = playedA;
  if (sampleB)
    *sampleB = playedB;
  return clampRate(0.85 * shrunk + 0.15 * ratingPart(a, b));
}

double bo3(double mapP) {
  const double p = qBound(0.0, mapP, 1.0);
  return p * p * (3.0 - 2.0 * p);
}

QString flippedScore(const QString& score) {
  const int dash = score.indexOf(QLatin1Char('-'));
  if (dash < 0)
    return score;
  return score.mid(dash + 1) + QLatin1Char('-') + score.left(dash);
}

int stableHash(const QString& text) {
  quint32 hash = 2166136261u;
  const QByteArray bytes = text.toUtf8();
  for (char c : bytes) {
    hash ^= quint32(uchar(c));
    hash *= 16777619u;
  }
  return int(hash & 0x7fffffff);
}

struct Side {
  QString name;
  double base = 0;
  double noisy = 0;
};

QString recordFor(int rank) {
  if (rank < 2)
    return QStringLiteral("3-0");
  if (rank < 5)
    return QStringLiteral("3-1");
  if (rank < 8)
    return QStringLiteral("3-2");
  if (rank < 11)
    return QStringLiteral("2-3");
  if (rank < 14)
    return QStringLiteral("1-3");
  return QStringLiteral("0-3");
}

}  // namespace

PredictionView previewMatch(const Catalog& catalog, const QString& teamA, const QString& teamB, const QString& format) {
  PredictionView view;
  const Team* a = findTeam(catalog, teamA);
  const Team* b = findTeam(catalog, teamB);
  if (!a || !b || a->id == b->id)
    return view;

  view.teamA = a->id;
  view.teamB = b->id;
  view.teamAName = a->name;
  view.teamBName = b->name;
  view.format = format;
  view.disclaimer = zh("百分比只用于把样本摆进界面。接入比赛数据前，不要把它当成预测模型。");

  const double formEdge = shrink(a->recentWinRate / qMax(0.01, a->recentWinRate + b->recentWinRate),
                                 qMin(a->seriesPlayed, b->seriesPlayed));
  const double strength = ratingPart(*a, *b);

  double mapSum = 0;
  const QStringList maps = activeMaps();
  for (const QString& map : maps) {
    MapOutlook outlook;
    outlook.map = map;
    outlook.winA = mapEdge(catalog, *a, *b, map, &outlook.sampleA, &outlook.sampleB);
    view.maps.push_back(outlook);
    mapSum += outlook.winA;
  }
  const double mapMean = view.maps.isEmpty() ? 0.5 : mapSum / double(view.maps.size());

  const HeadToHead* h2h = h2hBetween(catalog, a->id, b->id);
  double h2hEdge = 0.5;
  int h2hSample = 0;
  QString h2hDetail = zh("这个窗口里没有交手，这一项保持 50%，不参与加权。");
  if (h2h) {
    const bool aIsLeft = h2h->teamA == a->id;
    const int wins = aIsLeft ? h2h->winsA : h2h->winsB;
    const int losses = aIsLeft ? h2h->winsB : h2h->winsA;
    h2hSample = wins + losses;
    h2hEdge = h2hSample ? shrink(double(wins) / double(h2hSample), h2hSample) : 0.5;
    const QString last = aIsLeft ? h2h->lastScore : flippedScore(h2h->lastScore);
    h2hDetail = zh("窗口内 ") + QString::number(h2hSample) + zh(" 场，") + a->name + QLatin1Char(' ') +
                QString::number(wins) + QLatin1Char('-') + QString::number(losses) + zh("。最近一场 ") + last +
                zh("。样本少，权重 10%。");
  }

  const bool hasH2h = h2hSample > 0;
  const double blended = hasH2h ? 0.40 * formEdge + 0.30 * mapMean + 0.20 * strength + 0.10 * h2hEdge
                                : 0.45 * formEdge + 0.35 * mapMean + 0.20 * strength;
  view.winA = format == QLatin1String("BO1") ? clampRate(blended) : clampRate(bo3(blended));

  if (qAbs(view.winA - 0.5) < 0.03)
    view.headline = zh("双方接近。上方是系列胜率，下面的选图路径是另一组数。");
  else
    view.headline = zh("样本推演略倾向 ") + (view.winA >= 0.5 ? a->name : b->name) +
                    zh("。上方系列胜率已按赛制换算；选图路径只看该路径里的图。");

  Factor form;
  form.name = zh("近期状态");
  form.towardA = formEdge;
  form.detail = a->name + QLatin1Char(' ') + pct(a->recentWinRate) + zh("（") + QString::number(a->seriesWins) +
                QLatin1Char('/') + QString::number(a->seriesPlayed) + zh("），") + b->name + QLatin1Char(' ') +
                pct(b->recentWinRate) + zh("（") + QString::number(b->seriesWins) + QLatin1Char('/') +
                QString::number(b->seriesPlayed) + zh("）。权重 ") + (hasH2h ? QStringLiteral("40%") : QStringLiteral("45%")) +
                zh("。");
  view.factors.push_back(form);

  Factor mapsFactor;
  mapsFactor.name = zh("地图样本");
  mapsFactor.towardA = mapMean;
  mapsFactor.detail = zh("七张现役地图各自换算后取平均。权重 ") + (hasH2h ? QStringLiteral("30%") : QStringLiteral("35%")) +
                      zh("。");
  view.factors.push_back(mapsFactor);

  Factor rating;
  rating.name = zh("样本强度");
  rating.towardA = strength;
  rating.detail = a->name + QLatin1Char(' ') + QString::number(a->rating, 'f', 0) + zh(" 对 ") + b->name +
                  QLatin1Char(' ') + QString::number(b->rating, 'f', 0) + zh("。手填强度，不是实时排名。权重 20%。");
  view.factors.push_back(rating);

  Factor history;
  history.name = zh("历史交手");
  history.towardA = h2hEdge;
  history.detail = h2hDetail;
  view.factors.push_back(history);

  if (format == QLatin1String("BO1")) {
    view.vetoTitle = zh("单图决胜");
    for (const MapOutlook& outlook : view.maps) {
      VetoCase item;
      item.name = outlook.map;
      item.line = a->name + QLatin1Char(' ') + pct(outlook.winA) + zh("  ·  ") + b->name + QLatin1Char(' ') +
                  pct(1.0 - outlook.winA);
      item.seriesWinA = outlook.winA;
      view.vetoes.push_back(item);
    }
    std::sort(view.vetoes.begin(), view.vetoes.end(), [](const VetoCase& lhs, const VetoCase& rhs) {
      return lhs.seriesWinA > rhs.seriesWinA;
    });
    return view;
  }

  view.vetoTitle = zh("三图路径");
  QVector<int> order;
  for (int i = 0; i < view.maps.size(); ++i)
    order.push_back(i);
  std::sort(order.begin(), order.end(), [&view](int lhs, int rhs) { return view.maps[lhs].winA < view.maps[rhs].winA; });
  if (order.size() < 7)
    return view;

  const auto mapName = [&view, &order](int rank) { return view.maps[order[rank]].map; };
  const auto seriesOf = [&view, &order](int pickA, int pickB, int decider) {
    const double mean = (view.maps[order[pickA]].winA + view.maps[order[pickB]].winA + view.maps[order[decider]].winA) / 3.0;
    return clampRate(bo3(mean));
  };
  const auto join = [](const QStringList& parts) { return parts.join(QString::fromUtf8(" · ")); };

  VetoCase standard;
  standard.name = zh("常规禁选");
  standard.seriesWinA = seriesOf(4, 2, 3);
  standard.line = join({a->name + zh(" 禁 ") + mapName(0), b->name + zh(" 禁 ") + mapName(6),
                        a->name + zh(" 选 ") + mapName(4), b->name + zh(" 选 ") + mapName(2),
                        a->name + zh(" 禁 ") + mapName(1), b->name + zh(" 禁 ") + mapName(5), zh("决胜 ") + mapName(3)});
  view.vetoes.push_back(standard);

  VetoCase leave;
  leave.name = zh("A 先留弱图");
  leave.seriesWinA = seriesOf(5, 0, 3);
  leave.line = join({a->name + zh(" 禁 ") + mapName(1), b->name + zh(" 禁 ") + mapName(6),
                     a->name + zh(" 选 ") + mapName(5), b->name + zh(" 选 ") + mapName(0),
                     a->name + zh(" 禁 ") + mapName(2), b->name + zh(" 禁 ") + mapName(4), zh("决胜 ") + mapName(3)});
  view.vetoes.push_back(leave);

  VetoCase keep;
  keep.name = zh("B 留强图");
  keep.seriesWinA = seriesOf(6, 1, 5);
  keep.line = join({a->name + zh(" 禁 ") + mapName(0), b->name + zh(" 禁 ") + mapName(4),
                    a->name + zh(" 选 ") + mapName(6), b->name + zh(" 选 ") + mapName(1),
                    a->name + zh(" 禁 ") + mapName(2), b->name + zh(" 禁 ") + mapName(3), zh("决胜 ") + mapName(5)});
  view.vetoes.push_back(keep);
  return view;
}

MajorPreview previewMajor(const Catalog& catalog, int seed) {
  MajorPreview preview;
  preview.seed = seed;

  QVector<Side> base;
  QVector<Side> noisy;
  for (const Team& team : catalog.teams) {
    Side side;
    side.name = team.name;
    side.base = team.rating * (0.75 + 0.25 * team.recentWinRate);
    const double unit = (stableHash(team.id + QString::number(seed)) % 1000) / 1000.0;
    side.noisy = side.base + (unit - 0.5) * 6.0;
    base.push_back(side);
    noisy.push_back(side);
  }
  std::sort(base.begin(), base.end(), [](const Side& a, const Side& b) { return a.base > b.base; });
  std::sort(noisy.begin(), noisy.end(), [](const Side& a, const Side& b) { return a.noisy > b.noisy; });

  const double cutoff = base.size() > 7 ? base[7].base : 0;
  QHash<QString, double> advanceOf;
  QHash<QString, QString> regionOf;
  for (const Team& team : catalog.teams)
    regionOf.insert(team.name, team.region);
  for (const Side& side : base)
    advanceOf.insert(side.name, 1.0 / (1.0 + std::exp(-(side.base - cutoff) / 4.0)));

  for (int i = 0; i < noisy.size(); ++i) {
    SwissSlot slot;
    slot.teamName = noisy[i].name;
    slot.region = regionOf.value(slot.teamName);
    slot.record = recordFor(i);
    slot.status = i < 8 ? zh("晋级") : zh("淘汰");
    slot.advance = advanceOf.value(slot.teamName, 0.5);
    preview.swiss.push_back(slot);
  }

  QVector<Side> seeds;
  for (int i = 0; i < qMin(8, noisy.size()); ++i)
    seeds.push_back(noisy[i]);

  auto tie = [](const QString& round, const Side& a, const Side& b) {
    PlayoffTie row;
    row.round = round;
    const double sum = a.base + b.base;
    row.winA = sum <= 0 ? 0.5 : a.base / sum;
    row.lean = a.base >= b.base ? a.name : b.name;
    row.faceoff = a.name + QString::fromUtf8("  vs  ") + b.name;
    return row;
  };

  if (seeds.size() == 8) {
    const int pairs[4][2] = {{0, 7}, {3, 4}, {1, 6}, {2, 5}};
    Side winners[4];
    for (int i = 0; i < 4; ++i) {
      const Side& left = seeds[pairs[i][0]];
      const Side& right = seeds[pairs[i][1]];
      preview.ties.push_back(tie(zh("四分之一决赛"), left, right));
      winners[i] = left.base >= right.base ? left : right;
    }
    Side semis[2];
    for (int i = 0; i < 2; ++i) {
      preview.ties.push_back(tie(zh("半决赛"), winners[i * 2], winners[i * 2 + 1]));
      semis[i] = winners[i * 2].base >= winners[i * 2 + 1].base ? winners[i * 2] : winners[i * 2 + 1];
    }
    preview.ties.push_back(tie(zh("决赛"), semis[0], semis[1]));
  }

  double total = 0;
  for (const Side& side : seeds)
    total += side.base * side.base;
  for (const Side& side : seeds) {
    OddsRow row;
    row.teamName = side.name;
    row.probability = total <= 0 ? 0 : (side.base * side.base) / total;
    preview.champions.push_back(row);
  }
  std::sort(preview.champions.begin(), preview.champions.end(), [](const OddsRow& a, const OddsRow& b) {
    return a.probability > b.probability;
  });
  return preview;
}

}  // namespace cs2
