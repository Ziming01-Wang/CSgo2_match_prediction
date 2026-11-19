#include "app/Session.h"

Session::Session(QObject* parent) : QObject(parent) {}

const QVector<cs2::PredictionView>& Session::records() const {
  return records_;
}

void Session::save(const cs2::PredictionView& view) {
  records_.prepend(view);
  emit changed();
}

void Session::removeAt(int index) {
  if (index < 0 || index >= records_.size())
    return;
  records_.removeAt(index);
  emit changed();
}

void Session::clear() {
  if (records_.isEmpty())
    return;
  records_.clear();
  emit changed();
}
