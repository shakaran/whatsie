#include "screenlock.h"
#include "settingsmanager.h"

namespace ScreenLock {

bool isEnabled() {
  return SettingsManager::instance()
      .settings()
      .value(QStringLiteral("lockOnScreenLock"), false)
      .toBool();
}

void setEnabled(bool on) {
  SettingsManager::instance().settings().setValue(
      QStringLiteral("lockOnScreenLock"), on);
}

bool shouldLock(bool screensaverActive, bool passcodeConfigured) {
  return screensaverActive && passcodeConfigured && isEnabled();
}

} // namespace ScreenLock
