#ifndef SCHEDULEDMESSAGES_H
#define SCHEDULEDMESSAGES_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

// Schedules WhatsApp messages to be sent at a future time (issue #250).
//
// The schedule is persisted to disk, so a message still goes out even if the
// app was closed when it came due: on the next launch, once WhatsApp Web has
// loaded, any overdue message is sent as a catch-up. While the app is open a
// timer polls for due messages — the "listener" case.
//
// The manager itself never touches Qt WebEngine. When a message is due it emits
// sendRequested(); MainWindow turns that into the actual page automation and
// calls reportResult() back with the outcome. One message is in flight at a
// time, so a failing send can never wedge the queue (a stuck send times out).
class ScheduledMessages : public QObject {
  Q_OBJECT
public:
  enum class Status { Pending, Sent, Failed };

  struct Entry {
    QString id;
    QString number; // digits only, international format
    QString name;   // optional label shown in the UI
    QString text;
    QDateTime dueAt;
    QDateTime createdAt;
    Status status = Status::Pending;
    QString error; // set when status == Failed
  };

  explicit ScheduledMessages(QObject *parent = nullptr);

  QList<Entry> entries() const { return m_entries; }

  // Adds a message and returns its id. number is normalised to digits only.
  QString add(const QString &number, const QString &name, const QString &text,
              const QDateTime &dueAt);
  void remove(const QString &id);
  void removeCompleted(); // drop everything already Sent or Failed

  // MainWindow reports the outcome of a send it was asked to perform.
  void reportResult(const QString &id, bool ok, const QString &error);

  // Begin polling and immediately process anything already overdue. Safe to
  // call more than once (only the first call starts the timer).
  void start();

  // Persistent script that lives in the page and performs the send; and the
  // one-shot starter that opens the chat for a specific job. Kept here so the
  // whole feature reads in one place, even though MainWindow injects them.
  static QString senderScriptSource();
  static QString startJobScript(const QString &id, const QString &number,
                                const QString &text);

  static QString statusLabel(Status status);

signals:
  // Emitted when a message comes due: MainWindow should open the chat and send.
  void sendRequested(const QString &id, const QString &number,
                     const QString &text);
  // Emitted whenever the list or a status changes (the dialog listens).
  void changed();

private:
  void load();
  void save() const;
  void checkDue();
  QString storagePath() const;
  Entry *find(const QString &id);

  QList<Entry> m_entries;
  QString m_sendingId;         // the one message currently in flight, if any
  QDateTime m_sendingSince;    // to time out a wedged send
  class QTimer *m_timer = nullptr;
  bool m_started = false;
};

#endif // SCHEDULEDMESSAGES_H
