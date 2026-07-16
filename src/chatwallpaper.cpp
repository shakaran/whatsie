#include "chatwallpaper.h"
#include "settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QStandardPaths>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatly-chat-wallpaper";
static const char kStyleId[] = "whatly-chat-wallpaper";
static const char kSettingsKey[] = "chatWallpaper";

// Big enough to stay sharp on a 4K panel, small enough that the base64 string
// stays in the hundreds of kilobytes rather than the megabytes.
static const int kMaxEdge = 1920;
static const int kJpegQuality = 82;

// __CSS__ becomes a JSON string: the whole rule, or "" to remove the wallpaper.
// Re-running this on a loaded page swaps the image without a reload, so the
// same source serves both the profile script and the live update from Settings.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  try {
    var CSS = __CSS__;
    var el = document.getElementById('__STYLE_ID__');
    if (!CSS) {
      if (el) el.remove();
      return;
    }
    if (!el) {
      el = document.createElement('style');
      el.id = '__STYLE_ID__';
      (document.head || document.documentElement).appendChild(el);
    }
    el.textContent = CSS;
  } catch (e) { /* never break the page */ }
})();
)JS";

namespace {

QString storageDir() {
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString imagePath() { return storageDir() + QStringLiteral("/chat-wallpaper.jpg"); }

// The image as a data: URI, or empty when no wallpaper is set. WhatsApp's
// content-security policy allows data: URIs in stylesheets, which is why the
// image travels inline rather than over a custom scheme.
QString dataUri() {
  const QString path = ChatWallpaper::storedImagePath();
  if (path.isEmpty())
    return QString();

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return QString();

  return QStringLiteral("data:image/jpeg;base64,") +
         QString::fromLatin1(file.readAll().toBase64());
}

QString cssRule() {
  const QString uri = dataUri();
  if (uri.isEmpty())
    return QString();

  // The doodle overlay WhatsApp paints over #main is a translucent white wash;
  // it is left alone, as it only tints the image very slightly and its class
  // name is obfuscated.
  return QStringLiteral("#main{background-image:url(\"%1\")!important;"
                        "background-size:cover!important;"
                        "background-position:center center!important;"
                        "background-repeat:no-repeat!important;}")
      .arg(uri);
}

// A JS double-quoted string literal. Base64 and the CSS around it contain no
// characters needing escapes beyond the quote and backslash, but escape them
// anyway rather than trusting that.
QString jsStringLiteral(const QString &value) {
  QString escaped = value;
  escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
  escaped.replace(QLatin1Char('"'), QLatin1String("\\\""));
  return QLatin1Char('"') + escaped + QLatin1Char('"');
}

} // namespace

namespace ChatWallpaper {

QString storedImagePath() {
  if (!SettingsManager::instance()
           .settings()
           .value(QLatin1String(kSettingsKey))
           .toBool())
    return QString();

  const QString path = imagePath();
  return QFile::exists(path) ? path : QString();
}

QString scriptSource() {
  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__CSS__"), jsStringLiteral(cssRule()));
  source.replace(QLatin1String("__STYLE_ID__"), QLatin1String(kStyleId));
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  if (storedImagePath().isEmpty())
    return; // no wallpaper → nothing to inject on fresh loads

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

bool setImage(const QString &sourcePath, QString *error) {
  QImageReader reader(sourcePath);
  reader.setAutoTransform(true); // honour the EXIF orientation of a photo
  QImage image = reader.read();
  if (image.isNull()) {
    if (error)
      *error = reader.errorString();
    return false;
  }

  if (image.width() > kMaxEdge || image.height() > kMaxEdge)
    image = image.scaled(kMaxEdge, kMaxEdge, Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);

  if (!QDir().mkpath(storageDir())) {
    if (error)
      *error = QObject::tr("Cannot create %1").arg(storageDir());
    return false;
  }

  if (!image.save(imagePath(), "JPEG", kJpegQuality)) {
    if (error)
      *error = QObject::tr("Cannot write %1").arg(imagePath());
    return false;
  }

  SettingsManager::instance().settings().setValue(QLatin1String(kSettingsKey),
                                                  true);
  return true;
}

void clear() {
  QFile::remove(imagePath());
  SettingsManager::instance().settings().setValue(QLatin1String(kSettingsKey),
                                                  false);
}

} // namespace ChatWallpaper
