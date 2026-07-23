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

// Redirect Chromium's own file-descriptor-2 output — which never passes through
// the Qt message handler above — to a file, so the "[FATAL:...] Check failed"
// line that precedes a Qt WebEngine hard-abort (issue #3) is persisted for a
// bug report instead of vanishing into journald. No-op when stderr is a TTY (a
// terminal user already sees it) or on non-Unix platforms. Call once, after the
// application/organisation names are set (the path is derived from them) and
// before Qt WebEngine starts. The previous session is kept alongside as
// "<name>.prev", so a crash log survives a relaunch.
void captureNativeStderr();

// Absolute path captureNativeStderr() redirected to, or empty if it did not.
QString nativeStderrLogPath();

// Rotate the capture file: keep the previous session as "<path>.prev", then
// leave a fresh empty file at `path`. Returns true on success. This is the
// file-management half of captureNativeStderr(), split out so it can be tested
// without redirecting the running process's stderr.
bool rotateCaptureFile(const QString &path);

} // namespace DebugLog

#endif // DEBUGLOG_H
