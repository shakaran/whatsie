#include "debuglog.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QStringList>
#include <QtGlobal>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {

const int kMaxLines = 400;

QMutex g_mutex;
QStringList g_lines;
QtMessageHandler g_previousHandler = nullptr;
QString g_nativeStderrLogPath;

// Benign noise emitted by Qt/Chromium (Qt WebEngine) as its worker threads and
// thread-local storage are torn down at exit. It says nothing actionable but
// clutters the terminal, so it is kept in the captured log yet never printed.
bool isBenignTeardownNoise(const QString &message) {
  static const char *const patterns[] = {
      "QThreadStorage: entry ",         // "... destroyed before end of thread"
      "QThreadStorage: Thread ",         // TLS destruction ordering at exit
  };
  for (const char *p : patterns)
    if (message.contains(QLatin1String(p)))
      return true;
  return false;
}

void messageHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &message) {
  const char *level = "";
  switch (type) {
  case QtDebugMsg:    level = "debug";    break;
  case QtInfoMsg:     level = "info";     break;
  case QtWarningMsg:  level = "warning";  break;
  case QtCriticalMsg: level = "critical"; break;
  case QtFatalMsg:    level = "fatal";    break;
  }
  DebugLog::append(QStringLiteral("[%1] %2").arg(QLatin1String(level), message));

  // Swallow the benign teardown noise: still recorded above (so a bug report
  // keeps it) but not echoed to the terminal.
  if (isBenignTeardownNoise(message))
    return;

  // Keep printing to stderr exactly as before: capturing the log must not
  // silence it for anyone running from a terminal.
  if (g_previousHandler)
    g_previousHandler(type, context, message);
}

} // namespace

namespace DebugLog {

void append(const QString &line) {
  const QString stamped =
      QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")) +
      QLatin1Char(' ') + line;

  QMutexLocker locker(&g_mutex);
  g_lines.append(stamped);
  if (g_lines.size() > kMaxLines)
    g_lines.remove(0, g_lines.size() - kMaxLines);
}

QString recent(int maxLines) {
  QMutexLocker locker(&g_mutex);
  if (g_lines.isEmpty())
    return QString();
  const int from = qMax(0, g_lines.size() - maxLines);
  return QStringList(g_lines.mid(from)).join(QLatin1Char('\n'));
}

void install() {
  static bool installed = false;
  if (installed)
    return;
  installed = true;
  g_previousHandler = qInstallMessageHandler(messageHandler);
}

bool rotateCaptureFile(const QString &path) {
  // Keep the previous session's file (which may hold a crash) alongside, then
  // start this session fresh so the log stays small and self-describing.
  const QString prev = path + QStringLiteral(".prev");
  QFile::remove(prev);
  QFile::rename(path, prev);
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;
  f.close();
  return true;
}

void captureNativeStderr() {
#ifdef Q_OS_UNIX
  // A terminal user already sees Chromium's stderr; only redirect the
  // desktop/systemd launch, where it would otherwise be lost.
  if (::isatty(STDERR_FILENO))
    return;
  const QString dir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dir.isEmpty())
    return;
  QDir().mkpath(dir);
  const QString path = dir + QStringLiteral("/whatly-webengine.log");
  if (!rotateCaptureFile(path))
    return;

  const int fd = ::open(path.toLocal8Bit().constData(), O_WRONLY | O_APPEND);
  if (fd < 0)
    return;
  // The kernel commits each write() to the file immediately, so even the last
  // line Chromium emits before an abort survives — unlike an in-process reader.
  ::dup2(fd, STDERR_FILENO);
  ::close(fd);
  g_nativeStderrLogPath = path;
#endif
}

QString nativeStderrLogPath() { return g_nativeStderrLogPath; }

} // namespace DebugLog
