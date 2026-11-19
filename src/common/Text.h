#pragma once

#include <QString>

inline QString zh(const char* utf8) {
  return QString::fromUtf8(utf8);
}

inline QString pct(double ratio) {
  const double clamped = ratio < 0.0 ? 0.0 : (ratio > 1.0 ? 1.0 : ratio);
  return QString::number(clamped * 100.0, 'f', 1) + QLatin1String("%");
}
