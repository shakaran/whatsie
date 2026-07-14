#include "debuglog.h"

#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QtGlobal>

namespace {

const int kMaxLines = 400;

QMutex g_mutex;
QStringList g_lines;
QtMessageHandler g_previousHandler = nullptr;

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

} // namespace DebugLog
