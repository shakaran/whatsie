#ifndef FOCUSMODE_H
#define FOCUSMODE_H

#include <QString>

class QWebEngineProfile;

// Focus mode: hide the chat list's message previews and contact names, leaving
// the conversation you are actually in readable. A stronger, always-on sibling
// of the privacy blur — meant for screen sharing, screenshots and open-plan
// desks, where the blur's hover-to-reveal is still too revealing.
//
// Implemented as an injected stylesheet, so it toggles without a reload.
namespace FocusMode {

bool isEnabled();
void setEnabled(bool enabled);

// The injected script (a <style> element it adds/removes).
QString scriptSource();

// (Re)install on a profile; removes the script when disabled.
void install(QWebEngineProfile *profile);

} // namespace FocusMode

#endif // FOCUSMODE_H
