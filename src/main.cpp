#include "ui/MainWindow.h"
#include "ui/Theme.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QApplication app(argc, argv);
  QApplication::setApplicationName(QString::fromUtf8("CS2 职业比赛预测"));
  applyTheme(app);

  MainWindow window;
  window.show();
  return app.exec();
}
