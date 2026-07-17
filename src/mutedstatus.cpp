#include "mutedstatus.h"
#include "settingsmanager.h"

#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatly-muted-status";
static const char kSettingsKey[] = "hideMutedStatus";

// The heading WhatsApp puts above the muted section, in the languages Whatly
// ships. Matched case-insensitively and accent-insensitively against the
// heading's text; kept generous (both the full "Muted updates" phrase and the
// bare "muted" adjective) because WhatsApp has used both. If WhatsApp renames
// the section this list is the one thing to update.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  var ENABLE = __ENABLE__;
  if (window.__whatlyMutedStatusReady) {
    window.__whatlyMutedStatus = ENABLE;
    return;
  }
  window.__whatlyMutedStatus = ENABLE;
  window.__whatlyMutedStatusReady = true;

  // Localised "muted (updates)" headings, normalised (lowercased, accents
  // stripped) the same way the DOM text is before comparing.
  var NEEDLES = __NEEDLES__;

  var norm = function (s) {
    try {
      return (s || '').normalize('NFD').replace(/[̀-ͯ]/g, '')
        .toLowerCase().trim();
    } catch (e) { return (s || '').toLowerCase().trim(); }
  };
  var matches = function (text) {
    var t = norm(text);
    if (!t || t.length > 40) return false;   // headings are short
    for (var i = 0; i < NEEDLES.length; i++)
      if (t === NEEDLES[i] || t.indexOf(NEEDLES[i]) !== -1) return true;
    return false;
  };

  var MARK = 'data-whatly-muted-hidden';
  var hide = function (el) {
    if (el.getAttribute(MARK)) return;
    el.setAttribute(MARK, '1');
    el.setAttribute('data-whatly-muted-display', el.style.display || '');
    el.style.display = 'none';
  };
  var show = function (el) {
    if (!el.getAttribute(MARK)) return;
    el.style.display = el.getAttribute('data-whatly-muted-display') || '';
    el.removeAttribute(MARK);
    el.removeAttribute('data-whatly-muted-display');
  };

  // A section header is a short text node sitting in the status list. Once
  // found, the muted section is the header plus the following siblings up to
  // the next header, all hidden together. If the header sits alone in a wrapper
  // that holds the whole section, hide that wrapper instead.
  var isHeaderLike = function (el) {
    return el.getAttribute && (el.getAttribute('role') === 'heading' ||
      /^H[1-6]$/.test(el.tagName) || el.children.length === 0);
  };

  var apply = function () {
    try {
      var enabled = window.__whatlyMutedStatus;
      var marked = document.querySelectorAll('[' + MARK + ']');
      if (!enabled) {
        for (var m = 0; m < marked.length; m++) show(marked[m]);
        return;
      }
      // Only the left pane (chat list / Status-Updates), never the open chat.
      // Restricted to real headings: scanning every span/div would be both
      // wasteful on a timer and liable to hide a contact literally named after
      // one of the needle words.
      var scope = document.querySelector('#pane-side') || document.body;
      var headings = scope.querySelectorAll(
        '[role="heading"], h1, h2, h3, h4, h5, h6');
      for (var i = 0; i < headings.length; i++) {
        var h = headings[i];
        if (!matches(h.textContent) || !isHeaderLike(h)) continue;

        // Climb to the element that is a direct child of the list container,
        // i.e. the section's own top-level node.
        var node = h;
        while (node.parentElement && node.parentElement !== scope &&
               node.parentElement.childElementCount <= 1)
          node = node.parentElement;

        // Hide this node and every following sibling until the next section.
        hide(node);
        var sib = node.nextElementSibling;
        while (sib) {
          if (sib.querySelector &&
              [].some.call(sib.querySelectorAll(
                '[role="heading"], h1, h2, h3, h4, h5, h6'),
                function (x) { return matches(x.textContent); }))
            break;
          hide(sib);
          sib = sib.nextElementSibling;
        }
      }
    } catch (e) { /* never break the page */ }
  };

  apply();
  setInterval(apply, 1500);
})();
)JS";

namespace {

// The needles, pre-normalised (lowercase, no accents) as a JS array literal.
QString needlesLiteral() {
  static const char *kNeedles[] = {
      "muted updates", "muted", // en
      "actualizaciones silenciadas", "silenciados", "silenciadas", // es
      "mises a jour en sourdine", "en sourdine", // fr
      "stummgeschaltete updates", "stummgeschaltet", // de
      "aggiornamenti silenziati", "silenziati", // it
      "atualizacoes silenciadas", "silenciadas", // pt
      "gedempte updates", "gedempt", // nl
      "wyciszone aktualizacje", "wyciszone", // pl
      "otklyuchennye obnovleniya", // ru (transliterated fallback)
      "sessize alinmis guncellemeler", "sessize alindi", // tr
  };
  QString out = QLatin1String("[");
  const int n = int(sizeof(kNeedles) / sizeof(kNeedles[0]));
  for (int i = 0; i < n; ++i) {
    if (i)
      out += QLatin1Char(',');
    out += QLatin1Char('"');
    out += QString::fromUtf8(kNeedles[i]);
    out += QLatin1Char('"');
  }
  out += QLatin1Char(']');
  return out;
}

} // namespace

namespace MutedStatus {

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
  source.replace(QLatin1String("__ENABLE__"),
                 isEnabled() ? QStringLiteral("true") : QStringLiteral("false"));
  source.replace(QLatin1String("__NEEDLES__"), needlesLiteral());
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

} // namespace MutedStatus
