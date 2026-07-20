#include "scheduledmessagesdialog.h"
#include "scheduledmessages.h"

#include <QAbstractItemView>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ScheduledMessagesDialog::ScheduledMessagesDialog(ScheduledMessages *manager,
                                                 QWidget *parent)
    : QDialog(parent), m_manager(manager) {
  setWindowTitle(tr("Scheduled messages"));
  resize(560, 460);

  auto *layout = new QVBoxLayout(this);

  m_table = new QTableWidget(this);
  m_table->setColumnCount(4);
  m_table->setHorizontalHeaderLabels(
      {tr("To"), tr("When"), tr("Message"), tr("Status")});
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->verticalHeader()->setVisible(false);
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  layout->addWidget(m_table, 1);

  // ── Add form ──────────────────────────────────────────────────────────────
  auto *form = new QFormLayout;
  m_number = new QLineEdit(this);
  m_number->setPlaceholderText(tr("Phone number, international format e.g. 447700900000"));
  form->addRow(tr("To"), m_number);

  m_name = new QLineEdit(this);
  m_name->setPlaceholderText(tr("Optional label"));
  form->addRow(tr("Name"), m_name);

  m_when = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(300), this);
  m_when->setCalendarPopup(true);
  m_when->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm"));
  form->addRow(tr("When"), m_when);

  m_recurrence = new QComboBox(this);
  m_recurrence->addItem(tr("Once"),
                        int(ScheduledMessages::Recurrence::None));
  m_recurrence->addItem(tr("Daily"),
                        int(ScheduledMessages::Recurrence::Daily));
  m_recurrence->addItem(tr("Weekdays"),
                        int(ScheduledMessages::Recurrence::Weekdays));
  m_recurrence->addItem(tr("Weekly"),
                        int(ScheduledMessages::Recurrence::Weekly));
  form->addRow(tr("Repeat"), m_recurrence);

  m_reminder = new QCheckBox(
      tr("Remind me instead of sending (notify, don't message)"), this);
  form->addRow(QString(), m_reminder);

  m_message = new QPlainTextEdit(this);
  m_message->setPlaceholderText(tr("Message to send"));
  m_message->setFixedHeight(70);
  form->addRow(tr("Message"), m_message);
  layout->addLayout(form);

  auto *hint = new QLabel(
      tr("The app must be running and WhatsApp logged in at the scheduled "
         "time. If it is closed, the message is sent the next time you open "
         "the app. Sending opens the recipient's chat."),
      this);
  hint->setWordWrap(true);
  hint->setStyleSheet(QStringLiteral("color: gray;"));
  layout->addWidget(hint);

  // ── Buttons ───────────────────────────────────────────────────────────────
  auto *buttons = new QHBoxLayout;
  auto *schedule = new QPushButton(tr("Schedule"), this);
  auto *removeBtn = new QPushButton(tr("Remove selected"), this);
  auto *clearBtn = new QPushButton(tr("Clear sent/failed"), this);
  auto *closeBtn = new QPushButton(tr("Close"), this);
  buttons->addWidget(schedule);
  buttons->addWidget(removeBtn);
  buttons->addWidget(clearBtn);
  buttons->addStretch();
  buttons->addWidget(closeBtn);
  layout->addLayout(buttons);

  connect(schedule, &QPushButton::clicked, this,
          &ScheduledMessagesDialog::addFromForm);
  connect(removeBtn, &QPushButton::clicked, this,
          &ScheduledMessagesDialog::removeSelected);
  connect(clearBtn, &QPushButton::clicked, this, [this]() {
    m_manager->removeCompleted();
  });
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  connect(m_manager, &ScheduledMessages::changed, this,
          &ScheduledMessagesDialog::refresh);

  refresh();
}

void ScheduledMessagesDialog::addFromForm() {
  const QString number = m_number->text().trimmed();
  const QString text = m_message->toPlainText();
  if (number.isEmpty() || text.trimmed().isEmpty()) {
    QMessageBox::warning(this, tr("Scheduled messages"),
                         tr("Enter a phone number and a message."));
    return;
  }
  if (m_when->dateTime() <= QDateTime::currentDateTime())
    if (QMessageBox::question(
            this, tr("Scheduled messages"),
            tr("The time is in the past — send this message now?")) !=
        QMessageBox::Yes)
      return;

  const auto recurrence = static_cast<ScheduledMessages::Recurrence>(
      m_recurrence->currentData().toInt());
  m_manager->add(number, m_name->text(), text, m_when->dateTime(), recurrence,
                 m_reminder->isChecked());
  m_number->clear();
  m_name->clear();
  m_message->clear();
  m_reminder->setChecked(false);
  m_recurrence->setCurrentIndex(0);
  m_when->setDateTime(QDateTime::currentDateTime().addSecs(300));
}

void ScheduledMessagesDialog::removeSelected() {
  const int row = m_table->currentRow();
  if (row < 0)
    return;
  const QString id =
      m_table->item(row, 0)->data(Qt::UserRole).toString();
  if (!id.isEmpty())
    m_manager->remove(id);
}

void ScheduledMessagesDialog::refresh() {
  const auto entries = m_manager->entries();
  m_table->setRowCount(entries.size());
  for (int i = 0; i < entries.size(); ++i) {
    const auto &e = entries[i];
    const QString to =
        e.name.isEmpty() ? e.number : (e.name + QStringLiteral(" (") + e.number +
                                       QStringLiteral(")"));
    auto *toItem = new QTableWidgetItem(to);
    toItem->setData(Qt::UserRole, e.id);
    m_table->setItem(i, 0, toItem);
    m_table->setItem(
        i, 1,
        new QTableWidgetItem(e.dueAt.toString(QStringLiteral("yyyy-MM-dd HH:mm"))));
    QString preview = e.text;
    preview.replace(QLatin1Char('\n'), QLatin1Char(' '));
    if (preview.size() > 60)
      preview = preview.left(57) + QStringLiteral("…");
    m_table->setItem(i, 2, new QTableWidgetItem(preview));
    QString status = ScheduledMessages::statusLabel(e.status);
    if (e.status == ScheduledMessages::Status::Failed && !e.error.isEmpty())
      status += QStringLiteral(" — ") + e.error;
    m_table->setItem(i, 3, new QTableWidgetItem(status));
  }
}
