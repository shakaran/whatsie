#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QKeySequence>
#include <QList>
#include <QString>

// User-customisable keyboard shortcuts. Each app action registers an id, a
// human label and its default sequence; the stored override (if any) wins.
// The Settings UI lists the registry and writes overrides; MainWindow reads
// get() when it assigns shortcuts. Conflict detection is pure and unit-tested.
namespace Shortcuts {

struct Def {
  QString id;
  QString label;
  QKeySequence def;
};

// Register (or update) an action. Idempotent; the last default for an id wins.
void registerAction(const QString &id, const QString &label,
                    const QKeySequence &def);

// Everything registered so far, in registration order.
QList<Def> registered();

// The effective sequence for an id: the stored override, or its default.
QKeySequence get(const QString &id);

// Store an override (an empty sequence clears the shortcut).
void set(const QString &id, const QKeySequence &seq);

// Reset an id to its registered default (removes the override).
void reset(const QString &id);

// The id of another registered action already using `seq`, or an empty string
// if there is no conflict. An empty `seq` never conflicts.
QString conflictId(const QString &id, const QKeySequence &seq);

// Test seam: forget every registration (used by unit tests).
void clearRegistryForTest();

} // namespace Shortcuts

#endif // SHORTCUTS_H
