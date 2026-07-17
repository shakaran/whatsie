#ifndef WEBFONT_H
#define WEBFONT_H

#include <QString>
#include <QStringList>

class QWebEngineProfile;

// Overrides the font family WhatsApp Web renders text in, with a family
// installed on the system (issue #219). Implemented as an injected <style>: no
// dependency on WhatsApp's compiler-generated class names, and clearing the
// setting simply removes the stylesheet.
namespace WebFont {

// Available system families, with an empty first entry meaning "WhatsApp's
// own default" (inject nothing).
QStringList families();

// Empty means "use WhatsApp's default".
QString currentFamily();
void setCurrentFamily(const QString &family);

// Empty when no override is set.
QString scriptSource();

void install(QWebEngineProfile *profile);

} // namespace WebFont

#endif // WEBFONT_H
