#!/usr/bin/env bash
# Capture the main window in dark and light themes over CDP, with the privacy
# blur on, from the real logged-in session. Restores the config no matter what.
# Meant to be run in the background (it waits minutes for WhatsApp Web to load).
set -uo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"; cd "$REPO"
BIN="$REPO/build/whatly"; SHOTS="$REPO/tools/shots"; CFG="$HOME/.config/shakaran/whatly.conf"
mkdir -p "$SHOTS"
BK="$(mktemp)"; cp "$CFG" "$BK"
kill_native() { for p in $(pgrep -f "$BIN" 2>/dev/null || true); do kill -9 "$p" 2>/dev/null || true; done; sleep 1; }
restore() { cp "$BK" "$CFG"; rm -f "$BK"; kill_native; echo "config restored"; }
trap restore EXIT
setkey() { grep -q "^$1=" "$CFG" && sed -i "s|^$1=.*|$1=$2|" "$CFG" || echo "$1=$2" >> "$CFG"; }

cap() { # theme out
  kill_native
  setkey windowTheme "$1"; setkey privacyBlur all; setkey chatTheme none
  LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
    QTWEBENGINE_CHROMIUM_FLAGS="--remote-debugging-port=9222 --remote-allow-origins=* --disable-gpu" \
    "$BIN" >/dev/null 2>&1 &
  for _ in $(seq 1 45); do
    curl -s http://127.0.0.1:9222/json/version >/dev/null 2>&1 || { sleep 2; continue; }
    rows=$(python3 tools/cdp-eval.py '(document.querySelectorAll("#pane-side [role=\"row\"]").length)' 2>/dev/null || echo 0)
    [ "${rows:-0}" -ge 1 ] 2>/dev/null && break
    sleep 3
  done
  sleep 4
  # Dismiss any "What's new" / promo dialog WhatsApp shows on a fresh load.
  python3 tools/cdp-eval.py '(function(){var b=[].slice.call(document.querySelectorAll("button,div[role=\"button\"]")).find(function(x){return /continue|continuar|ok|got it|entendido/i.test((x.textContent||"").trim());}); if(b){b.click();return "dismissed";} return "none";})()' >/dev/null 2>&1 || true
  sleep 2
  python3 tools/cdp-screenshot.py "$2"
  kill_native
}

cap dark  "$SHOTS/main-dark.png"
cap light "$SHOTS/main-light.png"
echo "captured:"; ls -la "$SHOTS"/main-*.png 2>/dev/null
