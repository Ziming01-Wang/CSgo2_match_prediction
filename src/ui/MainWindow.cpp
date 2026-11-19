#include "ui/MainWindow.h"

#include "app/Session.h"
#include "common/Text.h"
#include "domain/Catalog.h"
#include "ui/pages/ArchivePage.h"
#include "ui/pages/HomePage.h"
#include "ui/pages/MajorPage.h"
#include "ui/pages/PredictPage.h"
#include "ui/pages/TeamPage.h"

#include <QButtonGroup>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), catalog_(cs2::sampleCatalog()) {
  setWindowTitle(zh("CS2 职业比赛预测"));
  resize(1400, 880);
  setMinimumSize(1240, 740);

  QPixmap icon(64, 64);
  icon.fill(Qt::transparent);
  QPainter painter(&icon);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(QStringLiteral("#e8a317")));
  painter.drawRoundedRect(2, 2, 60, 60, 14, 14);
  painter.setPen(QColor(QStringLiteral("#1a1206")));
  QFont markFont(QStringLiteral("Segoe UI"), 18, QFont::Bold);
  painter.setFont(markFont);
  painter.drawText(icon.rect(), Qt::AlignCenter, QStringLiteral("CS"));
  painter.end();
  setWindowIcon(QIcon(icon));

  session_ = new Session(this);

  auto* root = new QWidget(this);
  root->setObjectName(QStringLiteral("AppRoot"));
  root->setAttribute(Qt::WA_StyledBackground, true);
  setCentralWidget(root);

  auto* layout = new QHBoxLayout(root);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  auto* sidebar = new QFrame(root);
  sidebar->setObjectName(QStringLiteral("Sidebar"));
  sidebar->setFixedWidth(220);
  auto* side = new QVBoxLayout(sidebar);
  side->setContentsMargins(0, 18, 0, 12);
  side->setSpacing(2);

  auto* brand = new QHBoxLayout;
  brand->setContentsMargins(16, 0, 16, 12);
  brand->setSpacing(10);
  auto* mark = new QLabel(QStringLiteral("CS"), sidebar);
  mark->setObjectName(QStringLiteral("BrandMark"));
  mark->setAlignment(Qt::AlignCenter);
  mark->setFixedSize(42, 42);
  auto* names = new QVBoxLayout;
  names->setSpacing(0);
  auto* brandName = new QLabel(QStringLiteral("PREDICT"), sidebar);
  brandName->setObjectName(QStringLiteral("BrandName"));
  auto* brandSub = new QLabel(zh("职业比赛预测"), sidebar);
  brandSub->setObjectName(QStringLiteral("BrandSub"));
  names->addWidget(brandName);
  names->addWidget(brandSub);
  brand->addWidget(mark);
  brand->addLayout(names);
  side->addLayout(brand);

  stack_ = new QStackedWidget(root);
  stack_->setObjectName(QStringLiteral("Pages"));
  stack_->setAttribute(Qt::WA_StyledBackground, true);
  stack_->addWidget(buildHomePage(catalog_, stack_));
  stack_->addWidget(new TeamPage(catalog_, stack_));
  stack_->addWidget(new PredictPage(catalog_, session_, stack_));
  stack_->addWidget(new MajorPage(catalog_, stack_));
  stack_->addWidget(new ArchivePage(session_, catalog_.updatedAt, stack_));

  const QStringList groups = {zh("浏览"), zh("推演"), zh("记录")};
  const QStringList labels = {zh("比赛数据"), zh("战队分析"), zh("比赛预测"), zh("Major 模拟"), zh("预测记录")};
  const int groupAt[5] = {0, 0, 1, 1, 2};
  auto* nav = new QButtonGroup(this);
  nav->setExclusive(true);
  int seenGroup = -1;
  for (int i = 0; i < labels.size(); ++i) {
    if (groupAt[i] != seenGroup) {
      seenGroup = groupAt[i];
      auto* group = new QLabel(groups[seenGroup], sidebar);
      group->setObjectName(QStringLiteral("NavGroup"));
      side->addWidget(group);
    }
    auto* button = new QPushButton(labels[i], sidebar);
    button->setObjectName(QStringLiteral("NavButton"));
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    nav->addButton(button, i);
    side->addWidget(button);
    if (i == 4)
      archiveButton_ = button;
    if (i == 0)
      button->setChecked(true);
  }
  side->addStretch();
  auto* note = new QLabel(zh("数据源\n样本目录\n") + catalog_.updatedAt, sidebar);
  note->setObjectName(QStringLiteral("SideNote"));
  side->addWidget(note);

  layout->addWidget(sidebar);
  layout->addWidget(stack_, 1);

  connect(nav, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this](int id) { stack_->setCurrentIndex(id); });
  connect(session_, &Session::changed, this, [this] { refreshArchiveLabel(); });
  refreshArchiveLabel();
}

void MainWindow::refreshArchiveLabel() {
  if (!archiveButton_ || !session_)
    return;
  const int count = session_->records().size();
  archiveButton_->setText(count > 0 ? zh("预测记录   ") + QString::number(count) : zh("预测记录"));
}
