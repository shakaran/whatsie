// Regression tests for the bundled UI assets: the icons and logos that the
// About and Rate-the-app screens rely on. These caught (and now guard against)
// the case where a button or logo silently renders blank because its icon was
// never set or its resource went missing.
#include <QtTest>
#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

#include "about.h"
#include "rateapp.h"

class TstUiAssets : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void resourcesExist_data();
  void resourcesExist();
  void rateAppLogoAndIcons();
  void aboutLogo();
  void aboutButtonIcons();
};

void TstUiAssets::initTestCase() {
  QCoreApplication::setOrganizationName(QStringLiteral("shakaran"));
  QCoreApplication::setApplicationName(QStringLiteral("whatly-test"));
}

// Every icon/logo path referenced from the code or the .ui files must resolve to
// a real, loadable resource.
void TstUiAssets::resourcesExist_data() {
  QTest::addColumn<QString>("path");
  const char *paths[] = {
      ":/icons/app/icon-16.png",  ":/icons/app/icon-32.png",
      ":/icons/app/icon-48.png",  ":/icons/app/icon-64.png",
      ":/icons/app/icon-128.png", ":/icons/app/icon-256.png",
      ":/icons/app/icon-512.png", ":/icons/texture.png",
      ":/icons/shopping-cart-line.png", ":/icons/star-line.png",
      ":/icons/paypal-line.png",  ":/icons/funds-line.png",
      ":/icons/time-line.png",    ":/icons/check-line.png",
      ":/icons/links-line.png",   ":/icons/file-text-line.png",
      ":/icons/error-warning-line.png", ":/icons/grid-line.png",
      ":/icons/terminal-box-line.png",  ":/icons/heart-line.png",
      ":/icons/github-line.png",
  };
  for (const char *p : paths)
    QTest::newRow(p) << QString::fromLatin1(p);
}

void TstUiAssets::resourcesExist() {
  QFETCH(QString, path);
  QVERIFY2(QFile(path).exists(), qPrintable("missing resource: " + path));
  QVERIFY2(!QPixmap(path).isNull(), qPrintable("pixmap failed to load: " + path));
}

void TstUiAssets::rateAppLogoAndIcons() {
  RateApp w(nullptr, QStringLiteral("snap://whatly"), 1, 1, 0);

  auto *logo = w.findChild<QLabel *>(QStringLiteral("logoLabel"));
  QVERIFY2(logo, "RateApp: logoLabel not found");
  QVERIFY2(!logo->pixmap().isNull(), "RateApp: logo pixmap is null");

  const QStringList btns = {"rateNowBtn", "rateOnGithub", "donate",
                            "donate_2",   "laterBtn",     "alreadyDoneBtn"};
  for (const QString &n : btns) {
    auto *b = w.findChild<QPushButton *>(n);
    QVERIFY2(b, qPrintable("RateApp: button not found: " + n));
    QVERIFY2(!b->icon().isNull(),
             qPrintable("RateApp: button has no icon: " + n));
  }
}

void TstUiAssets::aboutLogo() {
  About w;
  auto *logo = w.findChild<QLabel *>(QStringLiteral("label"));
  QVERIFY2(logo, "About: logo label not found");
  QVERIFY2(!logo->pixmap().isNull(), "About: logo pixmap is null");
}

void TstUiAssets::aboutButtonIcons() {
  About w;
  const QStringList btns = {"donate",      "kofi",       "wise",
                            "rate",        "more_apps",  "source_code",
                            "report_bug",  "debugInfoButton"};
  QStringList missing;
  for (const QString &n : btns) {
    auto *b = w.findChild<QPushButton *>(n);
    if (b && b->icon().isNull())
      missing << n;
  }
  QVERIFY2(missing.isEmpty(),
           qPrintable("About: buttons with no icon: " + missing.join(", ")));
}

QTEST_MAIN(TstUiAssets)
#include "tst_ui_assets.moc"
