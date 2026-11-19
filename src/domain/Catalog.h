#pragma once

#include "domain/Types.h"

#include <QStringList>

namespace cs2 {

Catalog sampleCatalog();
QStringList activeMaps();

const Team* findTeam(const Catalog& catalog, const QString& id);
QString teamName(const Catalog& catalog, const QString& id);

int seriesScore(const Match& match, bool sideA);
QString winnerId(const Match& match);
QString mapLine(const Match& match);
QString opponentId(const Match& match, const QString& teamId);

QVector<Match> recentMatches(const Catalog& catalog);
QVector<Match> matchesFor(const Catalog& catalog, const QString& teamId);
QVector<MapStat> mapsFor(const Catalog& catalog, const QString& teamId);
const MapStat* findMap(const Catalog& catalog, const QString& teamId, const QString& map);
QVector<HeadToHead> h2hFor(const Catalog& catalog, const QString& teamId);
const HeadToHead* h2hBetween(const Catalog& catalog, const QString& a, const QString& b);

struct MapLeader {
  QString map;
  QString teamId;
  int wins = 0;
  int played = 0;
  int pool = 0;
};

QVector<MapLeader> mapLeaders(const Catalog& catalog);

}  // namespace cs2
