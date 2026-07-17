#ifndef SCHEDULEDMESSAGESDIALOG_H
#define SCHEDULEDMESSAGESDIALOG_H

#include <QDialog>

class ScheduledMessages;
class QTableWidget;
class QLineEdit;
class QPlainTextEdit;
class QDateTimeEdit;

// Lists scheduled messages and lets the user add or remove them. Purely a view
// over ScheduledMessages: it never sends anything itself.
class ScheduledMessagesDialog : public QDialog {
  Q_OBJECT
public:
  explicit ScheduledMessagesDialog(ScheduledMessages *manager,
                                   QWidget *parent = nullptr);

private slots:
  void addFromForm();
  void removeSelected();
  void refresh();

private:
  ScheduledMessages *m_manager;
  QTableWidget *m_table;
  QLineEdit *m_number;
  QLineEdit *m_name;
  QDateTimeEdit *m_when;
  QPlainTextEdit *m_message;
};

#endif // SCHEDULEDMESSAGESDIALOG_H
