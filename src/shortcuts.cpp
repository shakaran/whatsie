#include "shortcuts.h"
#include "settingsmanager.h"

#include <QHash>

namespace {
// Registration order preserved for the UI; defaults kept for reset().
QList<Shortcuts::Def> &registry() {
  static QList<Shortcuts::Def> r;
  return r;
}
QHash<QString, QKeySequence> &defaults() {
  static QHash<QString, QKeySequence> d;
  return d;
}
QString key(const QString &id) { return QStringLiteral("shortcut/") + id; }
} // namespace

namespace Shortcuts {

void registerAction(const QString &id, const QString &label,
                    const QKeySequence &def) {
  defaults()[id] = def;
  for (Def &d : registry()) {
    if (d.id == id) {
      d.label = label;
      d.def = def;
      return;
    }
  }
  registry().append({id, label, def});
}

QList<Def> registered() { return registry(); }

QKeySequence get(const QString &id) {
  const QVariant v =
      SettingsManager::instance().settings().value(key(id));
  if (v.isValid())
    return QKeySequence(v.toString());
  return defaults().value(id);
}

void set(const QString &id, const QKeySequence &seq) {
  SettingsManager::instance().settings().setValue(
      key(id), seq.toString(QKeySequence::PortableText));
}

void reset(const QString &id) {
  SettingsManager::instance().settings().remove(key(id));
}

QString conflictId(const QString &id, const QKeySequence &seq) {
  if (seq.isEmpty())
    return QString();
  for (const Def &d : registry()) {
    if (d.id == id)
      continue;
    if (get(d.id) == seq)
      return d.id;
  }
  return QString();
}

void clearRegistryForTest() {
  registry().clear();
  defaults().clear();
}

} // namespace Shortcuts
