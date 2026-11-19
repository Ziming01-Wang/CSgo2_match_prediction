#pragma once

#include <QString>
#include <QVector>

namespace cs2 {

// 一张图的比分。系列赛比分由各地图胜负汇总，不单独手填。
struct MapResult {
  QString map;
  int scoreA = 0;
  int scoreB = 0;
};

// 一场系列赛。teamA / teamB 是目录里的队伍 id，不是显示名。
struct Match {
  QString date;
  QString eventName;
  QString teamA;
  QString teamB;
  QString format;
  QVector<MapResult> maps;
};

// 队伍主档。recent* 与 form 由比赛列表汇总，rating 是样本强度，不是排名网站分数。
struct Team {
  QString id;
  QString name;
  QString region;
  int rank = 0;
  double rating = 0;
  double recentWinRate = 0.5;
  QString form;
  int seriesPlayed = 0;
  int seriesWins = 0;
};

struct MapStat {
  QString teamId;
  QString map;
  int played = 0;
  int wins = 0;
};

// teamA / teamB 按 id 字典序固定，避免同一对队伍存两行。
struct HeadToHead {
  QString teamA;
  QString teamB;
  int winsA = 0;
  int winsB = 0;
  QString lastDate;
  QString lastScore;
};

// 只读快照。页面不修改它。
struct Catalog {
  QString updatedAt;
  QString windowLabel;
  QVector<Team> teams;
  QVector<Match> matches;
  QVector<MapStat> mapStats;
  QVector<HeadToHead> h2h;
};

// 一条可展示的依据。towardA 是这一条单独看时对 A 的倾向，不是最终胜率。
struct Factor {
  QString name;
  QString detail;
  double towardA = 0.5;
};

struct MapOutlook {
  QString map;
  double winA = 0.5;
  int sampleA = 0;
  int sampleB = 0;
};

// 一种选图路径。BO1 时每张图一条；BO3 时是禁选之后的三图组合。
struct VetoCase {
  QString name;
  QString line;
  double seriesWinA = 0.5;
};

// 一次推演的完整视图。页面只渲染，不重新计算。
struct PredictionView {
  QString id;
  QString createdAt;
  QString teamA;
  QString teamB;
  QString teamAName;
  QString teamBName;
  QString format;
  double winA = 0.5;
  QString headline;
  QString vetoTitle;
  QVector<Factor> factors;
  QVector<MapOutlook> maps;
  QVector<VetoCase> vetoes;
  QString disclaimer;
};

struct SwissSlot {
  QString teamName;
  QString region;
  QString record;
  QString status;
  double advance = 0;
};

struct PlayoffTie {
  QString round;
  QString faceoff;
  QString lean;
  double winA = 0.5;
};

struct OddsRow {
  QString teamName;
  double probability = 0;
};

// 瑞士轮名次来自带噪声的强度；夺冠概率来自无噪声强度。两者故意分开。
struct MajorPreview {
  int seed = 1;
  QVector<SwissSlot> swiss;
  QVector<PlayoffTie> ties;
  QVector<OddsRow> champions;
};

}  // namespace cs2
