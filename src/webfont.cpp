#include "webfont.h"
#include "settingsmanager.h"

#include <QFontDatabase>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatly-web-font";
static const char kSettingsKey[] = "webFontFamily";

// Same shape as PrivacyBlur: an idempotent injector that adds, updates or
// removes a single <style> element, gated on a value passed in as a JS literal.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  try {
    var CSS = __CSS__;
    var el = document.getElementById('whatly-web-font');
    if (!CSS) { if (el) el.remove(); return; }
    if (!el) {
      el = document.createElement('style');
      el.id = 'whatly-web-font';
      (document.head || document.documentElement).appendChild(el);
    }
    el.textContent = CSS;
  } catch (e) { /* never break the page */ }
})();
)JS";

namespace {

// A CSS string literal wrapped in double quotes.
QString cssStringLiteral(const QString &value) {
  QString escaped = value;
  escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
  escaped.replace(QLatin1Char('"'), QLatin1String("\\\""));
  return QLatin1Char('"') + escaped + QLatin1Char('"');
}

// The chosen family as a JS double-quoted literal (so it reaches the page
// intact), or "" when nothing is set.
QString jsCssFor(const QString &family) {
  if (family.isEmpty())
    return QStringLiteral("\"\"");

  // Skip [data-icon] (WhatsApp's SVG-icon spans) and code/pre (its monospace
  // message formatting), so those keep their intended glyphs; everything else
  // inherits the chosen family with the platform sans-serif as a fallback.
  const QString css =
      QStringLiteral("#app,#app :not([data-icon]):not(code):not(pre){"
                     "font-family:%1,\"Segoe UI\",Helvetica,Arial,"
                     "sans-serif !important}")
          .arg(cssStringLiteral(family));

  QString e = css;
  e.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
  e.replace(QLatin1Char('"'), QLatin1String("\\\""));
  return QLatin1Char('"') + e + QLatin1Char('"');
}

} // namespace

namespace WebFont {

QStringList families() {
  QStringList list;
  list << QString(); // "WhatsApp default"
  list << QFontDatabase::families();
  return list;
}

QString currentFamily() {
  return SettingsManager::instance()
      .settings()
      .value(QLatin1String(kSettingsKey))
      .toString();
}

void setCurrentFamily(const QString &family) {
  SettingsManager::instance().settings().setValue(QLatin1String(kSettingsKey),
                                                  family);
}

QString scriptSource() {
  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__CSS__"), jsCssFor(currentFamily()));
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  if (currentFamily().isEmpty())
    return;

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace WebFont
