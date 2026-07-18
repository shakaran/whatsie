#include "webview.h"

#include <QBuffer>
#include <QChildEvent>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QImage>
#include <QMenu>
#include <QMimeData>
#include <mainwindow.h>
#include <QWebEngineContextMenuRequest>

#include "settingsmanager.h"

using QWebEngineContextMenuData = QWebEngineContextMenuRequest;

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent) {

  QObject *parentMainWindow = this->parent();
  while (!parentMainWindow->objectName().contains("MainWindow")) {
    parentMainWindow = parentMainWindow->parent();
  }
  MainWindow *mainWindow = dynamic_cast<MainWindow *>(parentMainWindow);

  connect(this, &WebView::titleChanged, mainWindow,
          &MainWindow::handleWebViewTitleChanged);
  connect(this, &WebView::loadFinished, mainWindow,
          &MainWindow::handleLoadFinished);
  connect(this, &WebView::renderProcessTerminated,
          [this](QWebEnginePage::RenderProcessTerminationStatus termStatus,
                 int statusCode) {
            QString status;
            switch (termStatus) {
            case QWebEnginePage::NormalTerminationStatus:
              status = tr("Render process normal exit");
              break;
            case QWebEnginePage::AbnormalTerminationStatus:
              status = tr("Render process abnormal exit");
              break;
            case QWebEnginePage::CrashedTerminationStatus:
              status = tr("Render process crashed");
              break;
            case QWebEnginePage::KilledTerminationStatus:
              status = tr("Render process killed");
              break;
            }
            // A normal exit is not a crash (it happens on a deliberate reload),
            // so never act on it. For the crash/kill/abnormal cases, honour the
            // "reload automatically after a crash" setting: reload without
            // interrupting the user, otherwise ask as before (issue #225).
            if (termStatus == QWebEnginePage::NormalTerminationStatus)
              return;

            const bool autoRestart = SettingsManager::instance()
                                         .settings()
                                         .value("autoRestartOnCrash", false)
                                         .toBool();
            if (autoRestart) {
              qWarning() << "Render process ended:" << status
                         << "code" << statusCode << "- reloading automatically.";
              QTimer::singleShot(0, this, [this] { this->reload(); });
              return;
            }

            QMessageBox::StandardButton btn =
                QMessageBox::question(window(), status,
                                      tr("Render process exited with code: %1\n"
                                         "Do you want to reload the page ?")
                                          .arg(statusCode));
            if (btn == QMessageBox::Yes)
              QTimer::singleShot(0, this, [this] { this->reload(); });
          });
}

void WebView::wheelEvent(QWheelEvent *event) {
  bool controlKeyIsHeld =
      QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
  // this doesn't work, (even after checking the global QApplication keyboard
  // modifiers) as expected, the Ctrl+wheel is managed by Chromium
  // WebenginePage directly. So, we manage it by injecting js to page using
  // WebEnginePage::injectPreventScrollWheelZoomHelper
  if ((event->modifiers() & Qt::ControlModifier) != 0 || controlKeyIsHeld) {
    qDebug() << "skipped ctrl + m_wheel event on webengineview";
    event->ignore();
  } else {
    QWebEngineView::wheelEvent(event);
  }
}

void WebView::contextMenuEvent(QContextMenuEvent *event) {
  auto menu = createStandardContextMenu();
  menu->setAttribute(Qt::WA_DeleteOnClose, true);
  // hide reload, back, forward, savepage, copyimagelink menus
  foreach (auto *action, menu->actions()) {
    if (action == page()->action(QWebEnginePage::SavePage) ||
        action == page()->action(QWebEnginePage::Reload) ||
        action == page()->action(QWebEnginePage::Back) ||
        action == page()->action(QWebEnginePage::Forward) ||
        action == page()->action(QWebEnginePage::CopyImageUrlToClipboard)) {
      action->setVisible(false);
    }
  }

  const QWebEngineContextMenuRequest &data = *lastContextMenuRequest();

  // allow context menu on image
  if (data.mediaType() == QWebEngineContextMenuData::MediaTypeImage) {
    QWebEngineView::contextMenuEvent(event);
    return;
  }
  // if content is not editable
  if (data.selectedText().isEmpty() && !data.isContentEditable()) {
    event->ignore();
    return;
  }

  connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
  menu->popup(event->globalPos());
}

// ── Clipboard image paste ─────────────────────────────────────────────────────
//
// Qt WebEngine drops the image when the clipboard also carries url/html
// flavours — which is exactly what a browser puts there on "Copy image". The
// page then only receives a text/uri-list and nothing is pasted. A clipboard
// holding just an image (a screenshot tool) arrives correctly as a File, so
// only the mixed case needs rescuing: read the image with QClipboard, which
// does see it, and hand it to the page as a File in a synthetic paste event.

// QWebEngineView delivers input through an internal child widget, so the key
// press has to be caught there rather than on the view itself.
bool WebView::event(QEvent *event) {
  if (event->type() == QEvent::ChildAdded) {
    auto *childEvent = static_cast<QChildEvent *>(event);
    if (childEvent->child() && childEvent->child()->isWidgetType())
      childEvent->child()->installEventFilter(this);
  }
  return QWebEngineView::event(event);
}

bool WebView::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->matches(QKeySequence::Paste) && pasteClipboardImage())
      return true; // handled here; skip the native paste that would drop it
  }
  return QWebEngineView::eventFilter(watched, event);
}

bool WebView::pasteClipboardImage() {
  const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
  if (!mimeData || !mimeData->hasImage())
    return false; // no image at all: nothing to rescue

  // An image-only clipboard already pastes correctly; leave that path alone.
  if (!mimeData->hasUrls() && !mimeData->hasHtml())
    return false;

  const QImage image = qvariant_cast<QImage>(mimeData->imageData());
  if (image.isNull())
    return false;

  QByteArray png;
  QBuffer buffer(&png);
  if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG"))
    return false;
  buffer.close();

  static const QString kInject = QStringLiteral(R"JS(
(function () {
  try {
    var binary = atob("%1");
    var bytes = new Uint8Array(binary.length);
    for (var i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
    var file = new File([bytes], "image.png", { type: "image/png" });
    var transfer = new DataTransfer();
    transfer.items.add(file);
    var target = document.activeElement || document.body;
    target.dispatchEvent(new ClipboardEvent("paste", {
      clipboardData: transfer, bubbles: true, cancelable: true
    }));
  } catch (e) {
    console.error("whatly: pasting the clipboard image failed: " + e);
  }
})();
)JS");

  page()->runJavaScript(kInject.arg(QString::fromLatin1(png.toBase64())));
  return true;
}
