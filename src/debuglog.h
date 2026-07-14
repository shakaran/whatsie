#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <QString>

// A ring buffer of the last few hundred log lines — the app's own qWarning
// output and WhatsApp Web's console — kept in memory so a bug report can carry
// them.
//
// Without this, everything the app knew about a failure went to stderr, which
// nobody launching from a desktop icon ever sees. The report a user could
// actually produce said only "it does not work".
namespace DebugLog {

// Both sources funnel through here. Thread-safe: Chromium delivers console
// messages from its own thread.
void append(const QString &line);

// Newest last, oldest first, at most `maxLines` of them.
QString recent(int maxLines = 120);

// Installs the Qt message handler that captures qWarning/qCritical/qInfo. Call
// once, early. The previous handler still runs, so stderr keeps working.
void install();

} // namespace DebugLog

#endif // DEBUGLOG_H
