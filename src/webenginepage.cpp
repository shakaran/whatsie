#include "webenginepage.h"
#include "common.h"
#include "webengineprofilemanager.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>

WebEnginePage::WebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent) {

  auto userAgent = profile->httpUserAgent();
  qDebug() << "WebEnginePage::Profile::UserAgent" << userAgent;
  auto webengineversion =
      userAgent.split("QtWebEngine").last().split(" ").first();
  auto toRemove = "QtWebEngine" + webengineversion;
  auto cleanUserAgent = userAgent.remove(toRemove).replace("  ", " ");
  profile->setHttpUserAgent(cleanUserAgent);

  connect(this, &QWebEnginePage::loadFinished, this,
          &WebEnginePage::handleLoadFinished);
  connect(this, &QWebEnginePage::authenticationRequired, this,
          &WebEnginePage::handleAuthenticationRequired);
  connect(this, &QWebEnginePage::permissionRequested, this,
          &WebEnginePage::handlePermissionRequested);
  connect(this, &QWebEnginePage::proxyAuthenticationRequired, this,
          &WebEnginePage::handleProxyAuthenticationRequired);
  connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this,
          &WebEnginePage::handleRegisterProtocolHandlerRequested);
  connect(this, &QWebEnginePage::selectClientCertificate, this,
          &WebEnginePage::handleSelectClientCertificate);
  connect(this, &QWebEnginePage::certificateError, this,
          &WebEnginePage::handleCertificateError);
}

bool WebEnginePage::acceptNavigationRequest(const QUrl &url,
                                            QWebEnginePage::NavigationType type,
                                            bool isMainFrame) {
  // Open link clicks in the default browser — but only when they leave
  // WhatsApp. The logout flow ends with a click-triggered navigation back to
  // web.whatsapp.com; hijacking it opened a browser tab and left the app
  // stuck on the "Logging out" overlay forever.
  if (QWebEnginePage::NavigationType::NavigationTypeLinkClicked == type &&
      url.host() != QLatin1String("web.whatsapp.com")) {
    QDesktopServices::openUrl(url);
    return false;
  }

  return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

// A window request — middle-clicking a link, target="_blank" — used to return a
// fresh WebEnginePage that was never put into a view and had no parent. Such a
// page leaks, and QWebEnginePage::view() returns nullptr for it, which the
// dialog handlers below dereference to parent their message boxes. Links belong
// in the browser here anyway (acceptNavigationRequest already sends a plain
// click there), so hand the URL to the desktop and discard the page. It is a
// plain QWebEnginePage on purpose: none of this class's dialog handlers are
// wired to it, so nothing can reach a view that does not exist.
QWebEnginePage *
WebEnginePage::createWindow(QWebEnginePage::WebWindowType type) {
  Q_UNUSED(type);
  auto *scratch = new QWebEnginePage(profile(), this);
  connect(scratch, &QWebEnginePage::urlChanged, scratch,
          [scratch](const QUrl &url) {
            if (url.isValid() && !url.isEmpty())
              QDesktopServices::openUrl(url);
            scratch->deleteLater();
          });
  return scratch;
}

// view() is null for any page that is not inside a view. A null parent is fine
// for a dialog — it just becomes top-level — whereas dereferencing view() is a
// crash.
QWidget *WebEnginePage::dialogParent() {
  QWidget *currentView = view();
  return currentView ? currentView->window() : nullptr;
}

// The dialogs below take their icons from the parent's style. With no parent
// there is still a style to ask: the application's.
static QStyle *dialogStyle(QWidget *parent) {
  return parent ? parent->style() : QApplication::style();
}

inline QString questionForPermission(const QWebEnginePermission &permission) {
  switch (permission.permissionType()) {
  case QWebEnginePermission::PermissionType::Geolocation:
    return WebEnginePage::tr("Allow %1 to access your location information?");
  case QWebEnginePermission::PermissionType::MediaAudioCapture:
    return WebEnginePage::tr("Allow %1 to access your microphone?");
  case QWebEnginePermission::PermissionType::MediaVideoCapture:
    return WebEnginePage::tr("Allow %1 to access your webcam?");
  case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
    return WebEnginePage::tr("Allow %1 to access your microphone and webcam?");
  case QWebEnginePermission::PermissionType::MouseLock:
    return WebEnginePage::tr("Allow %1 to lock your mouse cursor?");
  case QWebEnginePermission::PermissionType::DesktopVideoCapture:
    return WebEnginePage::tr("Allow %1 to capture video of your desktop?");
  case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
    return WebEnginePage::tr(
        "Allow %1 to capture audio and video of your desktop?");
  case QWebEnginePermission::PermissionType::Notifications:
    return WebEnginePage::tr("Allow %1 to show notification on your desktop?");
  case QWebEnginePermission::PermissionType::ClipboardReadWrite:
    return WebEnginePage::tr("Allow %1 to read your clipboard? This is needed to "
                             "paste images into a chat.");
  case QWebEnginePermission::PermissionType::LocalFontsAccess:
    return WebEnginePage::tr("Allow %1 to see the fonts installed on your system?");
  default:
    return QString();
  }
}

void WebEnginePage::handlePermissionRequested(QWebEnginePermission permission) {
  bool autoPlay = true;
  if (SettingsManager::instance().settings().value("autoPlayMedia").isValid())
    autoPlay = SettingsManager::instance()
                   .settings()
                   .value("autoPlayMedia", false)
                   .toBool();

  if (autoPlay && (permission.permissionType() == QWebEnginePermission::PermissionType::MediaVideoCapture ||
                   permission.permissionType() == QWebEnginePermission::PermissionType::MediaAudioVideoCapture)) {
    WebEngineProfileManager::instance().profile()->settings()
        ->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
  }

  QString title = tr("Permission Request");
  QString question = questionForPermission(permission).arg(permission.origin().host());

  QString permissionTypeStr = QString::number(static_cast<int>(permission.permissionType()));
  SettingsManager::instance().settings().beginGroup("permissions");

  if (SettingsManager::instance().settings().value(permissionTypeStr, false).toBool()) {
    permission.grant();
  } else if (question.isEmpty()) {
    // No wording for this type, so we cannot ask. Deny it, but do NOT persist
    // that denial: writing an answer the user was never asked for is what used
    // to silently and permanently disable permissions we simply had no case
    // for — clipboard reads among them. Leaving the key unset means a future
    // build that knows how to ask will still ask.
    qWarning() << "Denying permission type with no prompt:"
               << static_cast<int>(permission.permissionType());
    permission.deny();
  } else if (QMessageBox::question(dialogParent(), title, question) ==
             QMessageBox::Yes) {
    permission.grant();
    SettingsManager::instance().settings().setValue(permissionTypeStr, true);
  } else {
    permission.deny();
    SettingsManager::instance().settings().setValue(permissionTypeStr, false);
  }
  SettingsManager::instance().settings().endGroup();
}

void WebEnginePage::handleLoadFinished(bool ok) {

  // Grant notifications on first run — they are the whole point of a messaging
  // client. This used to write "permissions/Notifications", a key nothing ever
  // read (the handler keys on the numeric PermissionType), so it granted
  // nothing: the user was prompted instead, and a dismissed prompt left the
  // permission denied for good, with WhatsApp Web pointing them at a browser
  // address bar that does not exist here. Grant it on the profile too, so the
  // page sees it immediately.
  const auto notifications = QWebEnginePermission::PermissionType::Notifications;
  const QString notificationsKey =
      QStringLiteral("permissions/%1").arg(static_cast<int>(notifications));
  QSettings &settings = SettingsManager::instance().settings();
  if (!settings.value(notificationsKey).isValid()) {
    settings.setValue(notificationsKey, true);
    const QWebEnginePermission permission =
        profile()->queryPermission(QUrl(whatsAppOrigin), notifications);
    if (permission.isValid())
      permission.grant();
  }

  if (ok) {
    injectPreventScrollWheelZoomHelper();
    injectNewChatJavaScript();
  }
}

void WebEnginePage::fullScreenRequestedByPage(
    QWebEngineFullScreenRequest request) {
  request.accept();
}

QStringList WebEnginePage::chooseFiles(QWebEnginePage::FileSelectionMode mode,
                                       const QStringList &oldFiles,
                                       const QStringList &acceptedMimeTypes) {
  qDebug() << mode << oldFiles << acceptedMimeTypes;
  QFileDialog::FileMode dialogMode;
  if (mode == QWebEnginePage::FileSelectOpen) {
    dialogMode = QFileDialog::ExistingFile;
  } else {
    dialogMode = QFileDialog::ExistingFiles;
  }

  // Parented, so it is modal to the window instead of a stray top-level that
  // can end up behind it.
  QFileDialog *dialog = new QFileDialog(dialogParent());

  // The desktop's own file chooser by default. Qt's built-in one has no
  // bookmarks, no Recent, and no address bar, which left people unable to
  // reach a file that was not already under the directory it happened to open
  // in — they had to copy it somewhere reachable first. The setting stays for
  // anyone who prefers the Qt dialog.
  bool usenativeFileDialog = SettingsManager::instance()
                                 .settings()
                                 .value("useNativeFileDialog", true)
                                 .toBool();

  if (usenativeFileDialog == false) {
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);
  }
  dialog->setFileMode(dialogMode);

  // Reopen where the last attachment came from rather than in whatever the
  // process's working directory happens to be.
  const QString lastDir =
      SettingsManager::instance().settings().value("lastAttachmentDir").toString();
  if (!lastDir.isEmpty() && QDir(lastDir).exists())
    dialog->setDirectory(lastDir);

  QStringList mimeFilters;
  mimeFilters.append("application/octet-stream"); // to show All files(*)
  mimeFilters.append(acceptedMimeTypes);

  if (acceptedMimeTypes.contains("image/*")) {
    foreach (QByteArray mime, QImageReader::supportedImageFormats()) {
      mimeFilters.append("image/" + mime);
    }
  }

  mimeFilters.sort(Qt::CaseSensitive);
  dialog->setMimeTypeFilters(mimeFilters);

  QStringList selectedFiles;
  if (dialog->exec()) {
    selectedFiles = dialog->selectedFiles();
    if (!selectedFiles.isEmpty())
      SettingsManager::instance().settings().setValue(
          "lastAttachmentDir", QFileInfo(selectedFiles.first()).absolutePath());
  }
  dialog->deleteLater();
  return selectedFiles;
}

void WebEnginePage::handleCertificateError(
    const QWebEngineCertificateError &error) {
  QString description = error.description();
  QWidget *mainWindow = dialogParent();
  if (error.isOverridable()) {
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() &
                          ~Qt::WindowContextHelpButtonHint);
    Ui::CertificateErrorDialog certificateDialog;
    certificateDialog.setupUi(&dialog);
    certificateDialog.m_iconLabel->setText(QString());
    QIcon icon(dialogStyle(mainWindow)->standardIcon(QStyle::SP_MessageBoxWarning,
                                                 nullptr, mainWindow));
    certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
    certificateDialog.m_errorLabel->setText(description);
    dialog.setWindowTitle(tr("Certificate Error"));
    bool accepted = dialog.exec() == QDialog::Accepted;
    auto handler = const_cast<QWebEngineCertificateError &>(error);
    if (accepted)
      handler.acceptCertificate();
    else
      handler.rejectCertificate();
  }

  QMessageBox::critical(mainWindow, tr("Certificate Error"), description);
}

void WebEnginePage::handleAuthenticationRequired(const QUrl &requestUrl,
                                                 QAuthenticator *auth) {
  QWidget *mainWindow = dialogParent();
  QDialog dialog(mainWindow);
  dialog.setModal(true);
  dialog.setWindowFlags(dialog.windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  Ui::PasswordDialog passwordDialog;
  passwordDialog.setupUi(&dialog);

  passwordDialog.m_iconLabel->setText(QString());
  QIcon icon(dialogStyle(mainWindow)->standardIcon(QStyle::SP_MessageBoxQuestion,
                                               nullptr, mainWindow));
  passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

  QString introMessage(
      tr("Enter username and password for \"%1\" at %2")
          .arg(auth->realm(), requestUrl.toString().toHtmlEscaped()));
  passwordDialog.m_infoLabel->setText(introMessage);
  passwordDialog.m_infoLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted) {
    auth->setUser(passwordDialog.m_userNameLineEdit->text());
    auth->setPassword(passwordDialog.m_passwordLineEdit->text());
  } else {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

void WebEnginePage::handleProxyAuthenticationRequired(
    const QUrl &, QAuthenticator *auth, const QString &proxyHost) {
  QWidget *mainWindow = dialogParent();
  QDialog dialog(mainWindow);
  dialog.setModal(true);
  dialog.setWindowFlags(dialog.windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  Ui::PasswordDialog passwordDialog;
  passwordDialog.setupUi(&dialog);

  passwordDialog.m_iconLabel->setText(QString());
  QIcon icon(dialogStyle(mainWindow)->standardIcon(QStyle::SP_MessageBoxQuestion,
                                               nullptr, mainWindow));
  passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

  QString introMessage = tr("Connect to proxy \"%1\" using:");
  introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
  passwordDialog.m_infoLabel->setText(introMessage);
  passwordDialog.m_infoLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted) {
    auth->setUser(passwordDialog.m_userNameLineEdit->text());
    auth->setPassword(passwordDialog.m_passwordLineEdit->text());
  } else {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

//! [registerProtocolHandlerRequested]
void WebEnginePage::handleRegisterProtocolHandlerRequested(
    QWebEngineRegisterProtocolHandlerRequest request) {
  auto answer = QMessageBox::question(
      dialogParent(), tr("Permission Request"),
      tr("Allow %1 to open all %2 links?")
          .arg(request.origin().host(), request.scheme()));
  if (answer == QMessageBox::Yes)
    request.accept();
  else
    request.reject();
}
//! [registerProtocolHandlerRequested]

void WebEnginePage::handleSelectClientCertificate(
    QWebEngineClientCertificateSelection selection) {
  // Just select one.
  selection.select(selection.certificates().at(0));

  qDebug() << __FUNCTION__;
  auto certificates = selection.certificates();
  for (const QSslCertificate &cert : std::as_const(certificates)) {
    qDebug() << cert;
    selection.select(cert); // select the first available cert
    break;
  }
  qDebug() << selection.host();
}

void WebEnginePage::javaScriptConsoleMessage(
    WebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message,
    int lineId, const QString &sourceId) {
  // The page's console used to be discarded outright, which made anything only
  // WhatsApp Web can see — a failed paste, a blocked request — impossible to
  // diagnose from a bug report. Surface warnings and errors; the chattier
  // levels are compiled out of release builds by QT_NO_DEBUG_OUTPUT.
  const QString where = QStringLiteral("%1:%2").arg(sourceId).arg(lineId);
  switch (level) {
  case QWebEnginePage::ErrorMessageLevel:
    qWarning().noquote() << "[js error]" << where << message;
    break;
  case QWebEnginePage::WarningMessageLevel:
    qWarning().noquote() << "[js warn]" << where << message;
    break;
  default:
    qDebug().noquote() << "[js]" << where << message;
    break;
  }
}

void WebEnginePage::injectPreventScrollWheelZoomHelper() {
  QString js = R"(
                    (function () {
                        const SSWZ = function () {
                            this.keyScrollHandler = function (e) {
                                if (e.ctrlKey) {
                                    e.preventDefault();
                                    return false;
                                }
                            }
                        };
                        if (window === top) {
                            const sswz = new SSWZ();
                            window.addEventListener('wheel', sswz.keyScrollHandler, {
                                passive: false
                            });
                        }
                    })();
                )";
  this->runJavaScript(js);
}

void WebEnginePage::injectNewChatJavaScript() {
  QString js = R"(const openNewChatWhatsie = (phone,text) => {
                    const link = document.createElement('a');
                    link.setAttribute('href',
                    `whatsapp://send/?phone=${phone}&text=${text}`);
                    document.body.append(link);
                    link.click();
                    document.body.removeChild(link);
                };
                function openNewChatWhatsieDefined()
                {
                    return (openNewChatWhatsie != 'undefined');
                })";
  this->runJavaScript(js);
}
