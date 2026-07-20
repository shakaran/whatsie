#ifndef SCREENLOCK_H
#define SCREENLOCK_H

// "Lock Whatly when the desktop session locks." A thin policy layer over the
// existing app lock: when the screensaver/lock becomes active and the user has
// both enabled this and configured a passcode, Whatly locks itself too.
//
// The decision is a pure function so it is unit-tested; the D-Bus subscription
// to org.freedesktop.ScreenSaver lives in MainWindow (Linux only).
namespace ScreenLock {

bool isEnabled();          // the "lockOnScreenLock" setting (default false)
void setEnabled(bool on);

// Whether to lock now, given the screensaver state and whether a passcode is
// configured. Locking only makes sense when the saver just became active.
bool shouldLock(bool screensaverActive, bool passcodeConfigured);

} // namespace ScreenLock

#endif // SCREENLOCK_H
