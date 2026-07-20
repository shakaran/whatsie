#include "focusmode.h"
#include "settingsmanager.h"

#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatly-focus-mode";
static const char kStyleId[] = "whatly-focus-mode";
static const char kSettingsKey[] = "focusMode";

// __ON__ becomes true/false. Re-running this swaps the mode without a reload.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  try {
    var ON = __ON__;
    var el = document.getElementById('__STYLE_ID__');
    if (!ON) { if (el) el.remove(); return; }
    if (!el) {
      el = document.createElement('style');
      el.id = '__STYLE_ID__';
      (document.head || document.documentElement).appendChild(el);
    }
    // Blank the chat-list rows' text (names, previews) while keeping layout
    // and avatars, so the list is still navigable. The open conversation is
    // untouched. Selectors are deliberately broad: WhatsApp Web's classes
    // change, the ARIA roles do not.
    el.textContent = [
      '#pane-side [role="listitem"] span[title],',
      '#pane-side [role="listitem"] ._ak8j,',
      '#pane-side [role="listitem"] ._ak8k {',
      '  color: transparent !important;',
      '  text-shadow: none !important;',
      '  background: currentColor;',
      '  border-radius: 4px;',
      '  filter: blur(6px);',
      '}',
      '#pane-side [role="listitem"]:hover span[title] {',
      '  color: inherit !important; background: none; filter: none;',
      '}'
    ].join('\n');
  } catch (e) { /* never break the page */ }
})();
)JS";

namespace FocusMode {

bool isEnabled() {
  return SettingsManager::instance()
      .settings()
      .value(QLatin1String(kSettingsKey), false)
      .toBool();
}

void setEnabled(bool enabled) {
  SettingsManager::instance().settings().setValue(QLatin1String(kSettingsKey),
                                                  enabled);
}

QString scriptSource() {
  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__ON__"),
                 isEnabled() ? QLatin1String("true") : QLatin1String("false"));
  source.replace(QLatin1String("__STYLE_ID__"), QLatin1String(kStyleId));
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  if (!isEnabled())
    return;

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace FocusMode
