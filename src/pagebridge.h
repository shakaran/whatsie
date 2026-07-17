#ifndef PAGEBRIDGE_H
#define PAGEBRIDGE_H

#include <QObject>

// The one object WhatsApp's page can reach, over QWebChannel. Buttons Whatly
// injects into the page (see WebTweaks) call into here; nothing else is
// exposed, and the page cannot reach anything this class does not declare as a
// slot.
class PageBridge : public QObject {
  Q_OBJECT
public:
  explicit PageBridge(QObject *parent = nullptr) : QObject(parent) {}

public slots:
  // Invoked by the buttons injected next to the profile avatar.
  void toggleTheme() { emit themeToggleRequested(); }
  void togglePrivacyBlur() { emit privacyBlurToggleRequested(); }

  // Invoked by the scheduled-message sender once it has clicked Send (or given
  // up), so the schedule can mark the message sent or failed.
  void scheduledMessageResult(const QString &id, bool ok, const QString &error) {
    emit scheduledMessageFinished(id, ok, error);
  }

signals:
  void themeToggleRequested();
  void privacyBlurToggleRequested();
  void scheduledMessageFinished(const QString &id, bool ok,
                                const QString &error);
};

#endif // PAGEBRIDGE_H
