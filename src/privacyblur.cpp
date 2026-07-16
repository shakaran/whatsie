#include "privacyblur.h"
#include "settingsmanager.h"

#include <QCoreApplication>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatly-privacy-blur";
static const char kSettingsKey[] = "privacyBlur";

// Plain CSS in an injected <style>: no scripting, nothing to keep in sync with
// WhatsApp's React tree, and a rule that stops matching simply stops blurring
// rather than breaking anything.
//
// The selectors are structural on purpose. Every class in WhatsApp Web is
// compiler-generated (.x10tvbhy and the like) and changes with each deploy;
// #pane-side, #main and the ARIA roles have been stable for years.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  try {
    var CSS = __CSS__;
    var el = document.getElementById('whatly-privacy-blur');
    if (!CSS) { if (el) el.remove(); return; }
    if (!el) {
      el = document.createElement('style');
      el.id = 'whatly-privacy-blur';
      (document.head || document.documentElement).appendChild(el);
    }
    el.textContent = CSS;
  } catch (e) { /* never break the page */ }
})();
)JS";

namespace {

// The chat list: names, previews, and the avatars beside them.
const char kBlurList[] =
    "#pane-side [role=\"row\"]{filter:blur(5px);transition:filter .12s ease}"
    "#pane-side [role=\"row\"]:hover{filter:none}";

// The open conversation: the messages, and the name in its header.
const char kBlurChat[] =
    "#main [role=\"row\"]{filter:blur(5px);transition:filter .12s ease}"
    "#main [role=\"row\"]:hover{filter:none}"
    "#main header{filter:blur(5px);transition:filter .12s ease}"
    "#main header:hover{filter:none}";

// Photos, stickers and profile pictures, which stay legible at a blur that
// hides text — so they get a heavier one, and are revealed by hovering the row
// they sit in rather than the image itself.
const char kBlurMedia[] =
    "#main img,#pane-side img{filter:blur(10px);transition:filter .12s ease}"
    "#main [role=\"row\"]:hover img,#pane-side [role=\"row\"]:hover img,"
    "#main header:hover img{filter:none}";

QString cssFor(const QString &level) {
  QString css;
  if (level == QLatin1String("list") || level == QLatin1String("both") ||
      level == QLatin1String("all"))
    css += QLatin1String(kBlurList);
  if (level == QLatin1String("chat") || level == QLatin1String("both") ||
      level == QLatin1String("all"))
    css += QLatin1String(kBlurChat);
  if (level == QLatin1String("all"))
    css += QLatin1String(kBlurMedia);
  return css;
}

QString jsStringLiteral(const QString &value) {
  QString escaped = value;
  escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
  escaped.replace(QLatin1Char('"'), QLatin1String("\\\""));
  return QLatin1Char('"') + escaped + QLatin1Char('"');
}

} // namespace

namespace PrivacyBlur {

QList<Level> levels() {
  return {
      {QStringLiteral("off"),
       QCoreApplication::translate("PrivacyBlur", "Off")},
      {QStringLiteral("list"),
       QCoreApplication::translate("PrivacyBlur", "Chat list")},
      {QStringLiteral("chat"),
       QCoreApplication::translate("PrivacyBlur", "Open conversation")},
      {QStringLiteral("both"),
       QCoreApplication::translate("PrivacyBlur",
                                   "Chat list and conversation")},
      {QStringLiteral("all"),
       QCoreApplication::translate("PrivacyBlur",
                                   "Everything, photos included")},
  };
}

QString currentLevelId() {
  const QString id = SettingsManager::instance()
                         .settings()
                         .value(QLatin1String(kSettingsKey),
                                QStringLiteral("off"))
                         .toString();
  for (const Level &level : levels())
    if (level.id == id)
      return id;
  return QStringLiteral("off");
}

void setCurrentLevelId(const QString &id) {
  SettingsManager::instance().settings().setValue(QLatin1String(kSettingsKey),
                                                  id);
}

QString scriptSource() {
  const QString css = cssFor(currentLevelId());
  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__CSS__"), jsStringLiteral(css));
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  if (currentLevelId() == QLatin1String("off"))
    return;

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace PrivacyBlur
