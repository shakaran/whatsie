#ifndef CHATTHEME_H
#define CHATTHEME_H

#include <QList>
#include <QString>

class QWebEngineProfile;

// Recolours WhatsApp Web itself — not Whatly's own widgets, which the
// "windowTheme" setting covers.
//
// WhatsApp paints every colour through CSS custom properties, but their names
// are compiler-generated (--x184htih and the like) and change with each of its
// deploys, so a theme cannot list them. This one keys on the VALUE instead: it
// reads the declared colour of every property, and rewrites the ones that are
// WhatsApp green into the theme's hue, and the neutral greys into faintly
// tinted greys. Photos, avatars and stickers are never touched, because no
// image goes through a custom property.
namespace ChatTheme {

struct Theme {
  QString id;           // stored in the settings
  QString name;         // shown in Settings (translated at display time)
  int hue;              // 0-359, the colour the greens become
  double accentSat;     // ceiling on accent saturation: lower is softer
  double neutralTint;   // saturation given to greys: higher is more tinted
};

// "none" first: WhatsApp's own colours, nothing injected.
QList<Theme> themes();

QString currentThemeId();
void setCurrentThemeId(const QString &id);

// The recolouring userscript for the current theme, empty when it is "none".
// Also used to re-theme an already-loaded page, since Qt does not propagate
// profile-script changes to a live page.
QString scriptSource();

void install(QWebEngineProfile *profile);

} // namespace ChatTheme

#endif // CHATTHEME_H
