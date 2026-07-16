#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QCoreApplication>
#include <QSettings>

#include "appprofile.h"

// One QSettings, but scoped to the active account (see AppProfile). The default
// account keeps the file the app has always used — whatly.conf — so nothing
// moves on upgrade; a --profile=work account reads and writes whatly-work.conf
// instead. The file is chosen lazily, on first use, by which point main() has
// already fixed the account and set the organisation name.
class SettingsManager {
public:
  static SettingsManager &instance() {
    static SettingsManager instance;
    return instance;
  }

  QSettings &settings() { return *m_settings; }

private:
  SettingsManager()
      : m_settings(new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                                 QCoreApplication::organizationName(),
                                 QCoreApplication::applicationName() +
                                     AppProfile::suffix())) {}

  QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
