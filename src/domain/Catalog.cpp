#include "domain/Catalog.h"

#include <QHash>

#include <algorithm>
#include <initializer_list>

namespace cs2 {
namespace {

MapResult map(const char* name, int scoreA, int scoreB) {
  MapResult row;
  row.map = QString::fromLatin1(name);
  row.scoreA = scoreA;
  row.scoreB = scoreB;
  return row;
}

Match series(const char* date, const char* eventName, const char* teamA, const char* teamB,
             const char* format, std::initializer_list<MapResult> maps) {
  Match row;
  row.date = QString::fromLatin1(date);
  row.eventName = QString::fromLatin1(eventName);
  row.teamA = QString::fromLatin1(teamA);
  row.teamB = QString::fromLatin1(teamB);
  row.format = QString::fromLatin1(format);
  for (const MapResult& item : maps)
    row.maps.push_back(item);
  return row;
}

Team team(const char* id, const char* name, const char* region, int rank, double rating) {
  Team row;
  row.id = QString::fromLatin1(id);
  row.name = QString::fromLatin1(name);
  row.region = QString::fromLatin1(region);
  row.rank = rank;
  row.rating = rating;
  return row;
}

void bumpMap(QHash<QString, MapStat>& index, const QString& teamId, const QString& map, bool win) {
  const QString key = teamId + QLatin1Char('|') + map;
  MapStat& row = index[key];
  if (row.teamId.isEmpty()) {
    row.teamId = teamId;
    row.map = map;
  }
  ++row.played;
  if (win)
    ++row.wins;
}

void derive(Catalog& catalog) {
  QHash<QString, QVector<int>> byTeam;
  QHash<QString, MapStat> mapIndex;
  QHash<QString, HeadToHead> h2hIndex;

  for (int i = 0; i < catalog.matches.size(); ++i) {
    const Match& match = catalog.matches[i];
    byTeam[match.teamA].push_back(i);
    byTeam[match.teamB].push_back(i);

    const int scoreA = seriesScore(match, true);
    const int scoreB = seriesScore(match, false);
    for (const MapResult& item : match.maps) {
      bumpMap(mapIndex, match.teamA, item.map, item.scoreA > item.scoreB);
      bumpMap(mapIndex, match.teamB, item.map, item.scoreB > item.scoreA);
    }

    const bool aFirst = match.teamA < match.teamB;
    const QString key = (aFirst ? match.teamA : match.teamB) + QLatin1Char('|') + (aFirst ? match.teamB : match.teamA);
    HeadToHead& row = h2hIndex[key];
    if (row.teamA.isEmpty()) {
      row.teamA = aFirst ? match.teamA : match.teamB;
      row.teamB = aFirst ? match.teamB : match.teamA;
    }
    const bool storedA = row.teamA == match.teamA;
    const int left = storedA ? scoreA : scoreB;
    const int right = storedA ? scoreB : scoreA;
    if (left > right)
      ++row.winsA;
    else if (right > left)
      ++row.winsB;
    if (row.lastDate.isEmpty() || match.date >= row.lastDate) {
      row.lastDate = match.date;
      row.lastScore = QString::number(left) + QLatin1Char('-') + QString::number(right);
    }
  }

  for (int t = 0; t < catalog.teams.size(); ++t) {
    Team& team = catalog.teams[t];
    QVector<int> indexes = byTeam.value(team.id);
    std::sort(indexes.begin(), indexes.end(), [&catalog](int a, int b) {
      return catalog.matches[a].date > catalog.matches[b].date;
    });
    int wins = 0;
    QString form;
    const int shown = qMin(5, indexes.size());
    for (int i = 0; i < indexes.size(); ++i) {
      const bool won = winnerId(catalog.matches[indexes[i]]) == team.id;
      if (won)
        ++wins;
      if (i < shown)
        form += won ? QLatin1Char('W') : QLatin1Char('L');
    }
    team.seriesPlayed = indexes.size();
    team.seriesWins = wins;
    team.recentWinRate = indexes.isEmpty() ? 0.5 : double(wins) / double(indexes.size());
    team.form = form.isEmpty() ? QStringLiteral("-") : form;
  }

  catalog.mapStats = QVector<MapStat>::fromList(mapIndex.values());
  std::sort(catalog.mapStats.begin(), catalog.mapStats.end(), [](const MapStat& a, const MapStat& b) {
    if (a.teamId == b.teamId)
      return a.map < b.map;
    return a.teamId < b.teamId;
  });
  catalog.h2h = QVector<HeadToHead>::fromList(h2hIndex.values());
  std::sort(catalog.h2h.begin(), catalog.h2h.end(), [](const HeadToHead& a, const HeadToHead& b) {
    if (a.teamA == b.teamA)
      return a.teamB < b.teamB;
    return a.teamA < b.teamA;
  });
}

}  // namespace

QStringList activeMaps() {
  return QStringList() << QStringLiteral("Mirage") << QStringLiteral("Inferno") << QStringLiteral("Nuke")
                       << QStringLiteral("Ancient") << QStringLiteral("Anubis") << QStringLiteral("Dust2")
                       << QStringLiteral("Train");
}

Catalog sampleCatalog() {
  Catalog catalog;
  catalog.updatedAt = QStringLiteral("2026-11-18 21:00");
  catalog.windowLabel = QStringLiteral("2026-08-20") + QString::fromUtf8(" 至 ") + QStringLiteral("2026-11-18");

  catalog.teams = {
      team("vitality", "Vitality", "EU", 1, 93),     team("spirit", "Spirit", "EU", 2, 91),
      team("navi", "NAVI", "EU", 3, 88),             team("mouz", "MOUZ", "EU", 4, 86),
      team("faze", "FaZe", "EU", 5, 84),             team("g2", "G2", "EU", 6, 83),
      team("falcons", "Falcons", "MENA", 7, 81),     team("mongolz", "The MongolZ", "APAC", 8, 80),
      team("liquid", "Liquid", "NA", 9, 78),         team("furia", "FURIA", "SA", 10, 76),
      team("heroic", "Heroic", "EU", 11, 74),        team("astralis", "Astralis", "EU", 12, 72),
      team("3dmax", "3DMAX", "EU", 13, 71),          team("pain", "paiN", "SA", 14, 70),
      team("vp", "Virtus.pro", "EU", 15, 67),        team("col", "Complexity", "NA", 16, 65),
  };

  // 样本系列。地图比分是后面胜率、状态、交手的唯一来源。
  catalog.matches = {
      series("2026-11-16", "BLAST Premier Fall Final", "vitality", "spirit", "BO3",
             {map("Mirage", 13, 10), map("Inferno", 11, 13), map("Nuke", 13, 8)}),
      series("2026-11-15", "BLAST Premier Fall SF", "navi", "mouz", "BO3",
             {map("Nuke", 13, 7), map("Ancient", 13, 11)}),
      series("2026-11-15", "BLAST Premier Fall SF", "vitality", "faze", "BO3",
             {map("Inferno", 13, 8), map("Mirage", 10, 13), map("Nuke", 13, 6)}),
      series("2026-11-14", "BLAST Premier Fall QF", "spirit", "g2", "BO3",
             {map("Mirage", 13, 11), map("Anubis", 8, 13), map("Dust2", 13, 9)}),
      series("2026-11-14", "BLAST Premier Fall QF", "mouz", "falcons", "BO3",
             {map("Ancient", 13, 10), map("Mirage", 6, 13), map("Anubis", 13, 11)}),
      series("2026-11-11", "ESL Pro League", "g2", "liquid", "BO3", {map("Inferno", 13, 7), map("Mirage", 13, 9)}),
      series("2026-11-08", "IEM Fall", "mongolz", "heroic", "BO3",
             {map("Dust2", 13, 11), map("Nuke", 9, 13), map("Mirage", 13, 8)}),
      series("2026-11-05", "ESL Pro League", "3dmax", "pain", "BO3", {map("Anubis", 13, 10), map("Ancient", 13, 8)}),
      series("2026-11-03", "ESL Pro League", "faze", "navi", "BO3",
             {map("Dust2", 13, 9), map("Nuke", 11, 13), map("Mirage", 13, 7)}),
      series("2026-11-01", "ESL Pro League", "liquid", "col", "BO3",
             {map("Nuke", 13, 11), map("Inferno", 9, 13), map("Ancient", 13, 8)}),
      series("2026-10-26", "BLAST Premier Fall Group", "spirit", "vitality", "BO3",
             {map("Anubis", 13, 11), map("Inferno", 10, 13), map("Mirage", 13, 9)}),
      series("2026-10-18", "ESL Pro League", "furia", "pain", "BO3", {map("Mirage", 13, 8), map("Dust2", 13, 10)}),
      series("2026-10-15", "IEM Fall", "liquid", "heroic", "BO3", {map("Nuke", 13, 11), map("Inferno", 13, 9)}),
      series("2026-10-12", "IEM Fall", "vitality", "g2", "BO3", {map("Inferno", 13, 6), map("Mirage", 13, 11)}),
      series("2026-10-09", "CCT South America", "pain", "col", "BO1", {map("Dust2", 13, 6)}),
      series("2026-10-07", "BLAST Showdown", "astralis", "vp", "BO3", {map("Inferno", 13, 9), map("Train", 13, 6)}),
      series("2026-10-04", "ESL Pro League", "faze", "mouz", "BO3", {map("Dust2", 13, 11), map("Ancient", 13, 9)}),
      series("2026-10-01", "BLAST Premier Fall Group", "navi", "falcons", "BO3",
             {map("Nuke", 13, 5), map("Dust2", 13, 10)}),
      series("2026-09-30", "IEM Fall", "mongolz", "liquid", "BO3",
             {map("Dust2", 13, 8), map("Mirage", 10, 13), map("Ancient", 13, 11)}),
      series("2026-09-27", "Major Qualifier", "spirit", "vp", "BO3", {map("Mirage", 13, 5), map("Train", 13, 8)}),
      series("2026-09-23", "CCT South America", "furia", "col", "BO3", {map("Mirage", 13, 7), map("Dust2", 13, 11)}),
      series("2026-09-19", "ESL Pro League", "falcons", "astralis", "BO3",
             {map("Mirage", 13, 11), map("Inferno", 8, 13), map("Dust2", 13, 9)}),
      series("2026-09-14", "IEM Fall", "vitality", "navi", "BO3",
             {map("Inferno", 13, 10), map("Nuke", 8, 13), map("Mirage", 13, 11)}),
      series("2026-09-11", "ESL Pro League", "mouz", "g2", "BO3",
             {map("Ancient", 13, 8), map("Inferno", 11, 13), map("Anubis", 13, 9)}),
      series("2026-09-06", "IEM Fall", "mongolz", "furia", "BO3",
             {map("Dust2", 16, 14), map("Ancient", 11, 13), map("Mirage", 13, 10)}),
      series("2026-09-02", "ESL Pro League", "spirit", "faze", "BO3", {map("Mirage", 13, 8), map("Anubis", 13, 11)}),
      series("2026-08-29", "BLAST Showdown", "heroic", "astralis", "BO3", {map("Nuke", 13, 9), map("Inferno", 13, 11)}),
      series("2026-08-26", "ESL Pro League", "falcons", "furia", "BO3", {map("Mirage", 13, 9), map("Dust2", 13, 7)}),
      series("2026-08-22", "ESL Pro League", "navi", "g2", "BO3",
             {map("Nuke", 13, 8), map("Ancient", 10, 13), map("Mirage", 13, 6)}),
      series("2026-08-21", "BLAST Showdown", "3dmax", "vp", "BO3",
             {map("Anubis", 13, 9), map("Train", 11, 13), map("Ancient", 13, 7)}),
  };

  derive(catalog);
  return catalog;
}

const Team* findTeam(const Catalog& catalog, const QString& id) {
  for (const Team& team : catalog.teams) {
    if (team.id == id)
      return &team;
  }
  return nullptr;
}

QString teamName(const Catalog& catalog, const QString& id) {
  const Team* team = findTeam(catalog, id);
  return team ? team->name : id;
}

int seriesScore(const Match& match, bool sideA) {
  int score = 0;
  for (const MapResult& item : match.maps) {
    const bool aWon = item.scoreA > item.scoreB;
    if (sideA == aWon)
      ++score;
  }
  return score;
}

QString winnerId(const Match& match) {
  const int left = seriesScore(match, true);
  const int right = seriesScore(match, false);
  if (left == right)
    return QString();
  return left > right ? match.teamA : match.teamB;
}

QString mapLine(const Match& match) {
  QStringList parts;
  for (const MapResult& item : match.maps)
    parts << item.map + QLatin1Char(' ') + QString::number(item.scoreA) + QLatin1Char('-') + QString::number(item.scoreB);
  return parts.join(QString::fromUtf8(" · "));
}

QString opponentId(const Match& match, const QString& teamId) {
  return match.teamA == teamId ? match.teamB : match.teamA;
}

QVector<Match> recentMatches(const Catalog& catalog) {
  QVector<Match> rows = catalog.matches;
  std::sort(rows.begin(), rows.end(), [](const Match& a, const Match& b) { return a.date > b.date; });
  return rows;
}

QVector<Match> matchesFor(const Catalog& catalog, const QString& teamId) {
  QVector<Match> rows;
  for (const Match& match : catalog.matches) {
    if (match.teamA == teamId || match.teamB == teamId)
      rows.push_back(match);
  }
  std::sort(rows.begin(), rows.end(), [](const Match& a, const Match& b) { return a.date > b.date; });
  return rows;
}

QVector<MapStat> mapsFor(const Catalog& catalog, const QString& teamId) {
  QVector<MapStat> rows;
  for (const MapStat& row : catalog.mapStats) {
    if (row.teamId == teamId)
      rows.push_back(row);
  }
  std::sort(rows.begin(), rows.end(), [](const MapStat& a, const MapStat& b) {
    const double ra = a.played ? double(a.wins) / a.played : 0;
    const double rb = b.played ? double(b.wins) / b.played : 0;
    if (ra == rb)
      return a.played > b.played;
    return ra > rb;
  });
  return rows;
}

const MapStat* findMap(const Catalog& catalog, const QString& teamId, const QString& map) {
  for (const MapStat& row : catalog.mapStats) {
    if (row.teamId == teamId && row.map == map)
      return &row;
  }
  return nullptr;
}

QVector<HeadToHead> h2hFor(const Catalog& catalog, const QString& teamId) {
  QVector<HeadToHead> rows;
  for (const HeadToHead& row : catalog.h2h) {
    if (row.teamA == teamId || row.teamB == teamId)
      rows.push_back(row);
  }
  std::sort(rows.begin(), rows.end(), [](const HeadToHead& a, const HeadToHead& b) { return a.lastDate > b.lastDate; });
  return rows;
}

const HeadToHead* h2hBetween(const Catalog& catalog, const QString& a, const QString& b) {
  for (const HeadToHead& row : catalog.h2h) {
    if ((row.teamA == a && row.teamB == b) || (row.teamA == b && row.teamB == a))
      return &row;
  }
  return nullptr;
}

QVector<MapLeader> mapLeaders(const Catalog& catalog) {
  QVector<MapLeader> rows;
  for (const QString& map : activeMaps()) {
    MapLeader leader;
    leader.map = map;
    for (const MapStat& stat : catalog.mapStats) {
      if (stat.map != map || stat.played <= 0)
        continue;
      leader.pool += stat.played;
      const bool empty = leader.teamId.isEmpty();
      const double rate = double(stat.wins) / double(stat.played);
      const double best = leader.played ? double(leader.wins) / double(leader.played) : -1.0;
      const Team* next = findTeam(catalog, stat.teamId);
      const Team* current = findTeam(catalog, leader.teamId);
      const bool higherRate = rate > best;
      const bool sameRateMoreGames = rate == best && stat.played > leader.played;
      const bool sameSampleBetterRank = rate == best && stat.played == leader.played && next && current && next->rank < current->rank;
      if (empty || higherRate || sameRateMoreGames || sameSampleBetterRank) {
        leader.teamId = stat.teamId;
        leader.wins = stat.wins;
        leader.played = stat.played;
      }
    }
    rows.push_back(leader);
  }
  std::sort(rows.begin(), rows.end(), [](const MapLeader& a, const MapLeader& b) {
    const double ra = a.played ? double(a.wins) / double(a.played) : -1.0;
    const double rb = b.played ? double(b.wins) / double(b.played) : -1.0;
    return ra > rb;
  });
  return rows;
}

}  // namespace cs2
