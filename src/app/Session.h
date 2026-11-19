#pragma once

#include "domain/Types.h"

#include <QObject>
#include <QVector>

class Session : public QObject {
  Q_OBJECT

 public:
  explicit Session(QObject* parent = nullptr);

  const QVector<cs2::PredictionView>& records() const;
  void save(const cs2::PredictionView& view);
  void removeAt(int index);
  void clear();

 signals:
  void changed();

 private:
  QVector<cs2::PredictionView> records_;
};
