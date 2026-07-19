// Logic-level unit tests: the pure, headless parts of the app — helpers, the
// injected-script generators, the scheduled-message queue, sun calculations,
// identicons, palettes and dictionary resolution. No widgets, no WebEngine
// instances, no event loop, so it runs fast and offscreen.
#include <QtTest>
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

#include "utils.h"
#include "common.h"
#include "debuglog.h"
#include "appprofile.h"
#include "dictionaries.h"
#include "identicons.h"
#include "theme.h"
#include "scheduledmessages.h"
#include "sunclock.hpp"
#include "webfont.h"
#include "chattheme.h"
#include "mutedstatus.h"
#include "privacyblur.h"
#include "chatwallpaper.h"
#include "customcss.h"
#include "webtweaks.h"
#include "linkeddevicename.h"

// ─────────────────────────────────────────────────────────────────────────────
class TstUtils : public QObject {
  Q_OBJECT
private slots:
  void toCamelCase() {
    QCOMPARE(Utils::toCamelCase(QStringLiteral("hello world")),
             QStringLiteral("Hello World"));
    QCOMPARE(Utils::toCamelCase(QString()), QString());
  }
  void randomIds() {
    const QString a = Utils::generateRandomId(16);
    QCOMPARE(a.length(), 16);
    QVERIFY(a != Utils::generateRandomId(16)); // practically never equal
    QCOMPARE(Utils::genRand(8, true, false, false).length(), 8);
    const QString digits = Utils::genRand(20, false, false, true);
    for (const QChar &c : digits)
      QVERIFY(c.isDigit());
  }
  void secToDay() {
    const QString s = Utils::convertSectoDay(3661); // 1h 1m 1s
    QVERIFY(!s.isEmpty());
  }
  void xmlRoundTrip() {
    const QString raw = QStringLiteral("a<b>&\"'c");
    const QString enc = Utils::encodeXML(raw);
    QVERIFY(!enc.contains('<'));
    QCOMPARE(Utils::decodeXML(enc), raw);
  }
  void roundToOneDecimal() {
    QCOMPARE(Utils::RoundToOneDecimal(1.24f), 1.2f);
    QCOMPARE(Utils::RoundToOneDecimal(1.25f), 1.3f);
  }
  void phoneNumbers() {
    QVERIFY(Utils::isPhoneNumber(QStringLiteral("+34600123456")));
    QVERIFY(!Utils::isPhoneNumber(QStringLiteral("600123456"))); // no +
    QVERIFY(!Utils::isPhoneNumber(QStringLiteral("not a phone")));
    QVERIFY(!Utils::isPhoneNumber(QString()));
  }
  void desktopAndInstall() {
    QVERIFY(!Utils::detectDesktopEnvironment().isEmpty());
    Utils::getInstallType(); // may be empty for an uninstalled/native build
    QVERIFY(!Utils::appDebugInfo().isEmpty());
    const QString md = Utils::appDebugInfoMarkdown();
    QVERIFY(md.contains(QLatin1String("Version")));
    QVERIFY(!Utils::processMemoryInfo().isEmpty());
  }
  // The critical guard from issue #230: a cache delete must refuse any path that
  // could nuke something it shouldn't. None of these should ever delete.
  void deleteCacheRefusesDangerousPaths() {
    QVERIFY(!Utils::delete_cache(QString()));                 // empty
    QVERIFY(!Utils::delete_cache(QStringLiteral(".")));       // relative (cwd)
    QVERIFY(!Utils::delete_cache(QStringLiteral("relative/x")));
    QVERIFY(!Utils::delete_cache(QStringLiteral("/")));       // root
    QVERIFY(!Utils::delete_cache(QDir::homePath()));          // home
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstAppProfile : public QObject {
  Q_OBJECT
private slots:
  void defaults() {
    // Without initFromArgs the default profile is active and adds no suffix.
    QVERIFY(AppProfile::isDefault());
    QVERIFY(AppProfile::suffix().isEmpty());
    AppProfile::id();    // defined (may be empty for the default profile)
    AppProfile::label(); // defined
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstIdenticons : public QObject {
  Q_OBJECT
private slots:
  void letterTileSize() {
    const QPixmap p = Identicons::letterTile(QStringLiteral("Whatly"), QSize(128, 128));
    QVERIFY(!p.isNull());
    QCOMPARE(p.size(), QSize(128, 128));
  }
  void clipRRect() {
    QPixmap in(64, 64);
    in.fill(Qt::red);
    const QPixmap out = Identicons::clipRRect(in);
    QVERIFY(!out.isNull());
  }
  void colorCount() {
    QImage solid(10, 10, QImage::Format_ARGB32);
    solid.fill(Qt::blue);
    QCOMPARE(Identicons::colorCount(solid), quint32(1));
    solid.setPixelColor(0, 0, Qt::red);
    QCOMPARE(Identicons::colorCount(solid), quint32(2));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstTheme : public QObject {
  Q_OBJECT
private slots:
  void palettesDiffer() {
    const QPalette light = Theme::getLightPalette();
    const QPalette dark = Theme::getDarkPalette();
    QVERIFY(light.color(QPalette::Window) != dark.color(QPalette::Window));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstDictionaries : public QObject {
  Q_OBJECT
private slots:
  void apiDoesNotCrash() {
    // These depend on the install layout and the system locale, both of which
    // vary in a test/CI environment — so just exercise them without asserting a
    // particular value (a bare "C" locale legitimately yields no preference).
    Dictionaries::dictionaryPath();
    const QStringList all = Dictionaries::availableDictionaries();
    QVERIFY(all.size() >= 0);
    Dictionaries::preferredDictionary();
    Dictionaries::selectedDictionaries();
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstSunclock : public QObject {
  Q_OBJECT
private slots:
  void sunriseBeforeNoonBeforeSunset() {
    // Madrid, summer. Ordering must hold regardless of the absolute values.
    Sunclock sc(40.4168, -3.7038, 2);
    struct tm t = {};
    t.tm_year = 2021 - 1900;
    t.tm_mon = 5; // June
    t.tm_mday = 21;
    t.tm_hour = 12;
    const time_t date = timegm(&t);
    const time_t rise = sc.sunrise(date);
    const time_t noon = sc.solar_noon(date);
    const time_t set = sc.sunset(date);
    QVERIFY(rise < noon);
    QVERIFY(noon < set);
  }
  void irradianceInRange() {
    Sunclock sc(40.4168, -3.7038, 2);
    struct tm t = {};
    t.tm_year = 2021 - 1900;
    t.tm_mon = 5;
    t.tm_mday = 21;
    t.tm_hour = 12;
    const double v = sc.irradiance(timegm(&t));
    QVERIFY(v >= 0.0 && v <= 1.0);
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstScheduled : public QObject {
  Q_OBJECT
private slots:
  void addRemoveAndStatus() {
    ScheduledMessages sm;
    const int before = sm.entries().size();
    const QString id = sm.add(QStringLiteral("34600123456"),
                              QStringLiteral("Alice"),
                              QStringLiteral("hi there"),
                              QDateTime::currentDateTime().addSecs(3600));
    QVERIFY(!id.isEmpty());
    QCOMPARE(sm.entries().size(), before + 1);

    sm.reportResult(id, false, QStringLiteral("boom"));
    bool found = false;
    for (const auto &e : sm.entries())
      if (e.id == id) {
        QCOMPARE(e.status, ScheduledMessages::Status::Failed);
        QCOMPARE(e.error, QStringLiteral("boom"));
        found = true;
      }
    QVERIFY(found);

    sm.remove(id);
    for (const auto &e : sm.entries())
      QVERIFY(e.id != id);
  }
  void removeCompleted() {
    ScheduledMessages sm;
    const QString a = sm.add(QStringLiteral("34600000001"), QString(),
                             QStringLiteral("x"),
                             QDateTime::currentDateTime().addSecs(3600));
    sm.reportResult(a, true, QString()); // Sent
    sm.removeCompleted();
    for (const auto &e : sm.entries())
      QVERIFY(e.id != a);
  }
  void statusLabels() {
    QVERIFY(!ScheduledMessages::statusLabel(ScheduledMessages::Status::Pending).isEmpty());
    QVERIFY(!ScheduledMessages::statusLabel(ScheduledMessages::Status::Sent).isEmpty());
    QVERIFY(!ScheduledMessages::statusLabel(ScheduledMessages::Status::Failed).isEmpty());
  }
  void scripts() {
    QVERIFY(!ScheduledMessages::senderScriptSource().isEmpty());
    const QString job = ScheduledMessages::startJobScript(
        QStringLiteral("id1"), QStringLiteral("34600123456"),
        QStringLiteral("hello"));
    QVERIFY(job.contains(QLatin1String("34600123456")));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// The injected-script modules: scriptSource() must be non-trivial, and the
// settings-backed selection must round-trip.
class TstScripts : public QObject {
  Q_OBJECT
private slots:
  void webFont() {
    QVERIFY(!WebFont::families().isEmpty());
    WebFont::setCurrentFamily(QStringLiteral("DejaVu Sans"));
    QCOMPARE(WebFont::currentFamily(), QStringLiteral("DejaVu Sans"));
    QVERIFY(!WebFont::scriptSource().isEmpty());
    WebFont::setCurrentFamily(QString()); // back to WhatsApp default
    QVERIFY(WebFont::currentFamily().isEmpty());
  }
  void chatTheme() {
    const auto themes = ChatTheme::themes();
    QVERIFY(!themes.isEmpty());
    // setter/getter round-trip
    ChatTheme::setCurrentThemeId(themes.last().id);
    QCOMPARE(ChatTheme::currentThemeId(), themes.last().id);
    // The default theme is a no-op (empty script), but at least one real theme
    // must produce a stylesheet.
    bool anyScript = false;
    for (const auto &th : themes) {
      ChatTheme::setCurrentThemeId(th.id);
      if (!ChatTheme::scriptSource().isEmpty())
        anyScript = true;
    }
    QVERIFY(anyScript);
  }
  void mutedStatus() {
    MutedStatus::setEnabled(true);
    QVERIFY(MutedStatus::isEnabled());
    QVERIFY(!MutedStatus::scriptSource().isEmpty());
    MutedStatus::setEnabled(false);
    QVERIFY(!MutedStatus::isEnabled());
  }
  void privacyBlur() {
    QVERIFY(!PrivacyBlur::levels().isEmpty());
    const QString id = PrivacyBlur::levels().first().id;
    PrivacyBlur::setCurrentLevelId(id);
    QCOMPARE(PrivacyBlur::currentLevelId(), id);
    QVERIFY(!PrivacyBlur::scriptSource().isEmpty());
  }
  void otherScriptsNonEmpty() {
    QVERIFY(!ChatWallpaper::scriptSource().isEmpty());
    QVERIFY(!WebTweaks::scriptSource().isEmpty());
    QVERIFY(!LinkedDeviceName::scriptSource(QStringLiteral("Work")).isEmpty());
    // CustomCss without a file is inactive but the script must still be valid.
    CustomCss::scriptSource();
    QVERIFY(!CustomCss::isActive());
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstUtilsMore : public QObject {
  Q_OBJECT
private slots:
  void envVar() {
    qputenv("WHATLY_TEST_ENV", "hello");
    QCOMPARE(Utils::GetEnvironmentVar(QStringLiteral("WHATLY_TEST_ENV")),
             QStringLiteral("hello"));
  }
  void returnPath() {
    const QString p = Utils::returnPath(QStringLiteral("sub"),
                                        QDir::tempPath());
    QVERIFY(!p.isEmpty());
  }
  void refreshCacheSize() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile f(dir.filePath(QStringLiteral("a.bin")));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(QByteArray(2048, 'x'));
    f.close();
    const QString s = Utils::refreshCacheSize(dir.path());
    QVERIFY(!s.isEmpty());
  }
  void camelCaseVariants() {
    QCOMPARE(Utils::toCamelCase(QStringLiteral("one two three")),
             QStringLiteral("One Two Three"));
  }
  void installTypeFromEnv() {
    qputenv("INSTALL_TYPE", "snap");
    QCOMPARE(Utils::getInstallType(), QStringLiteral("snap"));
    qunsetenv("INSTALL_TYPE");
  }
  void secToDayComponents() {
    const QString s = Utils::convertSectoDay(90061); // 1d 1h 1m 1s
    QVERIFY(s.contains(QLatin1String("1 days")));
    QVERIFY(s.contains(QLatin1String("1 hours")));
  }
  void encodeAllSpecials() {
    const QString enc = Utils::encodeXML(QStringLiteral("<&>\"'"));
    QVERIFY(!enc.contains('<'));
    QVERIFY(!enc.contains('>'));
    QCOMPARE(Utils::decodeXML(enc), QStringLiteral("<&>\"'"));
  }
  void genRandCharsets() {
    const QString up = Utils::genRand(30, true, false, false);
    for (const QChar &c : up) QVERIFY(c.isUpper() || !c.isLetter());
    const QString lo = Utils::genRand(30, false, true, false);
    for (const QChar &c : lo) QVERIFY(c.isLower() || !c.isLetter());
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstCommon : public QObject {
  Q_OBJECT
private slots:
  void themeIconFallback() {
    const QIcon icon = themeIcon(QStringLiteral("whatly"),
                                 QStringLiteral(":/icons/app/icon-64.png"));
    QVERIFY(!icon.isNull());
    QVERIFY(!whatsAppOrigin.isEmpty());
    QVERIFY(!defaultUserAgentStr.isEmpty());
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstDebugLog : public QObject {
  Q_OBJECT
private slots:
  void captureAndFilter() {
    DebugLog::install(); // idempotent
    qWarning("whatly-test-marker-visible");
    // Benign teardown noise is still captured in the log, just not printed.
    qWarning("QThreadStorage: entry 7 destroyed before end of thread");
    const QString recent = DebugLog::recent(50);
    QVERIFY(recent.contains(QLatin1String("whatly-test-marker-visible")));
    QVERIFY(recent.contains(QLatin1String("QThreadStorage: entry 7")));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstAppProfileArgs : public QObject {
  Q_OBJECT
private slots:
  void namedProfileThenDefault() {
    // A named profile adds a suffix and is not the default.
    char arg0[] = "whatly";
    char arg1[] = "--profile=work";
    char *argv[] = {arg0, arg1, nullptr};
    // initFromArgs settles the profile once, for the lifetime of the process
    // (as in the real app), so this runs last and does not reset afterwards.
    AppProfile::initFromArgs(2, argv);
    QVERIFY(!AppProfile::isDefault());
    QVERIFY(!AppProfile::suffix().isEmpty());
    QVERIFY(AppProfile::id().contains(QLatin1String("work")));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstCustomCss : public QObject {
  Q_OBJECT
private slots:
  void loadAndClear() {
    QTemporaryFile css;
    css.setFileTemplate(QDir::tempPath() + QStringLiteral("/whatly_XXXXXX.css"));
    QVERIFY(css.open());
    css.write("body { background: #123456; }\n");
    css.close();

    QString err;
    QVERIFY2(CustomCss::setFromFile(css.fileName(), &err), qPrintable(err));
    QVERIFY(CustomCss::isActive());
    QVERIFY(CustomCss::css().contains(QLatin1String("#123456")));
    QVERIFY(!CustomCss::scriptSource().isEmpty());

    CustomCss::clear();
    QVERIFY(!CustomCss::isActive());
  }
  void rejectsMissingFile() {
    QString err;
    QVERIFY(!CustomCss::setFromFile(QStringLiteral("/no/such.css"), &err));
    QVERIFY(!err.isEmpty());
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstChatWallpaper : public QObject {
  Q_OBJECT
private slots:
  void setStoreClear() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString src = dir.filePath(QStringLiteral("wp.png"));
    QImage img(32, 32, QImage::Format_ARGB32);
    img.fill(Qt::darkCyan);
    QVERIFY(img.save(src));

    QString err;
    QVERIFY2(ChatWallpaper::setImage(src, &err), qPrintable(err));
    QVERIFY(!ChatWallpaper::storedImagePath().isEmpty());
    QVERIFY(!ChatWallpaper::scriptSource().isEmpty());

    ChatWallpaper::clear();
    QVERIFY(ChatWallpaper::storedImagePath().isEmpty());
  }
};

// ─────────────────────────────────────────────────────────────────────────────
class TstScheduledPersistence : public QObject {
  Q_OBJECT
private slots:
  void survivesReload() {
    QString id;
    {
      ScheduledMessages sm;
      id = sm.add(QStringLiteral("34600999999"), QStringLiteral("Persist"),
                  QStringLiteral("saved to disk"),
                  QDateTime::currentDateTime().addSecs(7200));
      QVERIFY(!id.isEmpty());
    }
    // A fresh instance loads the queue back from disk.
    ScheduledMessages sm2;
    bool found = false;
    for (const auto &e : sm2.entries())
      if (e.id == id) {
        QCOMPARE(e.name, QStringLiteral("Persist"));
        found = true;
      }
    QVERIFY(found);
    sm2.remove(id); // clean up
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// install() paths: give each injected-script module a real (headless) profile.
class TstScriptInstall : public QObject {
  Q_OBJECT
private slots:
  void installOnProfile() {
    QWebEngineProfile profile(QStringLiteral("whatly-test-profile"));
    const int before = profile.scripts()->count();
    WebFont::install(&profile);
    ChatTheme::install(&profile);
    MutedStatus::install(&profile);
    PrivacyBlur::install(&profile);
    ChatWallpaper::install(&profile);
    CustomCss::install(&profile);
    WebTweaks::install(&profile);
    LinkedDeviceName::install(&profile, QStringLiteral("Work"));
    // Each module injects at least one script, so the collection grew.
    QVERIFY(profile.scripts()->count() > before);
  }
};

int main(int argc, char *argv[]) {
  // Keep the (headless) QWebEngineProfile used by the install() test happy on CI
  // runners: no sandbox, no GPU. Must be set before QApplication.
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox --disable-gpu");
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("shakaran"));
  QCoreApplication::setApplicationName(QStringLiteral("whatly-test"));
  // Keep test settings out of the real config.
  QStandardPaths::setTestModeEnabled(true);

  int status = 0;
  auto run = [&](QObject *obj) { status |= QTest::qExec(obj, argc, argv); };
  { TstUtils t;               run(&t); }
  { TstUtilsMore t;           run(&t); }
  { TstCommon t;              run(&t); }
  { TstDebugLog t;            run(&t); }
  { TstIdenticons t;          run(&t); }
  { TstTheme t;               run(&t); }
  { TstDictionaries t;        run(&t); }
  { TstSunclock t;            run(&t); }
  { TstScheduled t;           run(&t); }
  { TstScheduledPersistence t; run(&t); }
  { TstScripts t;             run(&t); }
  { TstCustomCss t;           run(&t); }
  { TstChatWallpaper t;       run(&t); }
  { TstScriptInstall t;       run(&t); }
  // Profile-mutating test runs last so it doesn't disturb the others.
  { TstAppProfile t;          run(&t); }
  { TstAppProfileArgs t;      run(&t); }
  return status;
}

#include "tst_logic.moc"
