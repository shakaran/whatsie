// In-window accounts: a tab bar over a stack of WhatsApp views, one per signed-
// in account. The tab bar hides itself when only the default account exists, so
// a single-account setup is untouched by any of this.
#include "mainwindow.h"

#include <QFile>
#include <QInputDialog>
#include <QStandardPaths>
#include <QMenu>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QWidget>

#include "appprofile.h"
#include "common.h"
#include "utils.h"
#include "webview.h"

// The file `whatly --unread` reads: the current unread total for this account.
// Kept in the runtime dir (cleared on logout) with the profile suffix, so each
// --profile account has its own.
static QString unreadCountFile() {
  QString dir =
      QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
  if (dir.isEmpty())
    dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  return dir + QStringLiteral("/whatly-unread") + AppProfile::suffix();
}

void MainWindow::buildAccountArea() {
  auto *central = new QWidget(this);
  auto *layout = new QVBoxLayout(central);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_accountBar = new QTabBar(central);
  m_accountBar->setObjectName("accountBar");
  m_accountBar->setExpanding(false);
  m_accountBar->setDrawBase(false);
  m_accountBar->setFocusPolicy(Qt::NoFocus);
  m_accountBar->setContextMenuPolicy(Qt::CustomContextMenu);

  m_accountStack = new QStackedWidget(central);

  QSizePolicy expanding(QSizePolicy::Expanding, QSizePolicy::Expanding);
  expanding.setHorizontalStretch(1);
  expanding.setVerticalStretch(1);
  m_accountStack->setSizePolicy(expanding);

  layout->addWidget(m_accountBar);
  layout->addWidget(m_accountStack);
  setCentralWidget(central);

  connect(m_accountBar, &QTabBar::currentChanged, this, [this](int index) {
    // The last tab is the "+" affordance; choosing it adds an account instead
    // of switching to a page that is not there.
    if (index == m_accounts.size())
      promptAddAccount();
    else
      setActiveAccount(index);
  });
  connect(m_accountBar, &QTabBar::customContextMenuRequested, this,
          [this](const QPoint &pos) {
            const int index = m_accountBar->tabAt(pos);
            if (index < 0 || index >= m_accounts.size())
              return;
            QMenu menu;
            QAction *rename = menu.addAction(tr("Rename…"));
            QAction *remove = menu.addAction(tr("Remove account"));
            // The default account is the app's own session; it can be renamed
            // but not removed, or there would be nothing to fall back to.
            remove->setEnabled(m_accounts[index].id != QString() &&
                               m_accounts.size() > 1);
            QAction *chosen = menu.exec(m_accountBar->mapToGlobal(pos));
            if (chosen == rename)
              renameAccount(index);
            else if (chosen == remove)
              removeAccount(index);
          });
}

WebView *MainWindow::addAccount(const QString &id, const QString &name,
                                bool load) {
  auto *view = new WebView(m_accountStack);
  view->accountId = id;
  view->addAction(m_minimizeAction);
  view->addAction(m_lockAction);
  view->addAction(m_quitAction);

  m_accountStack->addWidget(view);
  m_accounts.append({id, name, view, 0});

  // The active view is the one the rest of MainWindow drives through
  // m_webEngine; without a page yet, point it here so the first account is
  // usable before its page finishes loading.
  if (!m_webEngine)
    m_webEngine = view;

  if (load)
    createPageFor(view, id);
  return view;
}

void MainWindow::setActiveAccount(int index) {
  if (index < 0 || index >= m_accounts.size())
    return;
  m_activeAccount = index;
  m_webEngine = m_accounts[index].view;   // everything current-account flows through this
  m_accountStack->setCurrentWidget(m_accounts[index].view);
  if (m_accountBar->currentIndex() != index) {
    QSignalBlocker block(m_accountBar);
    m_accountBar->setCurrentIndex(index);
  }
  // Re-point the lock overlay and refresh the title to the now-active account.
  if (m_webEngine && m_webEngine->page())
    setWindowTitle(QApplication::applicationDisplayName() + AppProfile::label() +
                   ": " + m_webEngine->page()->title());
}

int MainWindow::accountIndexForView(const QObject *view) const {
  for (int i = 0; i < m_accounts.size(); ++i)
    if (m_accounts[i].view == view)
      return i;
  return -1;
}

// Rebuild the tab labels: the account name, plus its own unread count, plus a
// trailing "+" tab. Cheap, and called only when something actually changed.
void MainWindow::refreshAccountTabs() {
  if (!m_accountBar)
    return;
  QSignalBlocker block(m_accountBar);

  while (m_accountBar->count() > m_accounts.size() + 1)
    m_accountBar->removeTab(m_accountBar->count() - 1);
  while (m_accountBar->count() < m_accounts.size() + 1)
    m_accountBar->addTab(QString());

  for (int i = 0; i < m_accounts.size(); ++i) {
    QString label = m_accounts[i].name;
    if (m_accounts[i].unread > 0)
      label += QStringLiteral("  (%1)").arg(m_accounts[i].unread);
    m_accountBar->setTabText(i, label);
  }
  m_accountBar->setTabText(m_accounts.size(), QStringLiteral("  +  "));
  m_accountBar->setTabToolTip(m_accounts.size(), tr("Add another account"));

  // A single account needs no tab bar — hidden, this is invisible.
  m_accountBar->setVisible(m_accounts.size() > 1);
  m_accountBar->setCurrentIndex(m_activeAccount);
}

void MainWindow::updateTrayUnread() {
  int total = 0;
  for (const Account &a : m_accounts)
    total += a.unread;

  if (QFile f(unreadCountFile());
      f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    f.write(QByteArray::number(total));
    f.close();
  }

  if (total > 0) {
    m_restoreAction->setText(tr("Restore") + " | " + QString::number(total) +
                             " " + (total > 1 ? tr("messages") : tr("message")));
    m_systemTrayIcon->setIcon(getTrayIcon(total));
    setWindowIcon(getTrayIcon(total));
  } else {
    m_restoreAction->setText(tr("Restore"));
    m_systemTrayIcon->setIcon(m_trayIconNormal);
    setWindowIcon(themeIcon("whatly", ":/icons/app/icon-64.png"));
  }
}

void MainWindow::promptAddAccount() {
  // Put the tab bar back on the account it was on: the click landed on "+",
  // which is not a real page.
  {
    QSignalBlocker block(m_accountBar);
    m_accountBar->setCurrentIndex(m_activeAccount);
  }

  bool ok = false;
  const QString name =
      QInputDialog::getText(this, tr("Add account"),
                            tr("Name for the new account:"), QLineEdit::Normal,
                            tr("Account %1").arg(m_accounts.size() + 1), &ok);
  if (!ok || name.trimmed().isEmpty())
    return;

  // A random, stable id keeps the storage directory name independent of the
  // display name, so renaming an account never moves its session.
  const QString id = Utils::generateRandomId(8);
  WebView *view = addAccount(id, name.trimmed(), true);
  saveAccounts();
  refreshAccountTabs();
  setActiveAccount(accountIndexForView(view));
}

void MainWindow::renameAccount(int index) {
  if (index < 0 || index >= m_accounts.size())
    return;
  bool ok = false;
  const QString name = QInputDialog::getText(
      this, tr("Rename account"), tr("Account name:"), QLineEdit::Normal,
      m_accounts[index].name, &ok);
  if (!ok || name.trimmed().isEmpty())
    return;
  m_accounts[index].name = name.trimmed();
  saveAccounts();
  refreshAccountTabs();
}

void MainWindow::removeAccount(int index) {
  // The default account (id "") is the app's own session and stays.
  if (index <= 0 || index >= m_accounts.size() ||
      m_accounts[index].id.isEmpty())
    return;

  Account account = m_accounts.takeAt(index);
  m_accountStack->removeWidget(account.view);
  account.view->deleteLater();

  if (m_activeAccount >= m_accounts.size())
    m_activeAccount = m_accounts.size() - 1;

  saveAccounts();
  refreshAccountTabs();
  setActiveAccount(m_activeAccount);
  updateTrayUnread();
}

// The accounts list lives in the (process-level) settings, so it is per
// --profile: launching --profile=work has its own separate set of tabs. Stored
// as parallel id/name lists; the default account is implicit and always first.
void MainWindow::saveAccounts() {
  QStringList ids, names;
  for (const Account &a : m_accounts) {
    if (a.id.isEmpty())
      continue;   // the default account is implicit
    ids << a.id;
    names << a.name;
  }
  QSettings &s = SettingsManager::instance().settings();
  s.setValue(QStringLiteral("accounts/ids"), ids);
  s.setValue(QStringLiteral("accounts/names"), names);
}

void MainWindow::loadAccounts() {
  QSettings &s = SettingsManager::instance().settings();
  const QStringList ids = s.value(QStringLiteral("accounts/ids")).toStringList();
  const QStringList names =
      s.value(QStringLiteral("accounts/names")).toStringList();

  // The default account is always present and always first.
  addAccount(QString(), tr("Account 1"), true);

  for (int i = 0; i < ids.size(); ++i) {
    const QString name =
        i < names.size() ? names[i] : tr("Account %1").arg(i + 2);
    addAccount(ids[i], name, true);
  }

  refreshAccountTabs();
  setActiveAccount(0);
}
