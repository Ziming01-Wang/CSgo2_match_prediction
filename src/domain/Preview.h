#pragma once

#include "domain/Types.h"

namespace cs2 {

// 用目录里已经汇总好的数字做可解释换算。这里不是预测模型。
PredictionView previewMatch(const Catalog& catalog, const QString& teamA, const QString& teamB, const QString& format);
MajorPreview previewMajor(const Catalog& catalog, int seed);

}  // namespace cs2
