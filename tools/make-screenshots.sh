#!/usr/bin/env bash
#
# Regenerate the Whatly README screenshots and feature cards.
#
#   tools/make-screenshots.sh            # Settings + About (safe, throwaway profile)
#   tools/make-screenshots.sh --with-chat  # also the chat card, from your REAL session
#
# The Settings and About shots use a disposable profile and contain no personal
# data. The chat card needs a logged-in session, so it launches YOUR profile
# with the privacy blur turned on, screenshots the web content over CDP, blurs a
# couple of extra regions the blur feature does not cover (the label pills, your
# avatar), and restores your settings afterwards.
#
# WhatsApp Web renders in your account language (set on your phone), not the
# app's — switch your phone's WhatsApp to English first if you want an English
# chat shot.
#
# Requires: a built ./build/whatly, spectacle, ImageMagick, python3 with
# Pillow + websockets, curl.
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO"
SHOTS="$REPO/tools/shots"; mkdir -p "$SHOTS"
BIN="$REPO/build/whatly"
WITH_CHAT=0; [ "${1:-}" = "--with-chat" ] && WITH_CHAT=1

[ -x "$BIN" ] || { echo "Build first: cmake --build build"; exit 1; }

en() { LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 "$@"; }
kill_whatly() { for p in $(pgrep -f "$BIN" 2>/dev/null || true); do kill -9 "$p" 2>/dev/null || true; done; sleep 1; }

# Grab the active window a few times until the size looks like the target dialog.
grab() { # $1=out  $2=regex of acceptable WxH
  for _ in 1 2 3 4; do
    sleep 2.5
    spectacle -a -b -n -o "$1" 2>/dev/null || true
    local sz; sz=$(identify -format '%wx%h' "$1" 2>/dev/null || echo "")
    echo "   grabbed $sz"
    [[ "$sz" =~ $2 ]] && return 0
    sleep 1
  done
}

echo "==> Settings + About (disposable profile, English)"
kill_whatly
TMP="$(mktemp -d)"
en XDG_CONFIG_HOME="$TMP/config" XDG_DATA_HOME="$TMP/data" "$BIN" >/dev/null 2>&1 &
sleep 8
en XDG_CONFIG_HOME="$TMP/config" XDG_DATA_HOME="$TMP/data" "$BIN" -s >/dev/null 2>&1 & grab "$SHOTS/settings.png" '^[0-9]{3}x9[0-9]{2}$'
en XDG_CONFIG_HOME="$TMP/config" XDG_DATA_HOME="$TMP/data" "$BIN" -i >/dev/null 2>&1 & grab "$SHOTS/about.png"    '^5[0-9]{2}x5[0-9]{2}$'
kill_whatly
rm -rf "$TMP"

if [ "$WITH_CHAT" = "1" ]; then
  echo "==> Chat card (your real session, privacy blur on)"
  CFG="$HOME/.config/shakaran/whatly.conf"
  BK="$(mktemp)"; cp "$CFG" "$BK"
  trap 'cp "$BK" "$CFG"; rm -f "$BK"; kill_whatly' EXIT
  sed -i 's/^chatTheme=.*/chatTheme=teal/; s/^privacyBlur=.*/privacyBlur=all/' "$CFG"
  grep -q '^privacyBlur=' "$CFG" || echo 'privacyBlur=all' >> "$CFG"
  en QTWEBENGINE_CHROMIUM_FLAGS="--remote-debugging-port=9222 --remote-allow-origins=* --disable-gpu --no-sandbox" \
     "$BIN" >/dev/null 2>&1 &
  echo "   waiting for the chat list to render..."
  for _ in $(seq 1 40); do
    curl -s http://127.0.0.1:9222/json/version >/dev/null 2>&1 || { sleep 2; continue; }
    rows=$(python3 tools/cdp-eval.py '(document.querySelectorAll("#pane-side [role=\"row\"]").length)' 2>/dev/null || echo 0)
    [ "${rows:-0}" -ge 1 ] 2>/dev/null && break
    sleep 3
  done
  # Blur the label-filter pills and the own avatar (the app's blur misses these).
  python3 tools/cdp-screenshot.py "$SHOTS/chat.png" \
      --blur 60,82,306,34,11 --blur 2,606,44,46,11 --blur 56,205,306,449,6
  echo "   VERIFY $SHOTS/chat.png is fully blurred before using it."
fi

echo "==> Composing cards -> docs/img"
python3 tools/make-cards.py --shots "$SHOTS" --out "$REPO/docs/img"
echo "Done."
